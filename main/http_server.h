#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H
#include <WebServer.h>

WebServer& init_http_server();
void handle_client();

#endif