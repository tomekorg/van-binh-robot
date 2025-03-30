#!/usr/bin/env bash
set -e

cd "$(dirname "$0")/web-ui" || { echo "Error: Failed to change directory to web-ui." >&2; exit 1; }

if [ -z "$NVM_DIR" ]; then
  echo "Error: NVM_DIR is not set. Please ensure nvm is installed and configured correctly." >&2
  exit 1
fi

if [ -s "$NVM_DIR/nvm.sh" ]; then
  source "$NVM_DIR/nvm.sh"
else
  echo "Error: nvm is not installed or cannot be found at \$NVM_DIR/nvm.sh" >&2
  exit 1
fi

if ! nvm use; then
  echo "Error: Failed to switch Node version using nvm." >&2
  exit 1
fi

if ! npm install; then
  echo "Error: npm install failed." >&2
  exit 1
fi

if ! npm run build; then
  echo "Error: npm run build failed." >&2
  exit 1
fi

if ! xxd -n bundle_html -i dist/bundle.html > ../main/include/bundle_html.h; then
  echo "Error: xxd failed." >&2
  exit 1
fi