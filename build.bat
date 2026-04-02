@echo off
setlocal EnableExtensions EnableDelayedExpansion

set "script_dir=%~dp0"
for %%I in ("%script_dir%.") do set "source_dir=%%~fI"
set "build_py=%source_dir%\tool\python\build.py"
set "venv_dir=%source_dir%\.venv"
set "PYTHON_BIN="

call :find_python
if not defined PYTHON_BIN (
    echo [build.bat] Error: no usable Python 3 interpreter found ^(tried FLY_PYTHON/.venv/python/python3/python^).
    exit /b 1
)

call :require_module "%PYTHON_BIN%" kconfiglib
if errorlevel 1 (
    echo [build.bat] Missing dependency: kconfiglib ^(current Python: %PYTHON_BIN%^)

    if "%FLY_AUTO_SETUP_PY%"=="0" (
        echo [build.bat] Auto setup is disabled. Please run:
        echo   "%PYTHON_BIN%" -m pip install kconfiglib requests
        echo [build.bat] Or set FLY_AUTO_SETUP_PY=1 and retry.
        exit /b 1
    )

    call :ensure_venv_and_deps "%PYTHON_BIN%"

    if exist "%venv_dir%\Scripts\python.exe" (
        set "PYTHON_BIN=%venv_dir%\Scripts\python.exe"
    )

    call :require_module "%PYTHON_BIN%" kconfiglib
    if errorlevel 1 (
        echo [build.bat] Error: failed to install dependencies automatically. Please run:
        echo   "%PYTHON_BIN%" -m pip install kconfiglib requests
        exit /b 1
    )
)

"%PYTHON_BIN%" "%build_py%" "%source_dir%" %*
exit /b %ERRORLEVEL%

:find_python
if defined FLY_PYTHON (
    if exist "%FLY_PYTHON%" (
        set "PYTHON_BIN=%FLY_PYTHON%"
        exit /b 0
    )
)

if exist "%venv_dir%\Scripts\python.exe" (
    set "PYTHON_BIN=%venv_dir%\Scripts\python.exe"
    exit /b 0
)

for %%C in (python3 python) do (
    for /f "delims=" %%P in ('where %%C 2^>nul') do (
        for /f "delims=" %%V in ('"%%P" --version 2^>nul') do (
            echo %%V | findstr /b /c:"Python 3" >nul
            if not errorlevel 1 (
                set "PYTHON_BIN=%%P"
                exit /b 0
            )
        )
    )
)

exit /b 1

:require_module
set "CHECK_PY=%~1"
set "CHECK_MODULE=%~2"
"%CHECK_PY%" -c "import %CHECK_MODULE%" >nul 2>nul
exit /b %ERRORLEVEL%

:ensure_venv_and_deps
set "BOOTSTRAP_PY=%~1"
set "USE_VENV=1"

if not exist "%venv_dir%\Scripts\python.exe" (
    echo [build.bat] Creating virtual environment: %venv_dir%
    "%BOOTSTRAP_PY%" -m venv "%venv_dir%" >nul 2>nul
    if errorlevel 1 (
        set "USE_VENV=0"
        echo [build.bat] Warning: failed to create .venv, fallback to user-level pip install.
    )
)

echo [build.bat] Installing Python dependencies ^(kconfiglib, requests^)...

if "%USE_VENV%"=="1" (
    if exist "%venv_dir%\Scripts\python.exe" (
        set "WORK_PY=%venv_dir%\Scripts\python.exe"
        set "PIP_NO_CACHE_DIR=1"
        "%WORK_PY%" -m pip install --upgrade --disable-pip-version-check pip >nul
        "%WORK_PY%" -m pip install --disable-pip-version-check kconfiglib requests >nul
        exit /b 0
    )
)

set "PIP_NO_CACHE_DIR=1"
"%BOOTSTRAP_PY%" -m pip install --user --disable-pip-version-check kconfiglib requests >nul
exit /b 0
