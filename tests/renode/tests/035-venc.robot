*** Settings ***
Resource        resources/stm32n6-common.robot
Force Tags      venc

*** Variables ***
${VENC_BASE}    0x48005000

*** Keywords ***
Read VENC Register
    [Arguments]    ${offset}
    ${addr}=    Evaluate    ${VENC_BASE} + ${offset}
    ${val}=     Execute Command    sysbus ReadDoubleWord ${addr}
    ${val}=     Strip String    ${val}
    RETURN    ${val}

Write VENC Register
    [Arguments]    ${offset}    ${value}
    ${addr}=    Evaluate    ${VENC_BASE} + ${offset}
    Execute Command    sysbus WriteDoubleWord ${addr} ${value}

*** Test Cases ***
VENC CR Reset Value
    [Tags]    L1-register
    Start STM32N6
    ${val}=    Read VENC Register    0x00
    Should Be Equal As Integers    ${val}    0

VENC CFG Writable
    [Tags]    L1-register
    Start STM32N6
    Write VENC Register    0x0C    0x01
    ${val}=    Read VENC Register    0x0C
    Should Be Equal As Integers    ${val}    0x01

VENC IER Writable
    [Tags]    L1-register
    Start STM32N6
    # NOTE: this is a team-defined placeholder register (CMSIS does
    # not publish a real VENC_TypeDef, see STM32N6_VENC.cs header);
    # write/read round-trip instead of the previous "int(val) >= 0"
    # check on the read-only SR register, which is true for any
    # 32-bit unsigned read.
    Write VENC Register    0x08    0x33333333
    ${val}=    Read VENC Register    0x08
    ${val}=    Convert To Integer    ${val}
    Should Be Equal As Integers    ${val}    0x33333333

Boot Regression
    [Tags]    boot-regression
    Start STM32N6
    Wait For NSH
