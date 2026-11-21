@echo off
setlocal EnableExtensions DisableDelayedExpansion

for %%I in ("%~dp0.") do set "project_dir=%%~fI"
call :find_python
if errorlevel 1 exit /b 1
"%python_bin%" -B "%project_dir%\tool\python\setup_fly.py" --build %*
exit /b %errorlevel%

:find_python
set "python_bin="
if defined FLY_PYTHON call :try_python "%FLY_PYTHON%"
for %%C in (python3 python) do (
    for /f "delims=" %%P in ('where %%C 2^>nul') do (
        if not defined python_bin call :try_python "%%P"
    )
)
if defined python_bin exit /b 0

rem These interpreter hints only bootstrap Python. Python selects and checks the SDK.
if defined FLY_ROOT call :try_sdk_python "%FLY_ROOT%"
if not defined python_bin call :try_sdk_python "%project_dir%\fly"
set "stored_hint="
if not defined python_bin if exist "%project_dir%\.fly-root" set /p "stored_hint=" < "%project_dir%\.fly-root"
if not defined python_bin if defined stored_hint call :try_sdk_python "%stored_hint%"
if defined python_bin exit /b 0
>&2 echo Python 3.8 or later is required to run FLY.
exit /b 1

:try_sdk_python
if exist "%~1\.venv\Scripts\python.exe" call :try_python "%~1\.venv\Scripts\python.exe"
if not defined python_bin if exist "%project_dir%\%~1\.venv\Scripts\python.exe" call :try_python "%project_dir%\%~1\.venv\Scripts\python.exe"
exit /b 0

:try_python
"%~1" -c "import sys; sys.exit(sys.version_info < (3, 8))" >nul 2>nul
if not errorlevel 1 set "python_bin=%~1"
exit /b 0
