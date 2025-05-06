@echo off
setlocal enabledelayedexpansion

:: Parámetros
set NAME=%1


cd ..\..\versions\v0
set EXEC_DIR=..\..\executables\v0

:: Eliminar ejecutables si existen
if exist %EXEC_DIR%\%NAME%.exe del %EXEC_DIR%\%NAME%.exe

:: Compilar los ejecutables
g++ -std=c++17 -Iinclude ".\src\empleado.cpp" ".\src\solucion.cpp" ".\src\general_utils.cpp" ".\src\main_utils.cpp" ".\src\nodoAstar.cpp" ".\%NAME%.cpp" -o %EXEC_DIR%\%NAME%.exe
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

set OUTPUT=..\..\output\v0\%NAME%.json

echo [ > %OUTPUT%


echo Empezando las ejecuciones...
set /A ultimaIteracion=0
for %%D in (2 3 4 5) do (
    set DIA=%%D
    for %%M in (2 3) do (
        set M=%%M
        set /A numEnfermeras=3*!M! + 1
        set /A maxL=!DIA!+1
        for /L %%L in (1,1,!maxL!) do (
            set L=%%L
            set /A RANGE=!DIA!-!L!
            if !RANGE! LEQ 0 set RANGE=1
            set /A PLUS_US=!RANGE! %% 2
            set /A US=!L! + !PLUS_US!

            echo ------Ejecutando: Enfermeras=!numEnfermeras!, DIAS=!DIA!, DEMANDA=!M!, LS=!L!, US=!US!

            call .\%NAME%.exe !numEnfermeras! !DIA! !M! !L! !US! >> %OUTPUT%


            if !DIA!==5 (
                if !M!==3 (
                    if !L!==!maxL! (
                        set /A ultimaIteracion=1
                        echo ITERACIONES ACABADAS
                        echo. >> %OUTPUT%
                    )
                )
            )
            if !ultimaIteracion!==0 (
                echo , >> %OUTPUT%
            )
        )
    )
)

echo ] >> %OUTPUT%

echo FINISHED EXECUTIONS!!!

exit /b
