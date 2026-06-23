#!/usr/bin/env bash
set -e

echo "==========================================="
echo " Startup Validator - Build Script (C++)"
echo "==========================================="
echo ""

echo "Checking dependencies..."

if ! command -v cmake >/dev/null 2>&1; then
  echo "cmake not found."
  exit 1
fi

if ! command -v curl >/dev/null 2>&1; then
  echo "curl not found."
  exit 1
fi

echo "Dependencies OK"
echo ""

cd backend
mkdir -p build
cd build

echo "Running CMake..."
cmake .. -DCMAKE_BUILD_TYPE=Release

echo ""
echo "Building..."
cmake --build . --config Release -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 2)

echo ""
echo "==========================================="
echo " BUILD COMPLETE!"
echo "==========================================="
echo ""
echo "Make sure Ollama is running and a model is installed:"
echo "  ollama pull llama3.2:3b"
echo "  export OLLAMA_MODEL=llama3.2:3b"
echo ""
echo "Run the server:"
echo "  ./backend/build/bin/startup_validator"
echo ""
echo "Then open: frontend/index.html in your browser"
