#!/bin/bash
cmake --build build --config Release --parallel

mkdir -p mac_release_temp/macOS
cp -r build/OrbitaLPG_artefacts/Release/VST3/Orbita-LPG.vst3 mac_release_temp/macOS/ 2>/dev/null || true
cp -r build/OrbitaLPG_artefacts/Release/AU/Orbita-LPG.component mac_release_temp/macOS/ 2>/dev/null || true
cp -r build/OrbitaLPG_artefacts/Release/CLAP/Orbita-LPG.clap mac_release_temp/macOS/ 2>/dev/null || true
cp -r build/OrbitaLPG_artefacts/Release/Standalone/Orbita-LPG.app mac_release_temp/macOS/ 2>/dev/null || true
cp -r installer mac_release_temp/macOS/ 2>/dev/null || true
for doc in MANUAL.md README.md PRESENTATION.md TECHNICAL.md ARCHITECTURE.md RELEASE_RULES.md; do
    cp "$doc" mac_release_temp/macOS/ 2>/dev/null || true
done

cd mac_release_temp
zip -r /Users/babyonk1/Desktop/Orbita-LPG-macOS-v1.2.3.zip macOS/
cd ..
rm -rf mac_release_temp

echo "Waiting for GitHub release v1.2.3 to be created..."
while true; do
    if gh release view v1.2.3 -R laurorobles/Orbita-LPG > /dev/null 2>&1; then
        echo "Release found! Uploading Mac zip..."
        gh release upload v1.2.3 /Users/babyonk1/Desktop/Orbita-LPG-macOS-v1.2.3.zip -R laurorobles/Orbita-LPG --clobber
        echo "Upload complete."
        break
    fi
    sleep 5
done
