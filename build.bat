@echo off
setlocal enabledelayedexpansion


set "script_dir=%~dp0"
set "script_path=%script_dir%%~nx0"

set "python_script_path=%script_dir%tool\python\build.py"

:: Clear any existing paths
set "allPythonPaths="

:: Check for virtual environment
set "venv_python=%script_dir%.venv\Scripts\python.exe"
if exist "%venv_python%" (
    set "allPythonPaths=!allPythonPaths! %venv_python%"
)

:: Collect all 'python' and 'python3' paths
for /f "delims=" %%a in ('where python') do (
    set "allPythonPaths=!allPythonPaths! %%a"
)

for /f "delims=" %%a in ('where python3') do (
    set "allPythonPaths=!allPythonPaths! %%a"
)

:: Variable to store filtered Python 3 paths
set "python3Paths="

:: Iterate over all collected paths to find Python 3 versions
for %%a in (%allPythonPaths%) do (
    set "pythonVersion="
    
    :: Get Python version
    for /f "delims=" %%b in ('%%a --version 2^>nul') do (
        set "pythonVersion=%%b"
    )
    
    :: Check if the version is Python 3.x.x
    echo !pythonVersion! | findstr /b "Python 3" >nul
    if not errorlevel 1 (
        set "python3Paths=%%a"
        goto :foundPython3
    )
)

:foundPython3

if not defined python3Paths (
    echo No Python 3 version found.
    exit /b 1
)
::echo !python3Paths!
call "%python3Paths%" %python_script_path% %script_dir% %*

endlocal