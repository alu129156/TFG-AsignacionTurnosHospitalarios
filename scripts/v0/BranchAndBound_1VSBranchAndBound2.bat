@echo off
setlocal enabledelayedexpansion

cd ..\..\versions\v0

set EXEC_DIR=..\..\executables\v0
set OUTPUT_IMAGES_FOLDER=..\..\output\v0\BranchAndBound_1-BranchAndBound_2\images

:: Eliminar ejecutables si existen
if exist %EXEC_DIR%\branchAndBound_1.exe del %EXEC_DIR%\branchAndBound_1.exe
if exist %EXEC_DIR%\branchAndBound_2.exe del %EXEC_DIR%\branchAndBound_2.exe

:: Compilar algoritmos
g++ ".\branchAndBound_1.cpp" -o %EXEC_DIR%\branchAndBound_1.exe
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
if not exist %EXEC_DIR%\branchAndBound_1.exe (
    echo No se encontró bruteForce.exe después de la compilación.
    exit /b 1
)
if not exist %EXEC_DIR%\branchAndBound_2.exe (
    echo No se encontró branchAndBound.exe después de la compilación.
    exit /b 1
)

cd %EXEC_DIR%

if exist %OUTPUT_IMAGES_FOLDER% (
    rmdir /s /q %OUTPUT_IMAGES_FOLDER%
)
mkdir %OUTPUT_IMAGES_FOLDER%

:: Archivos de salida
set BB1_OUTPUT=..\..\output\v0\BranchAndBound_1-BranchAndBound_2\branchAndBound_1.json
set BB2_OUTPUT=..\..\output\v0\BranchAndBound_1-BranchAndBound_2\branchAndBound_2.json


echo [ >> %BB1_OUTPUT%
echo [ >> %BB2_OUTPUT%


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

            call .\branchAndBound_1.exe !numEnfermeras! !DIA! !M! !L! !US! >> %BB1_OUTPUT%
            set bab_exit_code=!ERRORLEVEL!
            if !bab_exit_code! neq 0 (
                echo Branch and Bound ha excedido del tiempo maximo
            )
            call .\branchAndBound_2.exe !numEnfermeras! !DIA! !M! !L! !US! >>  %BB2_OUTPUT%
            set bf_exit_code=!ERRORLEVEL!
            if !bf_exit_code! neq 0 (
                echo Brute Force ha excedido del tiempo maximo
            )

            if !DIA!==5 (
                if !M!==3 (
                    if !L!==!maxL! (
                        set /A ultimaIteracion=1
                        echo ITERACIONES ACABADAS
                        echo. >> %BB1_OUTPUT%
                        echo. >> %BB2_OUTPUT%
                    )
                )
            )
            if !ultimaIteracion!==0 (
                echo , >> %BB1_OUTPUT%
                echo , >> %BB2_OUTPUT%
            )
        )
    )
)

echo ] >> %BB1_OUTPUT%
echo ] >> %BB2_OUTPUT%

echo Ejecuciones terminadas, ejecutando el postprocesado...
cd ..\..\scripts\v0\postprocessing

:: Activar el entorno virtual y ejecutar el script
call ..\..\..\venv\Scripts\activate
python postprocessing_script.py ..\%OUTPUT_IMAGES_FOLDER% ..\%BB1_OUTPUT% ..\%BB2_OUTPUT%
deactivate

echo ¡SCRIPT FINALIZADO!
pause