#!/bin/bash

SERVER_URL="https://openairish-apolonia-unbrothered.ngrok-free.dev/upload"
RESULTS_DIR="./results"

for file in "$RESULTS_DIR"/*; do
    if [ -f "$file" ]; then
        echo "Invio $file"
        curl -X POST -F "file=@$file" "$SERVER_URL"
    fi
done
