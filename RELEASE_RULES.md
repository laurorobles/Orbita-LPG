# 📦 RELEASE RULES & DEPLOYMENT

This project uses an automated CI/CD pipeline via GitHub Actions to compile and distribute binaries across all operating systems.

## 1. Automated Pipeline (GitHub Actions)
The workflow is defined in `.github/workflows/build.yml`.
- **Triggers:** Pushing a tag starting with `v` (e.g., `v1.2.1`) triggers the Release job.
- **Runners:** 
  - `ubuntu-latest` (Linux x64)
  - `windows-latest` (Windows x64)
- **Outputs:** Zips the VST3, CLAP, and Standalone apps along with all Markdown documentation and installer scripts, then creates an official GitHub Release.

## 2. macOS Compilation
Due to instability and limitations with GitHub's Apple Silicon runners, the macOS binaries (VST3, AU, CLAP, Standalone) are **built locally**.
- **Process:**
  1. Modify code.
  2. Run `cmake -B build -G "Ninja" -DCMAKE_BUILD_TYPE=Release -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64"`
  3. Run `cmake --build build --config Release --parallel`
  4. Run `package_mac_v2.sh` to zip the artifacts into `Orbita-LPG-macOS-vX.X.X.zip`.
  5. Manually attach the zip to the automated GitHub release.

## 3. Version Bumping
Before creating a new release, you must update the version string in `CMakeLists.txt`:
```cmake
project(OrbitaLPG VERSION X.X.X)
```
Then commit, tag, and push:
```bash
git add .
git commit -m "Prepare release vX.X.X"
git tag -a vX.X.X -m "Release vX.X.X"
git push origin main --tags
```
