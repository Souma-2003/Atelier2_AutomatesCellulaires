@echo off
echo ==============================================
echo        Run - Exercice 3 : Modes de Hachage
echo ==============================================
echo.

:: Vérifie si le fichier compilé existe
if not exist output\Exercice3.exe (
    echo [ERREUR] Le fichier output\Exercice3.exe est introuvable.
    echo Veuillez compiler Exercice3.cpp avant de lancer ce test.
    pause
    exit /b
)

echo Lancement du programme...
echo (Execution automatique des tests)
echo.

(
   
    echo 2
) | output\Exercice3.exe > resultats\resultats3.txt

echo.
echo ==============================================
echo     Programme termine. Resultats sauvegardes
echo ==============================================
echo.
pause
