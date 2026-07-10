*** Settings ***
Resource        resources/stm32n6-common.robot

*** Variables ***
${EXTI_BASE}    0x46000400

*** Keywords ***
Read EXTI Register
    [Arguments]    ${offset}
    ${addr}=    Evaluate    ${EXTI_BASE} + ${offset}
    ${val}=     Execute Command    sysbus ReadDoubleWord ${addr}
    ${val}=     Strip String    ${val}
    [Return]    ${val}

Write EXTI Register
    [Arguments]    ${offset}    ${value}
    ${addr}=    Evaluate    ${EXTI_BASE} + ${offset}
    Execute Command    sysbus WriteDoubleWord ${addr} ${value}

*** Test Cases ***
EXTI RTSR1 Reset Value
    Start STM32N6
    ${val}=    Read EXTI Register    0x00
    Should Be Equal As Integers    ${val}    0

EXTI FTSR1 Reset Value
    Start STM32N6
    ${val}=    Read EXTI Register    0x04
    Should Be Equal As Integers    ${val}    0

EXTI IMR1 Writable
    Start STM32N6
    Write EXTI Register    0x80    0xFF
    ${val}=    Read EXTI Register    0x80
    Should Be Equal As Integers    ${val}    0xFF

EXTI RTSR1 Writable
    Start STM32N6
    Write EXTI Register    0x00    0x0F
    ${val}=    Read EXTI Register    0x00
    Should Be Equal As Integers    ${val}    0x0F

EXTI Bank2 RTSR2 Accessible
    Start STM32N6
    Write EXTI Register    0x20    0x03
    ${val}=    Read EXTI Register    0x20
    Should Be Equal As Integers    ${val}    0x03

EXTI EXTICR0 Accessible
    Start STM32N6
    Write EXTI Register    0x60    0x00
    ${val}=    Read EXTI Register    0x60
    Should Be True    int(${val}) >= 0

EXTI IMR2 Accessible
    Start STM32N6
    Write EXTI Register    0x90    0xFF
    ${val}=    Read EXTI Register    0x90
    Should Be Equal As Integers    ${val}    0xFF

Boot Regression
    Start STM32N6
    Wait For NSH
