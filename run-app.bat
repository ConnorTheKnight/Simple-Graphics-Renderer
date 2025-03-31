@echo off
ECHO Setting up Parallel OpenGL Shape Renderer with Docker and X Server
ECHO ===============================================================

ECHO Checking if X server is running...
tasklist | find /i "vcxsrv.exe" >nul 2>&1
IF ERRORLEVEL 1 (
    ECHO X server not found. Please start VcXsrv first.
    ECHO You can download it from: https://sourceforge.net/projects/vcxsrv/
    ECHO Start it with 'Multiple windows', 'Display number: 0', 'Start no client', 'Disable access control'
    PAUSE
    EXIT /B 1
)

ECHO Setting DISPLAY environment variable...
SET DISPLAY=127.0.0.1:0.0

ECHO Building the container (this may take a minute)...
docker-compose build --no-cache

ECHO Running the parallel shape renderer...
ECHO - Use 'c' to switch to Circle mode
ECHO - Use 's' to switch to Square mode
ECHO - Use 'r' to recalculate culling and grid
ECHO - Use '+' to increase thread count
ECHO - Use '-' to decrease thread count
ECHO - Use 'q' or ESC to quit
ECHO.

docker-compose up

PAUSE