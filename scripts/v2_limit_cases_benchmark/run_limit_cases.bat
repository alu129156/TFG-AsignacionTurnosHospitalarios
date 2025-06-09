@echo off
setlocal enabledelayedexpansion

set NAME=%1

cd ..\..\versions\v2
set EXEC_DIR=..\..\executables\v2
set OUTPUT=..\..\output\v2\%NAME%_limit_cases_benchmark.json

if exist %EXEC_DIR%\%NAME%.exe del %EXEC_DIR%\%NAME%.exe

g++ -std=c++17 -Iinclude "../common-params/src/json_parser.cpp" ^
    ".\src\empleado_v2.cpp" ".\src\turnos_v2.cpp" ".\src\solucion_v2.cpp" ^
    ".\src\general_utils_v2.cpp" ".\src\main_utils_v2.cpp" ".\src\nodoAstar_v2.cpp" ^
    ".\%NAME%.cpp" -o %EXEC_DIR%\%NAME%_limit_bench.exe

if %ERRORLEVEL% neq 0 (
    echo Error en la compilación de %NAME%.
    exit /b %ERRORLEVEL%
)

cd %EXEC_DIR%
echo [ > %OUTPUT%

:: Casos límite individuales --> 1 variable muy alta
for %%V in (DIA EMPLEADOS) do (
    if %%V==DIA (
        set DIA=100
        set NUM=14
        set DEMANDA_PORECENTAJE=50
    ) else if %%V==EMPLEADOS (
        set DIA=14
        set NUM=300
        set DEMANDA_PORECENTAJE=5
    )

    call :ejecutarCaso !DIA! !NUM! !DEMANDA_PORECENTAJE! 0
)



:: Caso doble
call :ejecutarCaso 100 300 2 0

:: Casos extremos
call :ejecutarCaso 100 100 100 0
call :ejecutarCaso 200 150 80 1

echo ] >> %OUTPUT%
echo BENCHMARK COMPLETADO
exit /b

:ejecutarCaso
set DIA=%1
set NUM_ENFERMERAS=%2
set PORCENTAJE_DEMANDA=%3
set FIN=%4

:: Demanda total: 60% de las enfermeras
set /A DEMANDA_TOTAL=(%NUM_ENFERMERAS%*%PORCENTAJE_DEMANDA%)/100

:: Distribución de la demanda por tipo de turno
set /A DEM_LATE=(%DEMANDA_TOTAL%*20)/100
set /A DEM_DAY=(%DEMANDA_TOTAL%*45)/100
set /A DEM_EARLY=%DEMANDA_TOTAL% - %DEM_LATE% - %DEM_DAY%

:: Restricciones proporcionales a la duración
set /A LS=(%DIA%/8) + 1
set /A US=(%DIA%*80)/100
if !US! LSS !LS! set /A US=!LS!+1

:: Días consecutivos libres
if %DIA% LEQ 3 (
    set MIN_LIBRES=0
    set MAX_LIBRES=0
) else if %DIA% LEQ 6 (
    set MIN_LIBRES=1
    set MAX_LIBRES=1
) else if %DIA% LEQ 10 (
    set MIN_LIBRES=2
    set MAX_LIBRES=2
) else if %DIA% LEQ 20 (
    set MIN_LIBRES=2
    set MAX_LIBRES=3
) else if %DIA% LEQ 50 (
    set MIN_LIBRES=3
    set MAX_LIBRES=4
) else (
    set MIN_LIBRES=4
    set MAX_LIBRES=5
)

:: Días consecutivos trabajados (razonado por carga laboral hospitalaria)
if %DIA% LEQ 3 (
    set MIN_TRABAJADOS=1
    set MAX_TRABAJADOS=2
) else if %DIA% LEQ 6 (
    set MIN_TRABAJADOS=2
    set MAX_TRABAJADOS=3
) else if %DIA% LEQ 10 (
    set MIN_TRABAJADOS=2
    set MAX_TRABAJADOS=4
) else if %DIA% LEQ 20 (
    set MIN_TRABAJADOS=3
    set MAX_TRABAJADOS=5
) else if %DIA% LEQ 50 (
    set MIN_TRABAJADOS=3
    set MAX_TRABAJADOS=6
) else (
    set MIN_TRABAJADOS=4
    set MAX_TRABAJADOS=7
)

echo Ejecutando Benchmark De Caso Limite: DIAS=!DIA! DEMANDA_TOTAL=!DEMANDA_TOTAL! ENFERMERAS=!NUM_ENFERMERAS!

call %EXEC_DIR%\%NAME%_limit_bench.exe !NUM_ENFERMERAS! !DIA! !DEM_EARLY! !DEM_DAY! !DEM_LATE! !LS! !US! !MIN_LIBRES! !MAX_LIBRES! !MIN_TRABAJADOS! !MAX_TRABAJADOS! >> %OUTPUT%
if !FIN!==1 (
    echo. >> %OUTPUT%
) else (
    echo , >> %OUTPUT%
)

exit /b