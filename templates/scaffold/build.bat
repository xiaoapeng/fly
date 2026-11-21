@echo off
setlocal EnableExtensions DisableDelayedExpansion

for %%I in ("%~dp0.") do set "project_dir=%%~fI"
set "fly_dir=%FLY_ROOT%"
set "use_managed="
if defined fly_dir goto :dispatch
if exist "%project_dir%\fly\" (
    set "fly_dir=%project_dir%\fly"
    set "use_managed=1"
    goto :dispatch
)
if exist "%project_dir%\.fly-root" set /p "fly_dir=" < "%project_dir%\.fly-root"

:dispatch
if "%~1"=="setup" goto :setup
if not defined fly_dir goto :missing_sdk
if not exist "%fly_dir%\tool\python\build.py" goto :missing_sdk
if not exist "%fly_dir%\cmake\fly-configure.cmake" (
    >&2 echo This app needs a FLY SDK with application-root CMake. Update fly.lock and run setup, or set FLY_ROOT.
    exit /b 1
)
call :find_python
if errorlevel 1 exit /b 1
if defined use_managed if exist "%fly_dir%\.git\fly-managed" (
    "%python_bin%" -B "%project_dir%\tool\python\setup_fly.py" --check
    if errorlevel 1 exit /b 1
)
"%python_bin%" -B "%fly_dir%\tool\python\build.py" --project-dir "%project_dir%" %*
exit /b %errorlevel%

:missing_sdk
>&2 echo Could not locate FLY. Run build.bat setup to install the SDK from fly.lock, or set FLY_ROOT.
exit /b 1

:setup
call :find_python
if errorlevel 1 exit /b 1
"%python_bin%" -B "%project_dir%\tool\python\setup_fly.py" %2 %3 %4 %5 %6 %7 %8 %9
exit /b %errorlevel%

:find_python
set "python_bin="
if defined FLY_PYTHON call :try_python "%FLY_PYTHON%"
if not defined python_bin if defined fly_dir if exist "%fly_dir%\.venv\Scripts\python.exe" call :try_python "%fly_dir%\.venv\Scripts\python.exe"
for %%C in (python3 python) do (
    for /f "delims=" %%P in ('where %%C 2^>nul') do (
        if not defined python_bin call :try_python "%%P"
    )
)
if defined python_bin exit /b 0
>&2 echo Python 3.8 or later is required to run FLY.
exit /b 1

:try_python
"%~1" -c "import sys; sys.exit(sys.version_info < (3, 8))" >nul 2>nul
if not errorlevel 1 set "python_bin=%~1"
exit /b 0
