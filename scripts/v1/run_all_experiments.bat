@echo off
goto :1
echo *EJECUTANDO: Backtracking...
call run_experiment.bat bruteForce

echo *EJECUTANDO: branchAndBound...
call run_experiment.bat branchAndBound

echo *EJECUTANDO: heuristic_piston...
call run_experiment.bat heuristic_piston

echo *EJECUTANDO: heuristic_Astar...
call run_experiment.bat heuristic_Astar

echo *EJECUTANDO: heuristic_Astar_2...
call run_experiment.bat heuristic_Astar_2

echo *EJECUTANDO: heuristic_Astar_3...
call run_experiment.bat heuristic_Astar_3
:1
echo *EJECUTANDO: heuristic_tabu_search...
call run_experiment.bat heuristic_tabuSearch

:: Obtener la fecha desde la variable de entorno
call ..\date.bat
for /f "tokens=*" %%F in ('powershell -command "echo $env:DATETIME"') do set DATETIME=%%F
set IMAGES_FOLDER=images---%DATETIME%
echo Carpeta creada: %IMAGES_FOLDER%

goto :2
call ..\postprocessing\postprocessing.bat bruteForce branchAndBound Backtracking-BranchAndBound %IMAGES_FOLDER% v1 0
call ..\postprocessing\postprocessing.bat branchAndBound heuristic_piston BranchAndBound-HeuristicPiston %IMAGES_FOLDER% v1 0
call ..\postprocessing\postprocessing.bat branchAndBound heuristic_Astar BranchAndBound-HeuristicAstar %IMAGES_FOLDER% v1 0
call ..\postprocessing\postprocessing.bat branchAndBound heuristic_Astar_2 BranchAndBound-HeuristicAstar2 %IMAGES_FOLDER% v1 0
call ..\postprocessing\postprocessing.bat heuristic_piston heuristic_Astar HeuristicPiston-HeuristicAstar %IMAGES_FOLDER% v1 1
call ..\postprocessing\postprocessing.bat heuristic_Astar heuristic_Astar_2 HeuristicAstar-HeuristicAstar2 %IMAGES_FOLDER% v1 1
:2
call ..\postprocessing\postprocessing.bat heuristic_Astar_2 heuristic_Astar_3 HeuristicAstar2-HeuristicAstar3 %IMAGES_FOLDER% v1 1
call ..\postprocessing\postprocessing.bat heuristic_Astar_3 heuristic_tabuSearch HeuristicAstar3-HeuristicTabuSearch %IMAGES_FOLDER% v1 1

echo TODOS LOS EXPERIMENTOS FINALIZADOS.
pause
