@echo off
echo Cleaning build artifacts...

if exist build (
    rmdir /s /q build
    echo Build directory removed.
) else (
    echo No build directory found.
)

echo Clean complete.

