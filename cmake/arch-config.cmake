include_guard(GLOBAL)

include("${CMAKE_CURRENT_LIST_DIR}/tool_chain/${DEMAND_COMPILE_TOOL_CHAIN}.cmake")


function(arch_build_config)
    if( DEMAND_CMAKE_SYSTEM_PROCESSOR )
        set(CMAKE_SYSTEM_PROCESSOR ${DEMAND_CMAKE_SYSTEM_PROCESSOR} PARENT_SCOPE )
    endif()
    add_compile_options( ${DEMAND_COMPILER_OPTIONS} )
    add_link_options( ${DEMAND_LINK_OPTIONS} )
    add_definitions( ${DEMAND_COMPILE_DEFINITIONS} )
endfunction()

