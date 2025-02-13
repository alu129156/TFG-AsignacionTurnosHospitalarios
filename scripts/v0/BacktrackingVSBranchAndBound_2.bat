@echo off
setlocal enabledelayedexpansion

cd ..\..\versions\v0

set EXEC_DIR=..\..\executables\v0

:: Eliminar ejecutables si existen
if exist %EXEC_DIR%\bruteForce.exe del %EXEC_DIR%\bruteForce.exe
if exist %EXEC_DIR%\branchAndBound.exe del %EXEC_DIR%\branchAndBound.exe

:: Compilar algoritmos
g++ ".\bruteForce.cpp" -o %EXEC_DIR%\bruteForce.exe
if %ERRORLEVEL% neq 0 (
    echo Error en la compilación de bruteForce.
    exit /b %ERRORLEVEL%
)

g++ ".\branchAndBound_2.cpp" -o %EXEC_DIR%\branchAndBound_2.exe
if %ERRORLEVEL% neq 0 (
    echo Error en la compilación de branchAndBound.
    exit /b %ERRORLEVEL%
)

:: Verificar si los ejecutables se han generado correctamente
if not exist %EXEC_DIR%\bruteForce.exe (
    echo No se encontró bruteForce.exe después de la compilación.
    exit /b 1
)
if not exist %EXEC_DIR%\branchAndBound.exe (
    echo No se encontró branchAndBound.exe después de la compilación.
    exit /b 1
)

cd %EXEC_DIR%

:: Archivos de salida
set BF_OUTPUT=..\..\output\v0\Backtracking-BranchAndBound\bruteForce.json
set BB_OUTPUT=..\..\output\\v0\Backtracking-BranchAndBound\branchAndBound.json


echo [ >> %BB_OUTPUT%
echo [ >> %BF_OUTPUT%


echo Empezando las ejecuciones de bucle...
set /A ultimaIteracion=0
for %%D in (2 3 4 5) do (
    set DIA=%%D
    for %%M in (2 3) do (
        set M=%%M
        set /A numEnfermeras=3*!M! + 1
        set /A maxL=!DIA!+1
        for /L %%L in (1,1,!maxL!) do (
            set L=%%L
            set /A RANGE=!DIA!-!L!+1
            if !RANGE! LEQ 0 set RANGE=1
            set /A RAND_US=!RANDOM! %% !RANGE!
            set /A US=!L! + !RAND_US!

            echo ------Ejecutando: Enfermeras=!numEnfermeras!, DIAS=!DIA!, DEMANDA=!M!, LS=!L!, US=!US!

            :: Ejecutar branchAndBound y guardar la salida en un archivo temporal
            call .\branchAndBound.exe !numEnfermeras! !DIA! !M! !L! !US! >> %BB_OUTPUT%
            set bab_exit_code=!ERRORLEVEL!
            if !bab_exit_code! neq 0 (
                echo Branch and Bound ha excedido del tiempo maximo
            )
            call .\bruteForce.exe !numEnfermeras! !DIA! !M! !L! !US! >>  %BF_OUTPUT%
            set bf_exit_code=!ERRORLEVEL!
            if !bf_exit_code! neq 0 (
                echo Brute Force ha excedido del tiempo maximo
            )

            if !DIA!==5 (
                if !M!==3 (
                    if !L!==!maxL! (
                        set /A ultimaIteracion=1
                        echo ITERACIONES ACABADAS
                        echo. >> %BB_OUTPUT%
                        echo. >> %BF_OUTPUT%
                    )
                )
            )
            if !ultimaIteracion!==0 (
                echo , >> %BB_OUTPUT%
                echo , >> %BF_OUTPUT%
            )
        )
    )
)

echo ] >> %BB_OUTPUT%
echo ] >> %BF_OUTPUT%

echo Ejecuciones terminadas, ejecutando el postprocesado...
cd ..\..\scripts\v0\postprocessing

:: Activar el entorno virtual y ejecutar el script
call ..\..\..\venv\Scripts\activate
python postprocessing_script.py ..\%BF_OUTPUT% ..\%BB_OUTPUT%
deactivate

echo ¡SCRIPT FINALIZADO!
pause