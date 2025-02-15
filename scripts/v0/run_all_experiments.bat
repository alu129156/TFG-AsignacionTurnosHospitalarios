@echo off
call ..\date.bat

:: Obtener la fecha desde la variable de entorno
for /f "tokens=*" %%F in ('powershell -command "echo $env:DATETIME"') do set DATETIME=%%F
set IMAGES_FOLDER=images---%DATETIME%
echo Carpeta creada: %IMAGES_FOLDER%

echo *EJECUTANDO: Backtracking vs Branch_AndBound_2...
call run_experiment.bat bruteForce branchAndBound_2 Backtracking-BranchAndBound2 %IMAGES_FOLDER%

echo *EJECUTANDO: BranchAndBound_1 vs BranchAndBound_2...
call run_experiment.bat branchAndBound_1 branchAndBound_2 BranchAndBound1-BranchAndBound2 %IMAGES_FOLDER%

echo TODOS LOS EXPERIMENTOS FINALIZADOS.
pause
