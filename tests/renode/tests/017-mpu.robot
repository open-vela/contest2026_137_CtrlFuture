*** Settings ***
Resource        resources/stm32n6-common.robot
Force Tags      mpu

*** Variables ***
# ARMv8-M architectural MPU registers (Arm architecture reference
# manual, not chip-specific -- same addresses on every Cortex-M55).
${MPU_TYPE}     0xE000ED90
${MPU_CTRL}     0xE000ED94
${MPU_RNR}      0xE000ED98
${MPU_RBAR}     0xE000ED9C
${MPU_RLAR}     0xE000EDA0

*** Test Cases ***
MPU Type Register Reports Region Count
    [Tags]    L1-register
    Start STM32N6
    # MPU_TYPE.DREGION[15:8] reports the number of implemented
    # regions; on Cortex-M55 with MPU this is nonzero, unlike the
    # previous "int(val) >= 0" check which is true for any 32-bit
    # unsigned read and never actually verifies MPU presence.
    ${val}=    Execute Command    sysbus ReadDoubleWord ${MPU_TYPE}
    ${val}=    Strip String    ${val}
    ${val}=    Convert To Integer    ${val}
    ${dregion}=    Evaluate    (${val} >> 8) & 0xFF
    Should Be True    ${dregion} > 0

MPU Region Number Register Is Writable
    [Tags]    L2-state
    Start STM32N6
    # MPU_RNR selects which region RBAR/RLAR operate on; verify a
    # write is retained (functional check, not just "read succeeds").
    Execute Command    sysbus WriteDoubleWord ${MPU_RNR} 3
    ${val}=    Execute Command    sysbus ReadDoubleWord ${MPU_RNR}
    ${val}=    Strip String    ${val}
    ${val}=    Convert To Integer    ${val}
    Should Be Equal As Integers    ${val}    3

MPU Region Base And Limit Are Configurable
    [Tags]    L2-state
    Start STM32N6
    # Select region 0, then write RBAR/RLAR and verify the
    # configured base/limit round-trip through the region.
    Execute Command    sysbus WriteDoubleWord ${MPU_RNR} 0
    # RBAR: base address 0x34100000 (SRAM2), AP/XN bits left at 0
    Execute Command    sysbus WriteDoubleWord ${MPU_RBAR} 0x34100000
    # RLAR: limit address 0x341FFFE0 | EN bit0
    Execute Command    sysbus WriteDoubleWord ${MPU_RLAR} 0x341FFFE1
    ${rbar}=    Execute Command    sysbus ReadDoubleWord ${MPU_RBAR}
    ${rbar}=    Strip String    ${rbar}
    ${rbar}=    Convert To Integer    ${rbar}
    ${rlar}=    Execute Command    sysbus ReadDoubleWord ${MPU_RLAR}
    ${rlar}=    Strip String    ${rlar}
    ${rlar}=    Convert To Integer    ${rlar}
    Should Be Equal As Integers    ${rbar}    0x34100000
    Should Be Equal As Integers    ${rlar}    0x341FFFE1

MPU Control Register Enable Bit Is Writable
    [Tags]    L2-state
    Start STM32N6
    # MPU_CTRL.ENABLE is bit 0; verify it can be set and cleared
    # (the previous check only verified reads never raise an error,
    # which is true for any register regardless of MPU behavior).
    Execute Command    sysbus WriteDoubleWord ${MPU_CTRL} 0x1
    ${enabled}=    Execute Command    sysbus ReadDoubleWord ${MPU_CTRL}
    ${enabled}=    Strip String    ${enabled}
    ${enabled}=    Convert To Integer    ${enabled}
    Should Be True    (${enabled} & 0x1) == 0x1
    Execute Command    sysbus WriteDoubleWord ${MPU_CTRL} 0x0
    ${disabled}=    Execute Command    sysbus ReadDoubleWord ${MPU_CTRL}
    ${disabled}=    Strip String    ${disabled}
    ${disabled}=    Convert To Integer    ${disabled}
    Should Be True    (${disabled} & 0x1) == 0x0

Boot Regression
    [Tags]    boot-regression
    Start STM32N6
    Wait For NSH
