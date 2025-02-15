@echo off
setlocal enabledelayedexpansion

:: Parámetros
set NAME1=%1
set NAME2=%2
set OUTPUT_DIR=%3
set IMAGE_FOLDER_NAME=%4


cd ..\..\versions\v0
set EXEC_DIR=..\..\executables\v0
if not exist ..\..\output\v0\%OUTPUT_DIR%\%IMAGE_FOLDER_NAME% (
    mkdir ..\..\output\v0\%OUTPUT_DIR%\%IMAGE_FOLDER_NAME%
)
set OUTPUT_IMAGE_FOLDER=..\..\output\v0\%OUTPUT_DIR%\%IMAGE_FOLDER_NAME%

:: Eliminar ejecutables si existen
if exist %EXEC_DIR%\%NAME1%.exe del %EXEC_DIR%\%NAME1%.exe
if exist %EXEC_DIR%\%NAME2%.exe del %EXEC_DIR%\%NAME2%.exe

:: Compilar los ejecutables
g++ ".\%NAME1%.cpp" -o %EXEC_DIR%\%NAME1%.exe
if %ERRORLEVEL% neq 0 (
    echo Error en la compilación de %NAME1%.
    exit /b %ERRORLEVEL%
)

g++ ".\%NAME2%.cpp" -o %EXEC_DIR%\%NAME2%.exe
if %ERRORLEVEL% neq 0 (
    echo Error en la compilación de %NAME2%.
    exit /b %ERRORLEVEL%
)

:: Verificar si los ejecutables se han generado correctamente
if not exist %EXEC_DIR%\%NAME1%.exe (
    echo No se encontró %NAME1%.exe después de la compilación.
    exit /b 1
)
if not exist %EXEC_DIR%\%NAME2%.exe (
    echo No se encontró %NAME2%.exe después de la compilación.
    exit /b 1
)

cd %EXEC_DIR%

:: Archivos de salida
set OUTPUT1=..\..\output\v0\%OUTPUT_DIR%\%NAME1%.json
set OUTPUT2=..\..\output\v0\%OUTPUT_DIR%\%NAME2%.json
goto :fin
echo [ > %OUTPUT1%
echo [ > %OUTPUT2%

:: Ejecuciones en bucle
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
            set /A RANGE=!DIA!-!L!+1
            if !RANGE! LEQ 0 set RANGE=1
            set /A RAND_US=!RANDOM! %% !RANGE!
            set /A US=!L! + !RAND_US!

            echo ------Ejecutando: Enfermeras=!numEnfermeras!, DIAS=!DIA!, DEMANDA=!M!, LS=!L!, US=!US!

            call .\%NAME1%.exe !numEnfermeras! !DIA! !M! !L! !US! >> %OUTPUT1%
            call .\%NAME2%.exe !numEnfermeras! !DIA! !M! !L! !US! >> %OUTPUT2%

            if !DIA!==5 (
                if !M!==3 (
                    if !L!==!maxL! (
                        set /A ultimaIteracion=1
                        echo ITERACIONES ACABADAS
                        echo. >> %OUTPUT1%
                        echo. >> %OUTPUT2%
                    )
                )
            )
            if !ultimaIteracion!==0 (
                echo , >> %OUTPUT1%
                echo , >> %OUTPUT2%
            )
        )
    )
)

echo ] >> %OUTPUT1%
echo ] >> %OUTPUT2%

echo FINISHED SCRIPT!!!
:fin
cd ..\..\scripts\v0\postprocessing
echo Ejecutando postprocesamiento en Python...

:: Activar venv y ejecutar script
call ..\..\..\venv\Scripts\activate
python postprocessing_script.py ..\%OUTPUT_IMAGE_FOLDER% ..\%OUTPUT1% ..\%OUTPUT2%
deactivate

echo Postprocesamiento finalizado.
pause
