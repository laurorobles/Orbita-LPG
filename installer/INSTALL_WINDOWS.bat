@echo off
setlocal
echo ============================================================
echo  Orbita-LPG - Windows Automated Installer
echo ============================================================

set "INSTALL_DIR=%~dp0"
set "VST3_DIR=%COMMONPROGRAMFILES%\VST3"

echo [1/1] Installing VST3 Plugin to %VST3_DIR%...
if not exist "%VST3_DIR%" mkdir "%VST3_DIR%"
if exist "%INSTALL_DIR%Orbita-LPG.vst3" (
    xcopy /E /I /Y /Q "%INSTALL_DIR%Orbita-LPG.vst3" "%VST3_DIR%\Orbita-LPG.vst3"
)

echo.
echo ============================================================
echo  Installation Complete! Open your DAW and rescan plugins.
echo ============================================================
pause
