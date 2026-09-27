cmake_minimum_required(VERSION 3.10)

# Scheduler mutual exclusion is resolved by the Kconfig choice in this
# directory. Each OS sub-package self-gates with
# `if(CONFIG_PACKAGE_SCHEDULER_<leg>)` so add_subdirectory is unconditional
# here; when neither OS leg is selected the sub-packages' CMake guards
# short-circuit and no fetches or targets are registered.
add_subdirectory(eventhub-os)
add_subdirectory(freertos)