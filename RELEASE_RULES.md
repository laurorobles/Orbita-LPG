# 📦 RELEASE RULES & DEPLOYMENT

This project uses GitHub Actions for Windows and Linux builds, and local Universal builds for macOS.

## 1. Automated Pipeline (GitHub Actions)
Defined in `.github/workflows/build.yml`:
- **Triggers:** Pushing a version tag (`v*.*.*`) builds Windows (`x64`) and Linux (`x64`) binaries (VST3, CLAP, Standalone) and publishes a GitHub Release automatically.

## 2. macOS Universal Compilation (Local)
Built locally as a Universal Binary (`arm64;x86_64`) including VST3, AU, CLAP, and Standalone:
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64"
cmake --build build --config Release --parallel
```
Then zip the macOS artifacts + installer + docs as `Orbita-LPG-macOS.zip` and attach to the GitHub Release:
```bash
gh release upload vX.X.X Orbita-LPG-macOS.zip -R laurorobles/Orbita-LPG --clobber
```

## 3. Version Bumping
1. Update `project(OrbitaLPG VERSION X.X.X)` in `CMakeLists.txt`.
2. Update the version label in `Source/PluginEditor.cpp` and `Source/PluginEditor.h`.
3. Commit, tag, and push:
```bash
git add .
git commit -m "Release vX.X.X"
git tag -a vX.X.X -m "Release vX.X.X"
git push origin main --tags
```

---
*Maintained by Lauro Robles / Extasis Records (laurorobles@gmail.com).*
