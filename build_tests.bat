@echo off
setlocal enabledelayedexpansion

echo === Building Tests Only ===

REM Create build directory
if not exist build mkdir build
cd build

REM Compiler settings
set CC=g++
set CFLAGS=-std=c++17 -O3 -march=native -Wall -I../include
set LDFLAGS=

REM Source directories
set BOARD_SRC=../src/board
set ENGINE_SRC=../src/engine

REM Object files
set OBJS=

REM Compile board sources
echo Compiling board sources...
%CC% %CFLAGS% -c %BOARD_SRC%/sparse_board.cpp -o sparse_board.o
if errorlevel 1 goto :error
set OBJS=!OBJS! sparse_board.o

%CC% %CFLAGS% -c %BOARD_SRC%/zobrist.cpp -o zobrist.o
if errorlevel 1 goto :error
set OBJS=!OBJS! zobrist.o

REM Compile engine sources
echo Compiling engine sources...
%CC% %CFLAGS% -c %ENGINE_SRC%/move_generator.cpp -o move_generator.o
if errorlevel 1 goto :error
set OBJS=!OBJS! move_generator.o

%CC% %CFLAGS% -c %ENGINE_SRC%/evaluator.cpp -o evaluator.o
if errorlevel 1 goto :error
set OBJS=!OBJS! evaluator.o

%CC% %CFLAGS% -c %ENGINE_SRC%/threat_solver.cpp -o threat_solver.o
if errorlevel 1 goto :error
set OBJS=!OBJS! threat_solver.o

%CC% %CFLAGS% -c %ENGINE_SRC%/transposition_table.cpp -o transposition_table.o
if errorlevel 1 goto :error
set OBJS=!OBJS! transposition_table.o

%CC% %CFLAGS% -c %ENGINE_SRC%/search_engine.cpp -o search_engine.o
if errorlevel 1 goto :error
set OBJS=!OBJS! search_engine.o

REM Build tests
echo Building tests...
%CC% %CFLAGS% ../tests/board_tests.cpp %OBJS% %LDFLAGS% -o board_tests.exe
if errorlevel 1 goto :error

%CC% %CFLAGS% ../tests/evaluator_tests.cpp %OBJS% %LDFLAGS% -o evaluator_tests.exe
if errorlevel 1 goto :error

%CC% %CFLAGS% ../tests/engine_tests.cpp %OBJS% %LDFLAGS% -o engine_tests.exe
if errorlevel 1 goto :error

cd ..
echo.
echo === Tests built successfully! ===
echo Run tests from build/ directory:
echo   - board_tests.exe
echo   - evaluator_tests.exe
echo   - engine_tests.exe
goto :end

:error
echo.
echo === Build failed! ===
cd ..
exit /b 1

:end
endlocal

