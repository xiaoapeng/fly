# Renode 1.17.0 板级平台接入清单

本表覆盖 Renode 1.17.0 提交 `ab721d88e135a1bcb8ed2ecc5a38f51cbe61fdd2` 在 `platforms/boards/` 下递归查到的 **80 份 `.repl`**。文件名相对于该目录，芯片与内核栏首先依据 REPL 的 `using`、CPU 声明和部件配置。只确认了仿真模型时，用「模型」标明，板卡尚需核对实装型号、板载晶振和内存。 [版本发布](https://github.com/renode/renode/releases/tag/v1.17.0) · [固定的板级源码](https://github.com/renode/renode/tree/ab721d88e135a1bcb8ed2ecc5a38f51cbe61fdd2/platforms/boards) · [固定的芯片源码](https://github.com/renode/renode/tree/ab721d88e135a1bcb8ed2ecc5a38f51cbe61fdd2/platforms/cpus)

「可复用」指 FLY 已有对应的编译目标。扩展片段随基板绑定。同一芯片的板卡共用一份芯片支持，每个物理板卡包分别声明实际器件、引脚和内存。接入状态以新生成的 FLY 工程编译、打包及在 Renode 运行同一份 BIN 的结果为准。

## 复用已有芯片目标的板卡

| 板卡 | 实装芯片 | FLY 构建目标 | 板卡资料 |
|---|---|---|---|
| STM32F072B Discovery | STM32F072RB | `arch-stm32f072xb` | [ST 产品页](https://www.st.com/en/evaluation-tools/32f072bdiscovery.html) · [厂商 BSP](https://github.com/STMicroelectronics/32f072bdiscovery-bsp) |
| STM32F4 Discovery | STM32F407VG | `arch-stm32f407xx` | [ST 产品页](https://www.st.com/en/evaluation-tools/stm32f4discovery.html) · [STM32CubeF4](https://github.com/STMicroelectronics/STM32CubeF4) |
| NUCLEO-H753ZI | STM32H753ZI | `arch-stm32h753xx`，CM7 | [ST 产品页](https://www.st.com/en/evaluation-tools/nucleo-h753zi.html) · [STM32CubeH7 板卡示例](https://github.com/STMicroelectronics/STM32CubeH7/tree/master/Projects/NUCLEO-H753ZI) |
| STM32H747I-DISCO | STM32H747XI | `arch-stm32h747xx-cm7` 或 `arch-stm32h747xx-cm4`，按启动核绑定 | [ST 产品页](https://www.st.com/en/evaluation-tools/stm32h747i-disco.html) · [厂商 BSP](https://github.com/STMicroelectronics/stm32h747i-disco-bsp) |

上表四块板均通过了 Renode 裸机启动、固件 CRC 和 RTT 验证。H747 的验证覆盖 Cortex-M7。全表按上游目录和文件名排列，包含 12 个依赖基板的扩展片段。标为「系统」的条目还需要对应的 SoC 工具链、引导和镜像流程。「扩展」和「组合」沿用基板及芯片目标。

## 全部板级描述

| # | 上游文件 | 类型 | 芯片或模型证据 | FLY 接入项 |
|---:|---|---|---|---|
| 1 | `A2_CV32E40P.repl` | MCU 参考平台 | 仅引入 CORE-V MCU，CV32E40P | 新增 CORE-V MCU 启动与 RISC-V 加载，核对 DevKit 板级差异 |
| 2 | `arduino_101-shield.repl` | 板卡扩展组合 | Quark C1000 模型及 Arduino 101 shield | 新增 Quark 目标与启动流程，同第 35 项 |
| 3 | `arduino_nano_33_ble.repl` | MCU 板卡 | nRF52840 | 已实跑 `arduino-nano-33-ble`，与第 28 项共用 `arch-nrf52840`，直接启动 `0x00010000` 的 APP |
| 4 | `arduino_uno_r4_minima.repl` | MCU 板卡 | Renesas RA4M1，模型 R7FA4M1A | 已实跑 `arduino-uno-r4-minima`，保留前 16 KiB 引导区并直接启动 APP |
| 5 | `arty_litex_vexriscv.repl` | FPGA 平台 | Arty 上的 LiteX / VexRiscv | 固定 LiteX 配置和软核启动，核对位流对应的内存 |
| 6 | `arvsom.repl` | 系统 | 引入 StarFive JH7100 模型 | 新增 JH7100 SoC 与系统启动流程，同第 8 项 |
| 7 | `beaglev-fire.repl` | 系统 | Microchip PolarFire SoC 模型 | 新增 PolarFire SoC 启动与镜像流程，同第 27 项 |
| 8 | `beaglev_starlight.repl` | 系统 | 模型引入 StarFive JH7100 | 复核具体板卡版次，与第 6 项共用 SoC 支持 |
| 9 | `colibri-vf61.repl` | 系统 | Toradex Colibri VF61，Vybrid 模型 | 新增 Vybrid 及所选核的启动适配 |
| 10 | `cortex_a53_console.repl` | 仿真测试 | 直接声明 Cortex-A53 和控制台 | 可按虚拟平台接入 |
| 11 | `cortex_a53_virtio.repl` | 仿真测试 | 直接声明 Cortex-A53 和 VirtIO 设备 | 可作虚拟平台，需 AArch64 系统和 VirtIO 启动流程 |
| 12 | `crosslink-nx-evn.repl` | FPGA 平台 | CrossLink-NX 上的 LiteX / VexRiscv | 新增软核启动，按该设计固定内存和外设 |
| 13 | `eos-s3-qomu.repl` | MCU 板卡 | QuickLogic EOS S3，Cortex-M4F | 已实跑 `qomu`，使用 QORC SDK 的 SRAM APP 布局 |
| 14 | `eos-s3-quickfeather.repl` | MCU 板卡 | QuickLogic EOS S3，Cortex-M4F | 已实跑 `quickfeather`，与第 13 项复用芯片支持和 SRAM APP 布局 |
| 15 | `fsl_lx2160ardb.repl` | 系统 | 文件声明 Cortex-A53，实板 LX2160A 为 Cortex-A72 | 先核对 CPU 建模差异，再考虑 Layerscape 启动兼容 |
| 16 | `gr716-devboard.repl` | MCU 板卡 | GR716，LEON3FT / SPARC | 新增 SPARC 工具链和 GR716 启动 |
| 17 | `ice40up5k-mdp-evn.repl` | FPGA 平台 | iCE40UP5K，复用 Fomu / VexRiscv 模型 | 先固定实际 LiteX 设计和软核内存布局 |
| 18 | `leon3-externals.repl` | 扩展 | LEON3 平台的外部设备 | 随第 19 项使用 |
| 19 | `leon3.repl` | 处理器参考平台 | LEON3 CPU 加外部设备 | 新增 SPARC 支持，登记为模拟参考平台 |
| 20 | `mars_zx3-externals.repl` | 扩展 | Mars ZX3 外部板级设备 | 随第 21 项使用 |
| 21 | `mars_zx3.repl` | 系统 | Zynq-7000 模型及 Mars 扩展 | 新增 Zynq-7000 启动流程，同第 76 项 |
| 22 | `max32652-evkit.repl` | MCU 板卡 | MAX32652 | 已实跑 `max32652-evkit`，采用 ADI MSDK 的 MAX32650 系列启动代码 |
| 23 | `mimxrt1064_evk.repl` | MCU 板卡 | NXP i.MX RT1064 | 新增 RT1064 启动、XIP 配置与 MCUXpresso SDK 适配 |
| 24 | `mimxrt700_evk.repl` | MCU 板卡 | 模型引入 MIMXRT798S | 新增 RT700，核对应用核、内存与启动方式 |
| 25 | `miv-board-additional-uarts.repl` | 扩展 | 第 26 项加两路 CoreUART | 随第 26 项使用 |
| 26 | `miv-board.repl` | FPGA 平台 | Mi-V，模型 RISC-V RV32IMAF | 新增 Mi-V 软核启动及模型对应的板级布局 |
| 27 | `mpfs-icicle-kit.repl` | 系统 | Microchip PolarFire SoC 模型 | 复用第 7 项 SoC 支持，核对 Icicle Kit 启动和内存 |
| 28 | `nrf52840dk_nrf52840.repl` | MCU 板卡 | Nordic nRF52840 DK，nRF52840 | 已实跑 `nrf52840dk`，与第 3 项共用 Nordic nrfx 和 CMSIS 启动支持 |
| 29 | `nucleo_f767zi.repl` | MCU 板卡 | 实板 STM32F767ZI，REPL 引入 STM32F777 模型 | 已实跑 `nucleo-f767zi`，使用 ST CMSIS Device F7 的 F767 启动代码 |
| 30 | `nucleo_h753zi.repl` | MCU 板卡 | STM32H753ZI，REPL 引入 STM32H753 | 已实跑 `nucleo-h753zi`，复用 `arch-stm32h753xx` |
| 31 | `nucleo_wba52cg.repl` | MCU 板卡 | STM32WBA52CG | 新增 STM32WBA52 与所选安全态启动 |
| 32 | `nuvoton_npcx9m6fb_evb.repl` | MCU 板卡 | NPCX9M6FB，REPL 引入 NPCX9 模型 | 已实跑 `npcx9m6fb-evb`，直接加载 Code SRAM，SPI 引导待接入 |
| 33 | `nxp-frdm-imx8mplus.repl` | 系统 | i.MX 8M Plus，Cortex-A53 与 Cortex-M7 | 先确定运行核，整板还需要系统引导与镜像 |
| 34 | `nxp-s32k388evb.repl` | MCU 板卡 | S32K388 模型 | 新增 S32K388，核对启用内核、启动协议及可用 SDK |
| 35 | `quark_c1000-cc2520.repl` | 板卡扩展组合 | Quark C1000 加 CC2520 | 复用第 2 项 Quark 目标，CC2520 能力单独验证 |
| 36 | `ramn/BODY.repl` | 扩展 | RAMN BODY ECU 外设 | 随第 40 项使用，保留 ECU 角色 |
| 37 | `ramn/CHASSIS.repl` | 扩展 | RAMN CHASSIS ECU 外设 | 随第 40 项使用，保留 ECU 角色 |
| 38 | `ramn/GATEWAY.repl` | 扩展 | RAMN GATEWAY ECU 外设 | 随第 40 项使用，保留 ECU 角色 |
| 39 | `ramn/POWERTRAIN.repl` | 扩展 | RAMN POWERTRAIN ECU 外设 | 随第 40 项使用，保留 ECU 角色 |
| 40 | `ramn.repl` | MCU 板卡 | RAMN ECU，STM32L552 模型 | 新增 STM32L552 芯片支持，按 ECU 角色配置模型 |
| 41 | `renesas-ck_ra6m5.repl` | MCU 板卡 | RA6M5，模型 R7FA6M5B | 新增 RA6M5 目标，可共用 Renesas RA 的 SDK 接入 |
| 42 | `renesas-ek_ra2e1.repl` | MCU 板卡 | R7FA2E1A92DFM | 已实跑 `renesas-ek-ra2e1`，使用来自 Renesas FSP 的 Cortex-M23 启动代码 |
| 43 | `renesas-ek_ra8m1.repl` | MCU 板卡 | RA8M1，模型 R7FA8M1A | 新增 RA8M1 和 Cortex-M85 工具链配置 |
| 44 | `renesas-rz_t2m_rsk.repl` | 处理器板卡 | Renesas RZ/T2M，模型 Cortex-R52 | 新增 Cortex-R52 配置及 RZ/T2M 启动 |
| 45 | `sam4s_xplained.repl` | MCU 板卡 | SAM4S16C 模型 | 新增 SAM4S 启动及设备包 |
| 46 | `silabs/brd4116a.repl` | MCU 无线板 | EFR32MG26B410F3200IM48 | 已实跑 `silabs-brd4116a`，第 47–50 项共用固定版本的 Simplicity SDK |
| 47 | `silabs/brd4117a.repl` | MCU 无线板 | EFR32MG26B420F3200IM48 | 已实跑 `silabs-brd4117a` |
| 48 | `silabs/brd4118a.repl` | MCU 无线板 | EFR32MG26B510F3200IL136 | 已实跑 `silabs-brd4118a` |
| 49 | `silabs/brd4120a.repl` | MCU 无线板 | EFR32MG26B410F3200IM68 | 已实跑 `silabs-brd4120a` |
| 50 | `silabs/brd4121a.repl` | MCU 无线板 | EFR32MG26B420F3200IM68 | 已实跑 `silabs-brd4121a` |
| 51 | `silabs/brd4162a.repl` | MCU 无线板 | EFR32MG12 模型 | 新增 EFR32MG12 目标，与第 60 项复用 |
| 52 | `silabs/brd4186c.repl` | MCU 无线板 | EFR32MG24 模型 | 新增 EFR32MG24 目标 |
| 53 | `silabs/brd4402a.repl` | MCU 无线板 | EFR32xG22 通用模型 | 核对具体 EFR32 SKU，再补对应目标 |
| 54 | `silabs/sltb001a.repl` | MCU 板卡 | EFR32MG1 模型 | 新增 EFR32MG1 目标 |
| 55 | `silabs/slwstk6220a.repl` | MCU 无线套件 | EZR32WG330 模型 | 新增 EZR32WG330，核对无线板版本 |
| 56 | `silabs/stk3200.repl` | MCU 板卡 | EFM32ZG222 模型 | 新增 EFM32ZG222，核对设备包和板级时钟 |
| 57 | `silabs/stk3600.repl` | MCU 板卡 | EFM32LG995 模型 | 新增 EFM32LG995，核对设备包和板级时钟 |
| 58 | `silabs/stk3700.repl` | MCU 板卡 | EFM32GG995 模型 | 新增 EFM32GG995，核对设备包和板级时钟 |
| 59 | `silabs/stk3800.repl` | MCU 板卡 | EFM32WG995 模型 | 新增 EFM32WG995，核对设备包和板级时钟 |
| 60 | `sltb004a.repl` | MCU 板卡 | EFR32MG12 模型 | 复用第 51 项芯片支持，核对传感器及按键映射 |
| 61 | `stm32f072b_discovery.repl` | MCU 板卡 | STM32F072RB，REPL 引入 STM32F072 | 已实跑 `stm32f072b-disco`，复用 `arch-stm32f072xb` |
| 62 | `stm32f4_discovery-additional_gpios.repl` | 扩展 | PA3 按键和 PA5 LED | 随第 65 项使用 |
| 63 | `stm32f4_discovery-bb.repl` | 扩展 | 以太网 PHY 模型 | 随第 65 项使用 |
| 64 | `stm32f4_discovery-kit.repl` | 组合 | 第 65 项基板加第 62、63 项 | 共用 STM32F407VG 芯片绑定 |
| 65 | `stm32f4_discovery.repl` | MCU 板卡 | STM32F407VG，REPL 引入 STM32F4 | 已实跑 `stm32f407g-disc1`，复用 `arch-stm32f407xx` |
| 66 | `stm32f7_discovery-bb.repl` | MCU 板卡 | 文件自带 CPU 引用，STM32F746 模型 | 已实跑 `stm32f746g-disco`，绑定 `arch-stm32f746xx`，扩展板外设待验证 |
| 67 | `stm32h747-disco.repl` | MCU 板卡 | STM32H747XI，REPL 引入 STM32H747 | 已实跑 `stm32h747i-disco-cm7`，复用 Cortex-M7 启动，Cortex-M4 待接入 |
| 68 | `tegra2.repl` | 系统 | Tegra 2，Cortex-A9 模型 | 新增 Tegra 2 系统启动，核对具体载板 |
| 69 | `tegra3.repl` | 系统 | Tegra 3，Cortex-A9 模型 | 新增 Tegra 3 系统启动，核对具体载板 |
| 70 | `tegra_externals.repl` | 扩展 | Tegra 公用外部设备 | 随第 68、69 项使用 |
| 71 | `vegaboard_ri5cy.repl` | MCU 板卡 | VEGAboard 的 RI5CY 核模型 | 新增对应 RV32 启动，确认实装 RV32M1 与选核 |
| 72 | `versatile.repl` | 处理器参考平台 | Arm Versatile 模型 | 新增板型对应的 Arm 启动，核对具体硬件配置 |
| 73 | `vexpress-externals.repl` | 扩展 | Versatile Express 的外部设备 | 随第 74 项使用 |
| 74 | `vexpress.repl` | 处理器参考平台 | Arm Versatile Express 模型 | 新增具体处理器模组的启动流程 |
| 75 | `zedboard-externals.repl` | 扩展 | ZedBoard 的外部设备 | 随第 76 项使用 |
| 76 | `zedboard.repl` | 系统 | Zynq-7000 模型及 ZedBoard 扩展 | 复用第 21 项 SoC 支持，核对实际器件及引导 |
| 77 | `zolertia-firefly.repl` | MCU 板卡 | TI CC2538SF53 | 已实跑 `zolertia-firefly`，保留末尾 CCA 页，Renode 直接启动 APP |
| 78 | `zynqmp-zcu102-revA.repl` | 系统 | Zynq UltraScale+ MPSoC 模型 | 新增 ZynqMP SoC 启动流程，明确 A53 或 R5 目标 |
| 79 | `zynqmp-zcu102-revB.repl` | 系统板卡版本 | 引用 RevA 并覆盖板级设备 | 复用第 78 项 SoC 支持，按 RevB 核对外设 |
| 80 | `zynqmp-zcu104.repl` | 系统 | Zynq UltraScale+ MPSoC 模型 | 复用第 78 项 SoC 支持，按 ZCU104 定义启动配置 |

## 补芯片支持的上游来源

- CORE-V 的 ASIC 及 DevKit 数据来自 OpenHW。Arduino UNO R4 Minima 可用 Arduino 资料核对 RA4M1，Renesas RA 全系列的启动与驱动候选来自 FSP。 [CORE-V DevKit](https://docs.openhwgroup.org/projects/core-v-mcu-devkit-user-manual/en/latest/doc-src/hardware-description.html) · [Arduino UNO R4 Minima](https://docs.arduino.cc/hardware/uno-r4-minima/) · [Renesas FSP](https://github.com/renesas/fsp)
- nRF52840、EFR32MG26、STM32F7、MAX32652、CC2538、EOS S3 与 RA2E1 的启动源码版本记录在各架构目录。i.MX RT 的镜像和启动方式待接入。 [Nordic nrfx](https://github.com/NordicSemiconductor/nrfx) · [Silicon Labs Simplicity SDK](https://github.com/SiliconLabs/simplicity_sdk/tree/b41bec3ff2485199c1a5a9995b3e649e118c1b8d) · [NXP MCUXpresso SDK](https://github.com/nxp-mcuxpresso/mcuxsdk-manifests)
- LX2160A-RDB 是公开资料和模型声明需要对照核实的例子。NXP 标明实板使用 Cortex-A72，固定版 REPL 中 CPU 类型写为 Cortex-A53。 [NXP 板卡规格](https://www.nxp.com/products/processors-and-microcontrollers/arm-processors/layerscape-processors/layerscape-lx2160a-reference-design-board%3ALX2160A-RDB) · [LX2160ARDB REPL](https://github.com/renode/renode/blob/ab721d88e135a1bcb8ed2ecc5a38f51cbe61fdd2/platforms/boards/fsl_lx2160ardb.repl)

## 复核清单

在 Renode 仓库根目录运行以下命令，从固定提交列出板级描述。

```sh
git ls-tree -r --name-only ab721d88e135a1bcb8ed2ecc5a38f51cbe61fdd2 -- platforms/boards/ |
    sed -n 's|^platforms/boards/\(.*\.repl\)$|\1|p'
```

复核时逐项保留上游相对路径。实测记录应保存板卡与模板 ID、Renode 版本、固件包 ID、启动与 CRC 日志。外设验证范围随模型实际能力记录。 [Renode 平台描述文档](https://renode.readthedocs.io/en/latest/advanced/platform_description_format.html)
