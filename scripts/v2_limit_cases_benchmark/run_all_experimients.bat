@echo off
setlocal

echo === Ejecutando casos limite para tabuSearch ===
call run_limit_cases.bat heuristic_tabuSearch

echo === Ejecutando casos limite para Astar_3 ===
call run_limit_cases.bat heuristic_Astar_3

echo === Benchmark de casos limite completado ===

:: Obtener la fecha desde la variable de entorno
call ..\date.bat
for /f "tokens=*" %%F in ('powershell -command "echo $env:DATETIME"') do set DATETIME=%%F
set IMAGES_FOLDER=images---%DATETIME%
echo Carpeta redireccionada: %IMAGES_FOLDER%

call ..\postprocessing\postprocessing.bat heuristic_Astar_3_limit_cases_benchmark heuristic_tabuSearch_limit_cases_benchmark HeuristicAstar3-HeuristicTabuSearch_limit_cases_benchmark %IMAGES_FOLDER% v2 0 1

echo TODOS LOS EXPERIMENTOS FINALIZADOS.
pause
