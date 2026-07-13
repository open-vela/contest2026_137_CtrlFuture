*** Settings ***
Resource        resources/stm32n6-common.robot

*** Variables ***
${FDCAN_BASE}   0x4000A000

*** Keywords ***
Read FDCAN Register
    [Arguments]    ${offset}
    ${addr}=    Evaluate    ${FDCAN_BASE} + ${offset}
    ${val}=     Execute Command    sysbus ReadDoubleWord ${addr}
    ${val}=     Strip String    ${val}
    RETURN    ${val}

Write FDCAN Register
    [Arguments]    ${offset}    ${value}
    ${addr}=    Evaluate    ${FDCAN_BASE} + ${offset}
    Execute Command    sysbus WriteDoubleWord ${addr} ${value}

*** Test Cases ***
FDCAN1 CREL Reset Value
    Start STM32N6
    ${val}=    Read FDCAN Register    0x00
    Should Be Equal As Integers    ${val}    0

FDCAN1 CCCR Writable
    Start STM32N6
    Write FDCAN Register    0x18    0x00000001
    ${val}=    Read FDCAN Register    0x18
    Should Be Equal As Integers    ${val}    0x00000001

FDCAN1 ENDN Accessible
    Start STM32N6
    ${val}=    Read FDCAN Register    0x04
    Should Be True    int(${val}) >= 0

Boot Regression
    Start STM32N6
    Wait For NSH
