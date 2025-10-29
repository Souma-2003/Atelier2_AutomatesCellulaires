@echo off
echo ==============================================
echo       Run - Exercice 7 : AC_HASH 
echo ==============================================
echo.

:: Vérifie si le fichier compilé existe
if not exist output\Exercice7.exe (
    echo [ERREUR] Le fichier output\Exercice7.exe est introuvable.
    echo Veuillez compiler Exercice7.cpp avant de lancer ce test.
    pause
    exit /b
)

:: Dossier de sortie
if not exist resultats mkdir resultats

echo Lancement du programme...
echo (Exécution complète des tests - cela peut prendre quelques minutes)
echo.

:: Lancer le programme et enregistrer la sortie
output\Exercice7.exe > resultats\resultats7.txt

echo.
echo ==============================================
echo     Tests termines. Resultats sauvegardes dans:
echo         resultats\resultats7.txt
echo ==============================================
echo.
pause
