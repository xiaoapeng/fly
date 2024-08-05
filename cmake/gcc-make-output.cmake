
function(add_target_bin ELF_FILE)
    add_custom_command(
        TARGET ${ELF_FILE}
        POST_BUILD
        COMMAND ${CMAKE_SIZE} --format=berkeley $<TARGET_FILE:${ELF_FILE}>
        COMMAND ${CMAKE_OBJDUMP} --all-headers --demangle --disassemble $<TARGET_FILE:${ELF_FILE}> > ${ELF_FILE}.list
        COMMAND ${CMAKE_OBJDUMP} -d -S -s $<TARGET_FILE:${ELF_FILE}> > ${ELF_FILE}.S
        COMMAND ${CMAKE_OBJCOPY} -O ihex $<TARGET_FILE:${ELF_FILE}> ${ELF_FILE}.hex
        #删除 bin文件中的调试信息
        COMMAND ${CMAKE_OBJCOPY} --strip-debug --strip-unneeded -O binary $<TARGET_FILE:${ELF_FILE}> ${ELF_FILE}.bin
        COMMENT "Building ${ELF_FILE}\nBuilding ${ELF_FILE}.bin"
    )
endfunction()
