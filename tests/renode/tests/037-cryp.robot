*** Settings ***
Resource        resources/stm32n6-common.robot

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
    Start STM32N6
    ${val}=    Read CRYP Register    0x00
    Should Be Equal As Integers    ${val}    0

CRYP IVR0 Writable
    Start STM32N6
    Write CRYP Register    0x20    0xDEADBEEF
    ${val}=    Read CRYP Register    0x20
    Should Be Equal As Integers    ${val}    0xDEADBEEF

CRYP SR Accessible
    Start STM32N6
    ${val}=    Read CRYP Register    0x04
    Should Be True    int(${val}) >= 0

Boot Regression
    Start STM32N6
    Wait For NSH
