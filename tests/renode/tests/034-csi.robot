*** Settings ***
Resource        resources/stm32n6-common.robot
Force Tags      csi

*** Variables ***
${CSI_BASE}     0x48006000

*** Keywords ***
Read CSI Register
    [Arguments]    ${offset}
    ${addr}=    Evaluate    ${CSI_BASE} + ${offset}
    ${val}=     Execute Command    sysbus ReadDoubleWord ${addr}
    ${val}=     Strip String    ${val}
    RETURN    ${val}

Write CSI Register
    [Arguments]    ${offset}    ${value}
    ${addr}=    Evaluate    ${CSI_BASE} + ${offset}
    Execute Command    sysbus WriteDoubleWord ${addr} ${value}

*** Test Cases ***
CSI CR Reset Value
    [Tags]    L1-register
    Start STM32N6
    ${val}=    Read CSI Register    0x00
    Should Be Equal As Integers    ${val}    0

CSI IER Writable
    [Tags]    L1-register
    Start STM32N6
    Write CSI Register    0x08    0x01
    ${val}=    Read CSI Register    0x08
    Should Be Equal As Integers    ${val}    0x01

CSI IFR Writable
    [Tags]    L1-register
    Start STM32N6
    # NOTE: this is a team-defined placeholder register (see
    # STM32N6_CSI.cs header); write/read round-trip instead of the
    # previous "int(val) >= 0" check on the read-only SR register,
    # which is true for any 32-bit unsigned read.
    Write CSI Register    0x0C    0x5A5A5A5A
    ${val}=    Read CSI Register    0x0C
    ${val}=    Convert To Integer    ${val}
    Should Be Equal As Integers    ${val}    0x5A5A5A5A

Boot Regression
    [Tags]    boot-regression
    Start STM32N6
    Wait For NSH
