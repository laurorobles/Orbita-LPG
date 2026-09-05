#!/bin/bash
while pgrep -f "make" > /dev/null; do
    sleep 2
done
while pgrep -f "clang" > /dev/null; do
    sleep 2
done
cd /Users/babyonk1/Desktop/ExtasisRecords/Orbita-LPG-JUCE
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
zip -r /Users/babyonk1/Desktop/Orbita-LPG-macOS-v1.2.2.zip macOS/
cd ..
rm -rf mac_release_temp

gh release upload v1.2.2 /Users/babyonk1/Desktop/Orbita-LPG-macOS-v1.2.2.zip -R laurorobles/Orbita-LPG --clobber
