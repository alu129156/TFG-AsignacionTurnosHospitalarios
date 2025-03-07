@echo off

echo *EJECUTANDO: Backtracking...
call run_experiment.bat bruteForce
echo *EJECUTANDO: branchAndBound_1...
call run_experiment.batbranchAndBound_1
echo *EJECUTANDO: branchAndBound_2...
call run_experiment.bat branchAndBound_2

:: Obtener la fecha desde la variable de entorno
call ..\date.bat
for /f "tokens=*" %%F in ('powershell -command "echo $env:DATETIME"') do set DATETIME=%%F
set IMAGES_FOLDER=images---%DATETIME%

call ..\postprocessing\postprocessing.bat bruteForce branchAndBound_1 Backtracking-BranchAndBound2 %IMAGES_FOLDER% v0 0
call ..\postprocessing\postprocessing.bat branchAndBound_1 branchAndBound_2 BranchAndBound1-BranchAndBound2 %IMAGES_FOLDER% v0 0

echo TODOS LOS EXPERIMENTOS FINALIZADOS.
pause
