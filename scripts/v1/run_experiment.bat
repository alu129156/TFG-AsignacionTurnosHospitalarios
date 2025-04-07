@echo off
setlocal enabledelayedexpansion

:: Parámetros
set NAME=%1


cd ..\..\versions\v1
set EXEC_DIR=..\..\executables\v1

:: Eliminar ejecutables si existen
if exist %EXEC_DIR%\%NAME%.exe del %EXEC_DIR%\%NAME%.exe

:: Compilar los ejecutables
g++ ".\%NAME%.cpp" -o %EXEC_DIR%\%NAME%.exe
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

set OUTPUT=..\..\output\v1\%NAME%.json

echo [ > %OUTPUT%


echo Empezando las ejecuciones...
set /A ultimaIteracion=0
for %%D in (2 3 4 5 6 7 8 9 10 11) do (
    set DIA=%%D
    for %%M in (2 3 4) do (
        set M=%%M
        set /A numEnfermeras=3*!M! + 5
        set /A finalL=!DIA!-1
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

                echo ------Ejecutando: Enfermeras=!numEnfermeras!, DIAS=!DIA!, DEMANDA=!M!, LS=!LS!, US=!US! MIN_LIBRES=!MIN_LIBRES! MAX_LIBRES=!MAX_LIBRES! MIN_TRABAJADOS=!MIN_TRABAJADOS! MAX_TRABAJADOS=!MAX_TRABAJADOS!

                call .\%NAME%.exe !numEnfermeras! !DIA! !M! !LS! !US! !MIN_LIBRES! !MAX_LIBRES! !MIN_TRABAJADOS! !MAX_TRABAJADOS! >> %OUTPUT%

                if !DIA!==11 (
                    if !M!==4 (
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

echo ] >> %OUTPUT%

echo FINISHED EXECUTIONS!!!

exit /b
