@echo off
setlocal

set "PATH=C:\msys64\ucrt64\bin;%PATH%"

echo Building...
gcc -std=c23 -O2 -Wall -Wextra src/main.c src/map.c src/player.c src/renderer.c -o main.exe -lSDL3
if errorlevel 1 (
    echo Build FAILED. If main.exe is running, close it and try again.
    exit /b 1
)
echo Build success!
endlocal
