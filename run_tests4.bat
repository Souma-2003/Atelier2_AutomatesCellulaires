@echo off
echo ==============================================
echo Run - Exercice4 :   Performance AC-Hash vs SHA256
echo ==============================================
echo.

:: Vérifie si le fichier compilé existe
if not exist output\Exercice4.exe (
    echo [ERREUR] Le fichier output\Exercice4.exe est introuvable.
    echo Veuillez compiler Exercice4.cpp avant de lancer ce test.
    pause
    exit /b
)

echo Lancement du programme...
echo (Execution automatique des tests)
echo.

( 
    echo 3
    echo 0
) | output\Exercice4.exe > resultats\resultats4.txt

echo.
echo ==============================================
echo     Programme termine. Resultats sauvegardes
echo ==============================================
echo.
pause
