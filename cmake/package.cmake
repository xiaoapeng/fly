
include_guard(GLOBAL)

function(fly_register_package_library PACKAGE_NAME)
    list(APPEND FLY_ALL_PACKAGE_LIB_LIST ${PACKAGE_NAME})
    set(FLY_ALL_PACKAGE_LIB_LIST "${FLY_ALL_PACKAGE_LIB_LIST}" CACHE INTERNAL "All package lib")
endfunction()

function(fly_add_package_compile_options COMPILE_OPTIONS)
    foreach(lib_target IN LISTS FLY_ALL_PACKAGE_LIB_LIST)
        set(options ${COMPILE_OPTIONS})
        get_property(excluded TARGET ${lib_target} PROPERTY FLY_EXCLUDED_PACKAGE_COMPILE_OPTIONS)
        if(excluded)
            list(REMOVE_ITEM options ${excluded})
        endif()
        target_compile_options(${lib_target} PRIVATE ${options})
    endforeach()
endfunction()


function(fly_link_package_libraries TARGET_NAME)
    target_link_libraries(${TARGET_NAME} ${FLY_ALL_PACKAGE_LIB_LIST})
endfunction()
