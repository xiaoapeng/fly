
include_guard(GLOBAL)

function(package_lib_add PACKAGE_NAME)
    list(APPEND FLY_ALL_PACKAGE_LIB_LIST ${PACKAGE_NAME})
    set(FLY_ALL_PACKAGE_LIB_LIST "${FLY_ALL_PACKAGE_LIB_LIST}" CACHE INTERNAL "All package lib")
endfunction()

function(package_lib_compile_options COMPILE_OPTIONS)
    foreach(lib_target IN LISTS FLY_ALL_PACKAGE_LIB_LIST)
        target_compile_options(${lib_target} PRIVATE ${COMPILE_OPTIONS})
    endforeach()
endfunction()


function(target_link_package_lib TARGET_NAME)
    target_link_libraries(${TARGET_NAME} ${FLY_ALL_PACKAGE_LIB_LIST})
endfunction()
