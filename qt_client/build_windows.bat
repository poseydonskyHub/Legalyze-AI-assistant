@echo off
setlocal
cd /d "%~dp0"

echo === Legalyze Qt client: Windows сборка ===

if not exist "build" mkdir build
cd build

cmake .. 
if errorlevel 1 (
    echo Ошибка configure. Проверьте, что установлен Qt6 и CMake.
    pause
    exit /b 1
)

cmake --build . --config Release
if errorlevel 1 (
    echo Ошибка сборки. Проверьте Qt6 kit / compiler.
    pause
    exit /b 1
)

echo Сборка завершена.
pause
