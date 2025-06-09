@echo off
setlocal enabledelayedexpansion

:: Parámetros
set NAME=%1
set "FILENAME=%NAME:_general_benchmark=%"

cd ..\..\versions\v2
set EXEC_DIR=..\..\executables\v2

:: Eliminar ejecutables si existen
if exist %EXEC_DIR%\%NAME%.exe del %EXEC_DIR%\%NAME%.exe

:: Compilar los ejecutables
g++ -std=c++17 -Iinclude "../../versions/common-params/src/json_parser.cpp" ".\src\empleado_v2.cpp" ".\src\turnos_v2.cpp" ".\src\solucion_v2.cpp" ".\src\general_utils_v2.cpp" ".\src\main_utils_v2.cpp" ".\src\nodoAstar_v2.cpp" ".\%FILENAME%.cpp" -o %EXEC_DIR%\%NAME%.exe
if %ERRORLEVEL% neq 0 (
    echo Error en la compilación de %NAME%.
    exit /b %ERRORLEVEL%
)

:: Verificar si los ejecutables se han generado correctamente
if not exist %EXEC_DIR%\%NAME%.exe (
    echo No se encontró %NAME%.exe después de la compilación.
    exit /b 1
)

cd %EXEC_DIR%

set OUTPUT=..\..\output\v2\%NAME%.json

echo [ > %OUTPUT%


echo Empezando las ejecuciones...
set /A ultimaIteracion=0
for %%D in (2 4 8 16 28) do (
    set DIA=%%D
    for %%E in (6 14) do (
        for %%D in (5 9) do (
            for %%L in (1 3) do (
                set "DEM_EARLY=%%E"
                set "DEM_DAY=%%D"
                set "DEM_LATE=%%L"
                set /A DEM=!DEM_EARLY!+!DEM_DAY!+!DEM_LATE!

                set /A numEnfermeras=!DEM!+15
                set /A finalL=!DIA!/4
                for /L %%L in (0,1,!finalL!) do (
                    set LS=%%L
                    set /A halfUS=!DIA!/2
                    for %%U in (!halfUS! !DIA!) do (
                        set /A US=%%U

                        :: Valores de los días consecutivos libres y trabajados
                        if !DIA! LSS 3 (
                            set MIN_LIBRES=0
                            set MAX_LIBRES=0
                        ) else if !DIA! LSS 7 (
                            set MIN_LIBRES=1
                            set MAX_LIBRES=1
                        ) else if !DIA! LSS 10 (
                            set MIN_LIBRES=2
                            set MAX_LIBRES=2
                        ) else (
                            set MIN_LIBRES=2
                            set MAX_LIBRES=3
                        )

                        set /A MIN_TRABAJADOS=!DIA!-3
                        if !MIN_TRABAJADOS! LEQ 0 set MIN_TRABAJADOS=1

                        set /A MAX_TRABAJADOS=!DIA!-!MIN_LIBRES!

                        echo ------Ejecutando: Enfermeras=!numEnfermeras!, DIAS=!DIA!, DEM_EARLY=!DEM_EARLY!, DEM_DAY=!DEM_DAY!, DEM_LATE=!DEM_LATE!, LS=!LS!, US=!US! MIN_LIBRES=!MIN_LIBRES! MAX_LIBRES=!MAX_LIBRES! MIN_TRABAJADOS=!MIN_TRABAJADOS! MAX_TRABAJADOS=!MAX_TRABAJADOS!

                        call .\%NAME%.exe !numEnfermeras! !DIA! !DEM_EARLY! !DEM_DAY! !DEM_LATE! !LS! !US! !MIN_LIBRES! !MAX_LIBRES! !MIN_TRABAJADOS! !MAX_TRABAJADOS! >> %OUTPUT%

                        if !DIA!==28 (
                            if !DEM!==26 (
                                if !LS!==!finalL! (
                                    if !US!==!DIA! (
                                        set /A ultimaIteracion=1
                                        echo ITERACIONES ACABADAS
                                        echo. >> %OUTPUT%
                                    )
                                )
                            )
                        )
                        if !ultimaIteracion!==0 (
                            echo , >> %OUTPUT%
                        )
                    )
                )
            )
        )
    )
)

echo ] >> %OUTPUT%

echo FINISHED EXECUTIONS!!!

exit /b
