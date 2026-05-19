#!/usr/bin/env bash
set -e

cd "$(dirname "$0")"

echo "=== Legalyze Qt client: Linux сборка ==="

mkdir -p build
cd build

cmake ..
cmake --build .

echo "Сборка завершена. Запуск: ./Legalyze"
