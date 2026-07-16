*** Settings ***
Resource        resources/stm32n6-common.robot
Force Tags      cryp

*** Variables ***
${CRYP_BASE}    0x42006000

*** Keywords ***
Read CRYP Register
    [Arguments]    ${offset}
    ${addr}=    Evaluate    ${CRYP_BASE} + ${offset}
    ${val}=     Execute Command    sysbus ReadDoubleWord ${addr}
    ${val}=     Strip String    ${val}
    RETURN    ${val}

Write CRYP Register
    [Arguments]    ${offset}    ${value}
    ${addr}=    Evaluate    ${CRYP_BASE} + ${offset}
    Execute Command    sysbus WriteDoubleWord ${addr} ${value}

*** Test Cases ***
CRYP CR Reset Value
    [Tags]    L1-register
    Start STM32N6
    ${val}=    Read CRYP Register    0x00
    Should Be Equal As Integers    ${val}    0

CRYP IVR0 Writable
    [Tags]    L1-register
    Start STM32N6
    Write CRYP Register    0x20    0xDEADBEEF
    ${val}=    Read CRYP Register    0x20
    Should Be Equal As Integers    ${val}    0xDEADBEEF

CRYP IVR1 Writable
    [Tags]    L1-register
    Start STM32N6
    # NOTE: this is a team-defined placeholder register (CMSIS does
    # not publish CRYP_TypeDef/SAES_TypeDef, see STM32N6_CRYP.cs
    # header); write/read round-trip instead of the previous
    # "int(val) >= 0" check on the read-only SR register, which is
    # true for any 32-bit unsigned read.
    Write CRYP Register    0x24    0x0F0F0F0F
    ${val}=    Read CRYP Register    0x24
    ${val}=    Convert To Integer    ${val}
    Should Be Equal As Integers    ${val}    0x0F0F0F0F

Boot Regression
    [Tags]    boot-regression
    Start STM32N6
    Wait For NSH
