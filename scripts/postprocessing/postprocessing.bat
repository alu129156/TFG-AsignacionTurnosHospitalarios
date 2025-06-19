set NAME1=%1
set NAME2=%2
set OUTPUT_DIR=%3
set IMAGE_FOLDER_NAME=%4
set VERSION=%5
set EXEC_GRAPH=%6
set IS_LIMIT_CASES_BENCH=%7

cd ..\..\scripts\postprocessing
echo Ejecutando postprocesamiento en Python...

set OUTPUT1=..\..\output\%VERSION%\%NAME1%.json
set OUTPUT2=..\..\output\%VERSION%\%NAME2%.json
set OUTPUT_IMAGE_FOLDER=..\..\output\%VERSION%\%OUTPUT_DIR%\%IMAGE_FOLDER_NAME%
if not exist %OUTPUT_IMAGE_FOLDER% (
    mkdir %OUTPUT_IMAGE_FOLDER%
    echo Carpeta creada: %OUTPUT_IMAGE_FOLDER%
)

:: Activar venv y ejecutar scripts
call ..\..\venv\Scripts\activate
if %VERSION%==v0 if %EXEC_GRAPH% neq 1 (
    python -B heatmap_and_percentages_tables.py %OUTPUT_IMAGE_FOLDER% %OUTPUT1% %OUTPUT2%
)

if %EXEC_GRAPH%==1 (
    python -B heuristic_comparison_graph.py %OUTPUT_IMAGE_FOLDER% %OUTPUT1% %OUTPUT2%
    python -B heuristic_comparison_graph_coloured_variables.py %OUTPUT_IMAGE_FOLDER% %OUTPUT1% %OUTPUT2% days
    python -B heuristic_comparison_graph_coloured_variables.py %OUTPUT_IMAGE_FOLDER% %OUTPUT1% %OUTPUT2% demand
)

if %VERSION%==v2 (
    if %IS_LIMIT_CASES_BENCH%==0 (
        python -B heuristic_comparison_split_by_days.py %OUTPUT_IMAGE_FOLDER% %OUTPUT1% %OUTPUT2%
    )
)

if %IS_LIMIT_CASES_BENCH%==1 (
    python -B heuristic_sol-nosol_algorithm_graph.py %OUTPUT_IMAGE_FOLDER% %OUTPUT1%
    python -B heuristic_sol-nosol_algorithm_graph.py %OUTPUT_IMAGE_FOLDER% %OUTPUT2%
)

deactivate
exit /b