@echo off
echo ==============================================
echo AC-Hash+ Automated Test Runner
echo ==============================================

echo Compiling Exercice10.cpp ...
g++ -std=c++17 -O3 -o hash_test.exe Exercice10.cpp
if errorlevel 1 (
    echo  Compilation failed!
    pause
    exit /b 1
)

echo Compilation successful!
echo Running tests...
echo.

hash_test.exe > resultats\results.txt

echo.
echo Tests completed!
echo Results saved in resultats\results.txt

echo.
echo Cleaning up...
del hash_test.exe
echo  Done.

pause
