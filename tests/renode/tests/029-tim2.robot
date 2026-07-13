*** Settings ***
Resource        resources/stm32n6-common.robot

*** Variables ***
${TIM2_BASE}    0x40000000

*** Keywords ***
Read TIM2 Register
    [Arguments]    ${offset}
    ${addr}=    Evaluate    ${TIM2_BASE} + ${offset}
    ${val}=     Execute Command    sysbus ReadDoubleWord ${addr}
    ${val}=     Strip String    ${val}
    RETURN    ${val}

Write TIM2 Register
    [Arguments]    ${offset}    ${value}
    ${addr}=    Evaluate    ${TIM2_BASE} + ${offset}
    Execute Command    sysbus WriteDoubleWord ${addr} ${value}

*** Test Cases ***
TIM2 CR1 Reset Value
    Start STM32N6
    ${val}=    Read TIM2 Register    0x00
    Should Be Equal As Integers    ${val}    0

TIM2 CR1 Writable
    Start STM32N6
    Write TIM2 Register    0x00    0x0001
    ${val}=    Read TIM2 Register    0x00
    Should Be Equal As Integers    ${val}    0x0001

TIM2 SR Accessible
    Start STM32N6
    ${val}=    Read TIM2 Register    0x10
    Should Be True    int(${val}) >= 0

Boot Regression
    Start STM32N6
    Wait For NSH
