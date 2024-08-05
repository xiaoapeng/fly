include_guard(GLOBAL)

include("${CMAKE_CURRENT_LIST_DIR}/tool_chain/${DEMAND_COMPILE_TOOL_CHAIN}.cmake")

if( DEMAND_CMAKE_SYSTEM_PROCESSOR )
    set(CMAKE_SYSTEM_PROCESSOR ${DEMAND_CMAKE_SYSTEM_PROCESSOR})
endif()

function(arch_build_config)
    add_compile_options( ${DEMAND_COMPILER_OPTIONS} )
    add_link_options( ${DEMAND_LINK_OPTIONS} )
    add_definitions( ${DEMAND_COMPILE_DEFINITIONS} )
endfunction()

