@echo off
echo "Generating projects..."
cd ..
cmake -G "Visual Studio 17 2022" -A x64 .

if NOT ["%errorlevel%"]==["0"] (
    pause
    exit /b %errorlevel%
)