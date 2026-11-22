add_library(arch-ra4m1ab OBJECT EXCLUDE_FROM_ALL startup_ra4m1.c)
target_compile_definitions(arch-ra4m1ab PUBLIC R7FA4M1AB)
