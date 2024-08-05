include_guard(GLOBAL)

function(load_config_file config_file)
    if (EXISTS "${config_file}")
        
        file(READ "${config_file}" CONFIG_CONTENT)
        
        string(REPLACE "\n" ";" CONFIG_LINES "${CONFIG_CONTENT}")
        
        foreach(line ${CONFIG_LINES})
            if (line MATCHES "([^=]+)=(.+)")
                set(VAR ${CMAKE_MATCH_1})
                set(VAL ${CMAKE_MATCH_2})
                # 移除前后的空格
                string(STRIP ${VAR} VAR)
                string(STRIP ${VAL} VAL)
                string(REPLACE "\"" "" VAL ${VAL})
                set(${VAR} ${VAL} PARENT_SCOPE)
                message(STATUS "Loaded config: ${VAR}=${VAL}")
            endif()
        endforeach()
    else()
        message(FATAL_ERROR "Configuration file ${config_file} not found.")
    endif()
endfunction()