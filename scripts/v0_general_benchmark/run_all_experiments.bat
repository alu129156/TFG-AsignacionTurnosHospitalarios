@echo off

echo *EJECUTANDO: Backtracking...
call run_experiment.bat bruteForce

echo *EJECUTANDO: branchAndBound_1...
call run_experiment.bat branchAndBound_1
echo *EJECUTANDO: branchAndBound_2...
call run_experiment.bat branchAndBound_2

echo *EJECUTANDO: heuristic_Astar_3...
call run_experiment.bat heuristic_Astar_3

echo *EJECUTANDO: heuristic_tabuSearch...
call run_experiment.bat heuristic_tabuSearch

:: Obtener la fecha desde la variable de entorno
call ..\date.bat
for /f "tokens=*" %%F in ('powershell -command "echo $env:DATETIME"') do set DATETIME=%%F
set IMAGES_FOLDER=images---%DATETIME%

call ..\postprocessing\postprocessing.bat bruteForce branchAndBound_1 Backtracking-BranchAndBound1 %IMAGES_FOLDER% v0 0 0
call ..\postprocessing\postprocessing.bat branchAndBound_1 branchAndBound_2 BranchAndBound1-BranchAndBound2 %IMAGES_FOLDER% v0 0 0
call ..\postprocessing\postprocessing.bat heuristic_Astar_3 heuristic_tabuSearch Astar-TabuSearch %IMAGES_FOLDER% v0 1 0

echo TODOS LOS EXPERIMENTOS FINALIZADOS.
pause
