@echo off
echo ============================================
echo  Startup Validator -- Build Script (C++)
echo ============================================
echo.

where cmake >nul 2>&1 || (echo [ERROR] cmake not found. Install from https://cmake.org/download/ && exit /b 1)
where git   >nul 2>&1 || (echo [ERROR] git not found. Install from https://git-scm.com/         && exit /b 1)

cd backend
if not exist build mkdir build
cd build

echo Running CMake...
cmake .. -DCMAKE_BUILD_TYPE=Release

echo.
echo Building (first run downloads Crow + nlohmann/json)...
cmake --build . --config Release

echo.
echo ============================================
echo  BUILD COMPLETE!
echo ============================================
echo.
echo Make sure Ollama is running and a model is installed:
echo   ollama pull llama3.2:3b
echo   set OLLAMA_MODEL=llama3.2:3b
echo.
echo Run the server:
echo   backend\build\bin\Release\startup_validator.exe
echo.
echo Then open frontend\index.html in your browser
pause
