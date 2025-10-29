@echo off
echo ==============================================
echo Automated Test Runner - Cellular Automata
echo ==============================================

set EXE_FILE=hash_test.exe
set SRC_FILE=Exercice1.cpp

echo Compiling %SRC_FILE% ...
g++ -std=c++17 -O3 -o %EXE_FILE% %SRC_FILE%
if errorlevel 1 (
    echo Compilation failed!
    pause
    exit /b 1
)

echo Compilation successful!
echo Running automated tests...
echo.

:: Exécuter automatiquement les règles 30, 90, 110
(
    echo 30
    echo 90
    echo 110
    echo 0
) | %EXE_FILE% > resultats\results1.txt

echo Tests completed!
echo Results saved in resultats\results1.txt

echo Cleaning up...
del %EXE_FILE%
echo Done.
pause
