@echo off
echo ==============================================
echo       Run - Exercice 5 : AC_HASH Analysis
echo ==============================================
echo.

:: Vérifie si le fichier compilé existe
if not exist output\Exercice5.exe (
    echo [ERREUR] Le fichier output\Exercice5.exe est introuvable.
    echo Veuillez compiler Exercice5.cpp avant de lancer ce test.
    pause
    exit /b
)

:: Dossier de sortie
if not exist resultats mkdir resultats

echo Lancement du programme...
echo (Exécution complète des tests - cela peut prendre quelques minutes)
echo.

:: Lancer le programme et enregistrer la sortie
output\Exercice5.exe > resultats\resultats5.txt

echo.
echo ==============================================
echo     Tests termines. Resultats sauvegardes dans:
echo         resultats\resultats5.txt
echo ==============================================
echo.
pause
