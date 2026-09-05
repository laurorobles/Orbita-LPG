#!/bin/bash
echo "Waiting for GitHub release v1.2.2..."
while true; do
    if gh release view v1.2.2 -R laurorobles/Orbita-LPG > /dev/null 2>&1; then
        echo "Release found! Uploading Mac zip..."
        gh release upload v1.2.2 /Users/babyonk1/Desktop/Orbita-LPG-macOS-v1.2.2.zip -R laurorobles/Orbita-LPG --clobber
        echo "Upload complete."
        break
    fi
    sleep 5
done
