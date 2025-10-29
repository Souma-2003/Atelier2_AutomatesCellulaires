@echo off
echo ==============================================
echo       Run - Exercice 6 : AC_HASH 
echo ==============================================
echo.

:: Vérifie si le fichier compilé existe
if not exist output\Exercice6.exe (
    echo [ERREUR] Le fichier output\Exercice6.exe est introuvable.
    echo Veuillez compiler Exercice6.cpp avant de lancer ce test.
    pause
    exit /b
)

:: Dossier de sortie
if not exist resultats mkdir resultats

echo Lancement du programme...
echo (Exécution complète des tests - cela peut prendre quelques minutes)
echo.

:: Lancer le programme et enregistrer la sortie
output\Exercice6.exe > resultats\resultats6.txt

echo.
echo ==============================================
echo     Tests termines. Resultats sauvegardes dans:
echo         resultats\resultats6.txt
echo ==============================================
echo.
pause
