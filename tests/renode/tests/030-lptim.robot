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
LPTIM1 CR Enable Bit
    [Tags]    L2-state
    Start STM32N6
    # CR @ 0x10, enable bit 0
    Write LPTIM Register    0x10    0x01
    ${val}=    Read LPTIM Register    0x10
    ${val}=    Convert To Integer    ${val}
    Should Be Equal As Integers    ${val}    0x01

LPTIM1 ISR Write And Read
    [Tags]    L2-state
    Start STM32N6
    # ISR @ 0x00
    Write LPTIM Register    0x00    0x000000FF
    ${val}=    Read LPTIM Register    0x00
    ${val}=    Convert To Integer    ${val}
    Should Be Equal As Integers    ${val}    0x000000FF

LPTIM1 IER Write And Read
    [Tags]    L2-state
    Start STM32N6
    # IER @ 0x08, interrupt enable register
    Write LPTIM Register    0x08    0x00000003
    ${val}=    Read LPTIM Register    0x08
    ${val}=    Convert To Integer    ${val}
    Should Be Equal As Integers    ${val}    0x00000003

LPTIM1 CMP Compare Value
    [Tags]    L2-state
    Start STM32N6
    # CMP @ 0x14
    Write LPTIM Register    0x14    0x1234
    ${val}=    Read LPTIM Register    0x14
    ${val}=    Convert To Integer    ${val}
    Should Be Equal As Integers    ${val}    0x1234

Boot Regression
    [Tags]    boot-regression
    Start STM32N6
    Wait For NSH
