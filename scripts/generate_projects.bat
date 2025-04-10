@echo off
echo "Generating projects..."
cd ..
cmake -G "Visual Studio 17 2022" -A x64 -DCMAKE_POLICY_DEFAULT_CMP0091=NEW .

if NOT ["%errorlevel%"]==["0"] (
    pause
    exit /b %errorlevel%
)
pause