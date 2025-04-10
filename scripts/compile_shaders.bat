@echo off
SETLOCAL EnableDelayedExpansion

REM Set the base directory (parent of script directory)
SET "base_dir=%~dp0.."
FOR %%I IN ("%base_dir%") DO SET "base_dir=%%~fI"

REM Compile shaders in specified directories
CALL :compile_shaders "%base_dir%\Helios\resources\shaders"
CALL :compile_shaders "%base_dir%\Editor\resources\shaders"
CALL :compile_shaders "%base_dir%\examples"

PAUSE
GOTO :EOF

:compile_shaders
SET "target_dir=%~1"
SET "current_dir="

FOR /R "%target_dir%" %%F IN (*.vert *.frag) DO (
    IF EXIST "%%F" (
        SET "file=%%F"
        SET "dir=%%~dpF"
        SET "filename=%%~nxF"
        SET "out_dir=%%~dpFbin"

        REM Normalize slashes for comparison (not strictly necessary)
        SET "dir=!dir:\=/!"

        IF NOT "!dir!"=="!current_dir!" (
            ECHO.
            ECHO [!dir!]
            SET "current_dir=!dir!"
        )
       
        IF NOT EXIST "!out_dir!" (
            mkdir "!out_dir!"
        )
       
        ECHO "Compiling  -> bin\!filename!.spv"
        glslc "!file!" -o "!out_dir!\!filename!.spv"
    )
)

GOTO :EOF

