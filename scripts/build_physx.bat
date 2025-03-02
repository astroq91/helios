@echo off
setlocal enabledelayedexpansion

cd ..\Helios\vendor\physx
call generate_projects.bat

:: Change the path if you use a different compiler
if exist compiler\vc17win64 (
    cd compiler\vc17win64
) else if exist compiler\vc16win64 (
    cd compiler\vc16win64
) else (
    echo Couldn't find generated files for vc17 or vc16. 
    exit /b 1
)
cmake --build . --target=ALL_BUILD --parallel

if NOT ["%errorlevel%"]==["0"] (
    pause
    exit /b %errorlevel%
)

endlocal
