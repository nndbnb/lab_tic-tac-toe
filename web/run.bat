@echo off
echo === Starting Infinite Tic-Tac-Toe Web Interface ===
echo.

REM Check if Flask is installed
python -c "import flask" 2>nul
if errorlevel 1 (
    echo Flask is not installed. Installing...
    pip install -r requirements.txt
    if errorlevel 1 (
        echo Failed to install Flask. Please install manually: pip install Flask
        pause
        exit /b 1
    )
)

REM Check if web_cli.exe exists
if not exist ..\build\web_cli.exe (
    echo Warning: web_cli.exe not found in ..\build\
    echo Please build the C++ project first by running: ..\build.bat
    echo.
    pause
)

echo Starting Flask server...
echo Open http://localhost:5000 in your browser
echo.
python app.py

