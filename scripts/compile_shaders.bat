@echo off
set base_dir=%~dp0..\

cd /d "%base_dir%"

echo Helios:
cd Helios\resources\shaders
if not exist bin mkdir bin

for %%f in (*.vert *.frag) do (
    if exist "%%f" (
        echo Compiling %%f...
        glslc "%%f" -o "bin\%%~nxf"
    )
)

echo.

cd /d "%base_dir%"

echo Editor:
cd Editor\resources\shaders
if not exist bin mkdir bin

for %%f in (*.vert *.frag) do (
    if exist "%%f" (
        echo Compiling %%f...
        glslc "%%f" -o "bin\%%~nxf"
    )
)

pause
