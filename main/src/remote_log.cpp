#include "remote_log.h"
#include "esp_log.h"
#include "ws_queue.h"
#include <stdio.h>
#include <string>

// A pointer to store the default vprintf function
static vprintf_like_t default_vprintf = NULL;

// Helper function to strip ANSI escape codes from a string
static std::string strip_ansi_codes(const std::string& input) {
    std::string output;
    output.reserve(input.length());
    for (size_t i = 0; i < input.length(); ++i) {
        if (input[i] == '\x1b' && i + 1 < input.length() && input[i+1] == '[') {
            size_t j = i + 2;
            while (j < input.length() && input[j] != 'm') {
                j++;
            }
            if (j < input.length()) { // Found 'm'
                i = j; // Skip the entire sequence
            } else {
                output += input[i]; // Not a valid sequence, keep the character
            }
        } else {
            output += input[i];
        }
    }
    return output;
}


// Custom vprintf implementation to intercept logs
static int remote_log_vprintf(const char *format, va_list args) {
    // First, execute the default log behavior (e.g., printing to serial)
    int result = default_vprintf(format, args);

    // Now, format the log message for WebSocket transmission.
    // We need a copy of args because vprintf might consume it.
    va_list args_copy;
    va_copy(args_copy, args);
    char* log_str_p = nullptr;
    // vasprintf allocates the necessary memory for log_str_p
    if (vasprintf(&log_str_p, format, args_copy) < 0) {
        va_end(args_copy);
        return result; // Allocation failed
    }
    va_end(args_copy);

    std::string log_str_with_ansi(log_str_p);
    free(log_str_p);

    // Strip ANSI color codes to prevent JSON parsing errors
    std::string log_str = strip_ansi_codes(log_str_with_ansi);

    // Remove trailing newline characters that ESP_LOG adds
    size_t end = log_str.find_last_not_of("\r\n");
    if (std::string::npos != end) {
        log_str = log_str.substr(0, end + 1);
    }

    // Allocate a new WebSocket message
    WSMessage *msg = new WSMessage();
    if (!msg) {
        return result; // Allocation failed
    }

    // Create a JSON payload. ArduinoJson handles string escaping.
    msg->json["type"] = "log";
    msg->json["payload"] = log_str;

    // Send the message pointer to the outgoing queue.
    // Use a timeout of 0 to avoid blocking the logging function.
    if (xQueueSend(wsOutQueue, &msg, 0) != pdPASS) {
        // If the queue is full, delete the message to prevent a memory leak
        delete msg;
    }

    return result;
}

void initRemoteLog() {
    // Set our custom vprintf function as the new handler
    default_vprintf = esp_log_set_vprintf(remote_log_vprintf);
    ESP_LOGI("REMOTE_LOG", "Remote logging initialized.");
}