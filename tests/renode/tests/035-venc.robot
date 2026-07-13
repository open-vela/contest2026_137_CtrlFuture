*** Settings ***
Resource        resources/stm32n6-common.robot

*** Variables ***
${VENC_BASE}    0x50008000

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
    Start STM32N6
    ${val}=    Read VENC Register    0x00
    Should Be Equal As Integers    ${val}    0

VENC CFG Writable
    Start STM32N6
    Write VENC Register    0x0C    0x01
    ${val}=    Read VENC Register    0x0C
    Should Be Equal As Integers    ${val}    0x01

VENC SR Accessible
    Start STM32N6
    ${val}=    Read VENC Register    0x04
    Should Be True    int(${val}) >= 0

Boot Regression
    Start STM32N6
    Wait For NSH
