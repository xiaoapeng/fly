include_guard(GLOBAL)

# 检查.venv目录是否存在，如果存在则设置为Python根目录
if(EXISTS "${FLY_TOP_DIR}/.venv")
    message(STATUS "Found .venv directory, will prioritize Python from here")
    set(Python3_ROOT_DIR "${FLY_TOP_DIR}/.venv")
    if(EXISTS "${FLY_TOP_DIR}/.venv/Scripts/python.exe")
        set(Python3_EXECUTABLE "${FLY_TOP_DIR}/.venv/Scripts/python.exe" CACHE FILEPATH "Path to Python interpreter")
    elseif(EXISTS "${FLY_TOP_DIR}/.venv/bin/python")
        set(Python3_EXECUTABLE "${FLY_TOP_DIR}/.venv/bin/python" CACHE FILEPATH "Path to Python interpreter")
    endif()
endif()

find_package(Python3 COMPONENTS Interpreter REQUIRED)
message(STATUS "Python3 Interpreter: ${Python3_EXECUTABLE}")