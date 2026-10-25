cmake_minimum_required(VERSION 3.10)

if(CONFIG_PACKAGE_NSBTN)

add_library(nsbtn src/nsbtn.c)

target_include_directories(nsbtn PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

# PUBLIC: consumers include <nsbtn/nsbtn.h>, which exposes lwbtn types and
# nanosig signals, so both include dirs and LWBTN_CFG_* definitions must
# propagate to every consumer translation unit.
target_link_libraries(nsbtn PUBLIC lwbtn nanosig)

package_lib_add(nsbtn)

endif()
