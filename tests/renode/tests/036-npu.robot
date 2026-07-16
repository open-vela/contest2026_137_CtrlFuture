*** Settings ***
Resource        resources/stm32n6-common.robot
Force Tags      npu

*** Variables ***
${NPU_BASE}     0x480E0000

*** Keywords ***
Read NPU Register
    [Arguments]    ${offset}
    ${addr}=    Evaluate    ${NPU_BASE} + ${offset}
    ${val}=     Execute Command    sysbus ReadDoubleWord ${addr}
    ${val}=     Strip String    ${val}
    RETURN    ${val}

Write NPU Register
    [Arguments]    ${offset}    ${value}
    ${addr}=    Evaluate    ${NPU_BASE} + ${offset}
    Execute Command    sysbus WriteDoubleWord ${addr} ${value}

*** Test Cases ***
NPU CR Reset Value
    [Tags]    L1-register
    Start STM32N6
    ${val}=    Read NPU Register    0x00
    Should Be Equal As Integers    ${val}    0

NPU CFG Writable
    [Tags]    L1-register
    Start STM32N6
    Write NPU Register    0x10    0x01
    ${val}=    Read NPU Register    0x10
    Should Be Equal As Integers    ${val}    0x01

NPU IER Writable
    [Tags]    L1-register
    Start STM32N6
    # NOTE: this is a team-defined placeholder register (CMSIS only
    # publishes NPU_BASE, not a real NPU_TypeDef, see
    # STM32N6_NPU.cs header); write/read round-trip instead of the
    # previous "int(val) >= 0" check on the read-only SR register,
    # which is true for any 32-bit unsigned read.
    Write NPU Register    0x08    0x77007700
    ${val}=    Read NPU Register    0x08
    ${val}=    Convert To Integer    ${val}
    Should Be Equal As Integers    ${val}    0x77007700

Boot Regression
    [Tags]    boot-regression
    Start STM32N6
    Wait For NSH
