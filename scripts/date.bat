@echo off
for /f "tokens=1-3 delims=/ " %%A in ("%DATE%") do (
    set DAY=%%A
    set MONTH=%%B
    set YEAR=%%C
)

for /f "tokens=1-2 delims=:." %%D in ("%TIME%") do (
    set HOUR=%%D
    set MINUTE=%%E
)

set DATETIME=%YEAR%-%MONTH%-%DAY%_%HOUR%-%MINUTE%
set DATETIME=%DATETIME: =%

:: Variable de entorno
setx DATETIME %DATETIME%

exit /b
