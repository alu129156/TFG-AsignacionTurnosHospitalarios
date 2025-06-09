@echo off
goto :1
echo *EJECUTANDO: Backtracking...
call run_experiment.bat bruteForce

echo *EJECUTANDO: branchAndBound...
call run_experiment.bat branchAndBound

echo *EJECUTANDO: heuristic_Astar...
call run_experiment.bat heuristic_Astar
:1
echo *EJECUTANDO: heuristic_Astar_2...
call run_experiment.bat heuristic_Astar_2
goto :3
echo *EJECUTANDO: heuristic_Astar_3...
call run_experiment.bat heuristic_Astar_3

echo *EJECUTANDO: heuristic_tabu_search...
call run_experiment.bat heuristic_tabuSearch
:3

:: Obtener la fecha desde la variable de entorno
call ..\date.bat
for /f "tokens=*" %%F in ('powershell -command "echo $env:DATETIME"') do set DATETIME=%%F
set IMAGES_FOLDER=images---%DATETIME%
echo Carpeta creada: %IMAGES_FOLDER%

call ..\postprocessing\postprocessing.bat heuristic_Astar heuristic_Astar_2 HeuristicAstar-HeuristicAstar2 %IMAGES_FOLDER% v1 1 0
call ..\postprocessing\postprocessing.bat heuristic_Astar_2 heuristic_Astar_3 HeuristicAstar2-HeuristicAstar3 %IMAGES_FOLDER% v1 1 0
call ..\postprocessing\postprocessing.bat heuristic_Astar_2 heuristic_tabuSearch HeuristicAstar2-HeuristicTabuSearch %IMAGES_FOLDER% v1 1 0
call ..\postprocessing\postprocessing.bat heuristic_Astar_3 heuristic_tabuSearch HeuristicAstar3-HeuristicTabuSearch %IMAGES_FOLDER% v1 1 0

echo TODOS LOS EXPERIMENTOS FINALIZADOS.
pause
