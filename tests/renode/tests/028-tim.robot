*** Settings ***
Resource        resources/stm32n6-common.robot

*** Variables ***
${TIM_BASE}     0x42000000

*** Keywords ***
Read TIM Register
    [Arguments]    ${offset}
    ${addr}=    Evaluate    ${TIM_BASE} + ${offset}
    ${val}=     Execute Command    sysbus ReadDoubleWord ${addr}
    ${val}=     Strip String    ${val}
    RETURN    ${val}

Write TIM Register
    [Arguments]    ${offset}    ${value}
    ${addr}=    Evaluate    ${TIM_BASE} + ${offset}
    Execute Command    sysbus WriteDoubleWord ${addr} ${value}

*** Test Cases ***
TIM1 CR1 Reset Value
    Start STM32N6
    # CR1 @ 0x00
    ${val}=    Read TIM Register    0x00
    Should Be Equal As Integers    ${val}    0

TIM1 CR1 Writable
    Start STM32N6
    # CR1 @ 0x00
    Write TIM Register    0x00    0x01
    ${val}=    Read TIM Register    0x00
    Should Be Equal As Integers    ${val}    0x01

TIM1 SR Accessible
    Start STM32N6
    # SR @ 0x10
    ${val}=    Read TIM Register    0x10
    Should Be True    int(${val}) >= 0

Boot Regression
    Start STM32N6
    Wait For NSH
