# 📋 REGLAS DE LANZAMIENTO — ÓRBITA-LPG / EXTASIS RECORDS

## Versiones
- Formato: **SemVer** (`MAJOR.MINOR.PATCH`)
- Sincronizar en `CMakeLists.txt` antes de cada release

## Checklist pre-release
- [ ] Versión actualizada en `CMakeLists.txt`
- [ ] `README.md` actualizado (features, screenshot)
- [ ] `MANUAL.md` refleja todos los parámetros actuales
- [ ] `TECHNICAL.md` actualizado (formatos, requisitos)
- [ ] `ARCHITECTURE.md` refleja el DSP actual
- [ ] Git tag `vX.Y.Z` creado y pusheado
- [ ] GitHub Actions compiló exitosamente para Windows, macOS y Linux
- [ ] Release notes publicadas en GitHub con enlace a Gumroad
- [ ] `screenshot.png` en `assets/` actualizado con la UI actual

## Comando de release
```bash
git tag -a vX.Y.Z -m "Descripción del release"
git push origin vX.Y.Z
```
El pipeline de GitHub Actions compila automáticamente y publica el release.

> **Licencia:** [http://laurorobles.gumroad.com](http://laurorobles.gumroad.com)
