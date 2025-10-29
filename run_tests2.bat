@echo off
echo ==============================================
echo      Run - Hachage par Automate Cellulaire 
echo ==============================================
echo.

:: Vérifie que le programme compilé existe
if not exist output\Exercice2.exe (
    echo [ERREUR] Le fichier output\Exercice2.exe est introuvable.
    echo Compilation requise avant exécution.
    pause
    exit /b
)

echo Lancement du programme...
echo (Execution automatique des tests)
echo.

(
    echo 1
    echo eyfguh
    echo 6
    echo 5
    echo 2
    echo 3
    echo 0
) | output\Exercice2.exe > resultats\resultats2.txt

echo.
echo ==============================================
echo      Programme termine. Resultats sauvegardes
echo ==============================================
echo.
pause
