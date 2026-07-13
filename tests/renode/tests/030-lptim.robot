*** Settings ***
Resource        resources/stm32n6-common.robot

*** Variables ***
${LPTIM_BASE}   0x40002400

*** Keywords ***
Read LPTIM Register
    [Arguments]    ${offset}
    ${addr}=    Evaluate    ${LPTIM_BASE} + ${offset}
    ${val}=     Execute Command    sysbus ReadDoubleWord ${addr}
    ${val}=     Strip String    ${val}
    RETURN    ${val}

Write LPTIM Register
    [Arguments]    ${offset}    ${value}
    ${addr}=    Evaluate    ${LPTIM_BASE} + ${offset}
    Execute Command    sysbus WriteDoubleWord ${addr} ${value}

*** Test Cases ***
LPTIM1 ISR Reset Value
    Start STM32N6
    ${val}=    Read LPTIM Register    0x00
    Should Be Equal As Integers    ${val}    0

LPTIM1 CR Writable
    Start STM32N6
    Write LPTIM Register    0x10    0x01
    ${val}=    Read LPTIM Register    0x10
    Should Be Equal As Integers    ${val}    0x01

LPTIM1 CNT Accessible
    Start STM32N6
    ${val}=    Read LPTIM Register    0x1C
    Should Be True    int(${val}) >= 0

Boot Regression
    Start STM32N6
    Wait For NSH
