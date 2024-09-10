
set(FLY_SOURCE_DL_BASE_DIR "${FLY_TOP_DIR}/dl/")

file(GLOB PACKAGE_DIRECTORIES_LIST "${CMAKE_CURRENT_SOURCE_DIR}/*")

foreach(dir IN LISTS PACKAGE_DIRECTORIES_LIST)
    if(IS_DIRECTORY ${dir})
        add_subdirectory(${dir})
    endif()
endforeach()
