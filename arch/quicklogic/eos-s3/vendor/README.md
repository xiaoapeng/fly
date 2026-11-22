# QuickLogic EOS S3 vendor startup

The GCC reset and vector code is fetched into FLY's shared cache from QuickLogic
QORC SDK commit `d61d064146c0ee927aa12b088b3bbbce60615f4d`, under `HAL/startup/`.
The original ARM redistribution notices are in both source files.
The EOS S3 board support package distributes the unmodified application linker
script with the QORC SDK license.

- [QORC SDK startup](https://github.com/QuickLogic-Corp/qorc-sdk/tree/d61d064146c0ee927aa12b088b3bbbce60615f4d/HAL/startup)
- [QORC SDK application linker script](https://github.com/QuickLogic-Corp/qorc-sdk/blob/d61d064146c0ee927aa12b088b3bbbce60615f4d/qf_apps/qf_helloworldsw/GCC_Project/quickfeather.ld)
- [QORC SDK license](https://github.com/QuickLogic-Corp/qorc-sdk/blob/d61d064146c0ee927aa12b088b3bbbce60615f4d/LICENSE)
