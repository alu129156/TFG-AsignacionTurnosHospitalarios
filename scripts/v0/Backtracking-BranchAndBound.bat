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

g++ ".\branchAndBound.cpp" -o %EXEC_DIR%\branchAndBound.exe
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
set BF_OUTPUT=..\..\output\v0\Backtracking-BranchAndBound\bruteForce.txt
set BB_OUTPUT=..\..\output\\v0\Backtracking-BranchAndBound\branchAndBound.txt
set ITERATIONS_OUTPUT=..\..\output\\v0\Backtracking-BranchAndBound\iterationsBFvsBaB.txt

:: Limpiar los archivos
echo RESULTADOS EJECUCIONES BRUTE FORCE > %BF_OUTPUT%
echo RESULTADOS EJECUCIONES BRANCH AND BOUND > %BB_OUTPUT%
echo PORCENTAJES DE ITERACIONES (BRANCHandBOUND/BACKTRACKING) > %ITERATIONS_OUTPUT%

echo Empezando las ejecuciones de bucle...
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
            echo INPUT: [Enfermeras=!numEnfermeras!, DIAS=!DIA!, DEMANDA=!M!, LS=!L!, US=!US!] >> %BF_OUTPUT%
            echo INPUT: [Enfermeras=!numEnfermeras!, DIAS=!DIA!, DEMANDA=!M!, LS=!L!, US=!US!] >> %BB_OUTPUT%

            :: Ejecutar branchAndBound y guardar la salida en un archivo temporal
            set status_output=0
            call .\branchAndBound.exe !numEnfermeras! !DIA! !M! !L! !US! > branchAndBound_tmp.txt
            set bab_exit_code=!ERRORLEVEL!
            if !bab_exit_code! neq 0 (
                echo Branch and Bound ha excedido del tiempo maximo de 7 mins
                set status_output=1
            )
            call .\bruteForce.exe !numEnfermeras! !DIA! !M! !L! !US! > bruteForce_tmp.txt
            set bf_exit_code=!ERRORLEVEL!
            if !bf_exit_code! neq 0 (
                echo Brute Force ha excedido del tiempo maximo de 7 mins
                set status_output=1
            )

            :: Las iteraciones solo si los dos programas acaban bien
            if !status_output! == 0 (
                set itBaB=0
                set it=0

                :: Leer el número de iteraciones de Branch and Bound (Solo en la línea 3)
                set lineNumber=0
                for /F "delims=" %%i in (branchAndBound_tmp.txt) do (
                    set /A lineNumber+=1
                    if !lineNumber! EQU 3 (
                        set "line=%%i"
                        set "line=!line:	=!"  :: Quitar tabulaciones
                        for /F "tokens=2 delims=:" %%j in ("!line!") do (
                            set itBaB=%%j
                            set itBaB=!itBaB: =!
                            set itBaB=!itBaB:,=!
                        )
                    )
                )
                :: Leer el número de iteraciones de Brute Force (Solo en la línea 3)
                set lineNumber=0
                for /F "delims=" %%i in (bruteForce_tmp.txt) do (
                    set /A lineNumber+=1
                    if !lineNumber! EQU 3 (
                        set "line=%%i"
                        set "line=!line:	=!"
                        for /F "tokens=2 delims=:" %%j in ("!line!") do (
                            set it=%%j
                            set it=!it: =!
                            set it=!it:,=!
                        )
                    )
                )

                echo it: !it!, bAb: !itBaB!
                echo INPUT: [Enfermeras=!numEnfermeras!, DIAS=!DIA!, DEMANDA=!M!, LS=!L!, US=!US!] == !itBaB!/!it! >> %ITERATIONS_OUTPUT%
            )

            type bruteForce_tmp.txt >> %BF_OUTPUT%
            type branchAndBound_tmp.txt >> %BB_OUTPUT%
        )
    )
)

if exist bruteForce_tmp.txt del bruteForce_tmp.txt
if exist branchAndBound_tmp.txt del branchAndBound_tmp.txt

echo EJECUCIONES TERMINADAS.
pause