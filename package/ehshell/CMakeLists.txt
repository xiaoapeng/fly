cmake_minimum_required(VERSION 3.10)

if(CONFIG_PACKAGE_EHSHELL)

include("${FLY_TOP_DIR}/cmake/fetch_git_project.cmake")

message("Fetching ehshell...")

function(fnv1a_64_hash_cmake input_str output_var)
    find_package(Python3 COMPONENTS Interpreter REQUIRED)

    execute_process(
        COMMAND "${Python3_EXECUTABLE}" "-c"
        "import sys, functools; \
         h_init = 14695981039346656037; \
         p = 1099511628211; \
         s = sys.argv[1].encode('utf-8'); \
         res = functools.reduce(lambda h, c: ((h ^ c) * p) & 0xFFFFFFFFFFFFFFFF, s, h_init); \
         print(f'0x{res:016X}')" 
        "${input_str}"
        OUTPUT_VARIABLE raw_hex
        OUTPUT_STRIP_TRAILING_WHITESPACE
        RESULT_VARIABLE ret
    )

    if(NOT ret EQUAL 0)
        message(FATAL_ERROR "Python hash calculation failed!")
    endif()

    string(STRIP "${raw_hex}" clean_hex)
    
    set(${output_var} "${clean_hex}ULL" PARENT_SCOPE)
endfunction()

fetch_git_project(
    ehshell
    DEEP_CLONE     ON
    AUTO_UPDATE    OFF
    GIT_REPOSITORY https://github.com/xiaoapeng/ehshell.git
    GIT_TAG        master
)

fetch_git_project_add(ehshell)
package_lib_add(ehshell)
fnv1a_64_hash_cmake("${CONFIG_PACKAGE_EHSHELL_PASSWORD}" FINAL_HASH)
target_compile_definitions(ehshell PRIVATE -DCONFIG_PACKAGE_EHSHELL_PASSWORD_HASH=${FINAL_HASH})

if(NOT CONFIG_PACKAGE_EHSHELL_BUILTIN_NONE)
    package_lib_add(ehshell_builtin)
endif()

endif()
