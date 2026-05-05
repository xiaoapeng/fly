cmake_minimum_required(VERSION 3.10)

project(co_ctx_test LANGUAGES C CXX ASM)

include("${FLY_TOP_DIR}/cmake/package.cmake")

set(TARGET_FLAGS
    "-Wall"
    "-Wextra"
    "-Wconversion"
    "-Wsign-conversion"
    "-mpreferred-stack-boundary=4"
    "$<$<CONFIG:Debug>:-g;-O0>"
    "$<$<CONFIG:Release>:-g;-O3>"
    "$<$<CONFIG:MinSizeRel>:-g;-Os>"
    "$<$<CONFIG:RelWithDebInfo>:-g;-Os>"
)

target_compile_definitions(eventhub PUBLIC
    "-DEH_CONFIG_DEFAULT_DEBUG_LEVEL=EH_DBG_DEBUG"
    "-DEH_CONFIG_DEBUG_ENTER_SIGN=\"\\r\\n\""
)

package_lib_compile_options("${TARGET_FLAGS}")

add_executable(co_ctx_test )
target_compile_options(co_ctx_test PRIVATE "${TARGET_FLAGS}")
target_link_package_lib(co_ctx_test)

target_sources(co_ctx_test PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src/main.c"
)

add_target_make_map(co_ctx_test)
