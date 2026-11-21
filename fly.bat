@echo off
setlocal EnableExtensions DisableDelayedExpansion

for %%I in ("%~dp0.") do set "sdk_dir=%%~fI"
set "python_bin="
if defined FLY_PYTHON call :try_python "%FLY_PYTHON%"
if not defined python_bin if exist "%sdk_dir%\.venv\Scripts\python.exe" call :try_python "%sdk_dir%\.venv\Scripts\python.exe"
for %%C in (python3 python) do (
    for /f "delims=" %%P in ('where %%C 2^>nul') do (
        if not defined python_bin call :try_python "%%P"
    )
)
if not defined python_bin (
    >&2 echo Python 3.8 or later is required to generate a FLY project.
    exit /b 1
)
"%python_bin%" -B "%sdk_dir%\tool\python\project_new.py" %*
exit /b %errorlevel%

:try_python
"%~1" -c "import sys; sys.exit(sys.version_info < (3, 8))" >nul 2>nul
if not errorlevel 1 set "python_bin=%~1"
exit /b 0
