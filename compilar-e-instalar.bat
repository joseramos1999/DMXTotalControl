@echo off
setlocal EnableExtensions
rem ===========================================================================
rem  DMX Total Control - compilar e instalar (Windows)
rem
rem  Uso:  compilar-e-instalar.bat            compila en Release e instala
rem        compilar-e-instalar.bat debug      compila en Debug e instala
rem
rem  Requisitos: MSYS2 en C:\msys64 con los paquetes del README e Inno Setup 6.
rem  Compila, crea el instalador (build\installer) y lo ejecuta: instala en
rem  %LOCALAPPDATA%\Programs\DMXTotalControl (no requiere admin), con accesos
rem  directos en el escritorio y en el menu Inicio, igual que el instalador
rem  que se publica en GitHub.
rem ===========================================================================

set "SRCDIR=%~dp0"
if "%SRCDIR:~-1%"=="\" set "SRCDIR=%SRCDIR:~0,-1%"
set "MSYS2=C:\msys64"
set "INSTALLDIR=%LOCALAPPDATA%\Programs\DMXTotalControl"
set "D2XXLIB=C:\projects\D2XXSDK\amd64\libftd2xx.a"

set "BUILD_TYPE=Release"
if /I "%~1"=="debug" set "BUILD_TYPE=Debug"

echo.
echo  ==========================================
echo   DMX Total Control - compilar e instalar
echo   Modo: %BUILD_TYPE%
echo  ==========================================
echo.

rem --- 1. Comprobaciones -----------------------------------------------------
if not exist "%MSYS2%\usr\bin\bash.exe" (
    echo [ERROR] No se encuentra MSYS2 en %MSYS2%.
    echo         Instalalo desde https://www.msys2.org/ y los paquetes del README.
    goto :fail
)
rem el entorno de MSYS2 con Qt: UCRT64 (el actual) o MINGW64 (el antiguo)
set "MSYSTEM="
if exist "%MSYS2%\ucrt64\bin\qmake6.exe" set "MSYSTEM=UCRT64"
if not defined MSYSTEM if exist "%MSYS2%\mingw64\bin\qmake6.exe" set "MSYSTEM=MINGW64"
if not defined MSYSTEM (
    echo [ERROR] Faltan Qt 6 u otros paquetes de MSYS2. Revisa el README.
    goto :fail
)
set "CHERE_INVOKING=1"

if not exist "%D2XXLIB%" (
    echo [1/4] Preparando el SDK D2XX de FTDI ^(interfaces DMX USB^)...
    "%MSYS2%\usr\bin\bash.exe" -lc "set -e; export PATH=$MINGW_PREFIX/bin:$PATH; mkdir -p /c/projects/D2XXSDK && cd /c/projects/D2XXSDK && curl -sSL -o cdm.zip https://qlcplus.org/misc/CDM-v2.12.36.20-WHQL-Certified.zip && unzip -o -q cdm.zip && cd amd64 && (command -v gendef >/dev/null || pacman -S --needed --noconfirm $MINGW_PACKAGE_PREFIX-tools-git) && gendef - ftd2xx64.dll > ftd2xx.def && dlltool -k --input-def ftd2xx.def --dllname ftd2xx64.dll --output-lib libftd2xx.a"
    if errorlevel 1 (
        echo [ERROR] No se pudo preparar el SDK D2XX.
        goto :fail
    )
) else (
    echo [1/4] SDK D2XX de FTDI: OK
)

rem --- 2. Compilar y crear el instalador -------------------------------------
echo [2/4] Compilando y creando el instalador... ^(la primera vez, o al cambiar de modo, tarda unos minutos^)
pushd "%SRCDIR%"
"%MSYS2%\usr\bin\bash.exe" -lc "BUILD_TYPE=%BUILD_TYPE% ./build-installer.sh"
set "BUILD_RESULT=%ERRORLEVEL%"
popd
if not "%BUILD_RESULT%"=="0" (
    echo.
    echo [ERROR] La compilacion o el instalador han fallado. Revisa los mensajes de arriba.
    echo         El instalador necesita Inno Setup 6: https://jrsoftware.org/isinfo.php
    goto :fail
)

rem el instalador recien creado (el mas nuevo de build\installer)
set "SETUP="
for /f "delims=" %%F in ('dir /b /o-d "%SRCDIR%\build\installer\DMXTotalControl-Setup-*.exe" 2^>nul') do (
    if not defined SETUP set "SETUP=%SRCDIR%\build\installer\%%F"
)
if not defined SETUP (
    echo [ERROR] No se encuentra el instalador en %SRCDIR%\build\installer.
    goto :fail
)

rem --- 3. Instalar -------------------------------------------------------------
echo [3/4] Instalando en %INSTALLDIR% ...
tasklist /FI "IMAGENAME eq dmxtotalcontrol.exe" 2>nul | find /I "dmxtotalcontrol.exe" >nul
if not errorlevel 1 (
    echo        Cerrando DMX Total Control, que esta abierto...
    taskkill /IM dmxtotalcontrol.exe /F >nul 2>&1
    timeout /t 2 /nobreak >nul
)

rem para el usuario, con el acceso directo del escritorio; los shows .qxw no
rem se asocian (el instalador lo ofrece al ejecutarlo a mano)
"%SETUP%" /SILENT /SUPPRESSMSGBOXES /NORESTART /CURRENTUSER /DIR="%INSTALLDIR%" /TASKS="desktopicon"
if errorlevel 1 (
    echo [ERROR] La instalacion ha fallado ^(codigo %ERRORLEVEL%^).
    goto :fail
)

rem --- 4. Listo ----------------------------------------------------------------
echo [4/4] Instalado. El instalador para otros equipos esta en:
echo    %SETUP%

echo.
echo  Listo. DMX Total Control instalado en:
echo    %INSTALLDIR%
echo  Puedes abrirlo desde el escritorio o el menu Inicio, y desinstalarlo
echo  desde Configuracion de Windows ^> Aplicaciones.
echo.
set /p "RUN=Abrir DMX Total Control ahora? [S/n] "
if /I not "%RUN%"=="n" start "" "%INSTALLDIR%\dmxtotalcontrol.exe"
endlocal
exit /b 0

:fail
echo.
pause
endlocal
exit /b 1
