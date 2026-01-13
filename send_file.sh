#!/bin/bash
#SBATCH --account=tra25_IngInfBo
#SBATCH -t 00:20:00
#SBATCH --nodes=1
#SBATCH -o send_file.out

# Server URL
SERVER_URL="https://openairish-apolonia-unbrothered.ngrok-free.dev/upload"

# Cartelle da inviare
DIRS=("results" "output")

for dir in "${DIRS[@]}"; do
    if [ -d "$dir" ]; then
        for file in "$dir"/*; do
            if [ -f "$file" ]; then
                echo "Invio $file"
                curl -X POST -F "file=@$file" "$SERVER_URL"
            fi
        done
    else
        echo "Attenzione: cartella $dir non trovata"
    fi
done

# Note:
# - Per il server ngrok, assicurati che sia in ascolto
# - Puoi salvare l'URL in un json se vuoi
