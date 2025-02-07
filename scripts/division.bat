@echo off
:: Este script realiza divisiones con decimales.
:: Uso: division.bat dividendo divisor
:: Devuelve: el resultado de la división por pantalla

:: Captura los argumentos
set dividendo=%1
set divisor=%2
set decimales=0

:: Calcular la parte entera
set /a "entero=%dividendo%/%divisor%"
set /a "resto0=%dividendo%%%%divisor%"
echo entro script

:div_decimal
if [!resto%decimales%!]==[0] goto div_fin
set /a sig=%decimales%+1
set /a decimal%decimales%=!resto%decimales%!0/%divisor%
set /a resto%sig%=!resto%decimales%!0%%%divisor%
set /a "decimales+=1"
goto div_decimal

:div_fin
:: Concatenar parte entera y decimal
for /l %%a in (0,1,%decimales%) do (
    set final=!final!!decimal%%a!
)
:: Mostrar solo el resultado
echo %entero%.%final%
exit /b
