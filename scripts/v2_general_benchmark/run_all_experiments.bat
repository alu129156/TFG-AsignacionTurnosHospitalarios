@echo off
goto :3
echo *EJECUTANDO: heuristic_Astar_3...
call run_experiment.bat heuristic_Astar_3_general_benchmark

echo *EJECUTANDO: heuristic_tabuSearch...
call run_experiment.bat heuristic_tabuSearch_general_benchmark
:3
:: Obtener la fecha desde la variable de entorno
call ..\date.bat
for /f "tokens=*" %%F in ('powershell -command "echo $env:DATETIME"') do set DATETIME=%%F
set IMAGES_FOLDER=images---%DATETIME%
echo Carpeta creada: %IMAGES_FOLDER%

call ..\postprocessing\postprocessing.bat heuristic_Astar_3_general_benchmark heuristic_tabuSearch_general_benchmark HeuristicAstar2-HeuristicTabuSearch_general_benchmark %IMAGES_FOLDER% v2 1 0

echo TODOS LOS EXPERIMENTOS FINALIZADOS.
pause
