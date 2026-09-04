#!/bin/bash
echo "Waiting for GitHub release v1.2.1 to be created by Actions..."
while true; do
    if gh release view v1.2.1 -R laurorobles/Orbita-LPG > /dev/null 2>&1; then
        echo "Release found! Uploading Mac zip..."
        gh release upload v1.2.1 /Users/babyonk1/Desktop/Orbita-LPG-macOS-v1.2.1.zip -R laurorobles/Orbita-LPG --clobber
        echo "Upload complete."
        break
    fi
    echo "Still waiting..."
    sleep 10
done
