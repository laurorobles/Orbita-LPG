#!/bin/bash
echo "============================================================"
echo " Orbita-LPG - macOS Automated Installer"
echo "============================================================"

INSTALL_DIR="$(cd "$(dirname "$0")" && pwd)"
VST3_DIR="/Library/Audio/Plug-Ins/VST3"
AU_DIR="/Library/Audio/Plug-Ins/Components"

echo "[1/2] Installing VST3 Plugin to $VST3_DIR..."
sudo mkdir -p "$VST3_DIR"
if [ -d "$INSTALL_DIR/Orbita-LPG.vst3" ]; then
    sudo cp -R "$INSTALL_DIR/Orbita-LPG.vst3" "$VST3_DIR/"
fi

echo "[2/2] Installing AU Plugin to $AU_DIR..."
sudo mkdir -p "$AU_DIR"
if [ -d "$INSTALL_DIR/Orbita-LPG.component" ]; then
    sudo cp -R "$INSTALL_DIR/Orbita-LPG.component" "$AU_DIR/"
fi

echo ""
echo "============================================================"
echo " Installation Complete! Open your DAW and rescan plugins."
echo "============================================================"
