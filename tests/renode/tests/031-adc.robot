*** Settings ***
Resource        resources/stm32n6-common.robot

*** Variables ***
${ADC_BASE}     0x40026000

*** Keywords ***
Read ADC Register
    [Arguments]    ${offset}
    ${addr}=    Evaluate    ${ADC_BASE} + ${offset}
    ${val}=     Execute Command    sysbus ReadDoubleWord ${addr}
    ${val}=     Strip String    ${val}
    RETURN    ${val}

Write ADC Register
    [Arguments]    ${offset}    ${value}
    ${addr}=    Evaluate    ${ADC_BASE} + ${offset}
    Execute Command    sysbus WriteDoubleWord ${addr} ${value}

*** Test Cases ***
ADC1 ISR Reset Value
    Start STM32N6
    ${val}=    Read ADC Register    0x00
    Should Be Equal As Integers    ${val}    0

ADC1 CR Writable
    Start STM32N6
    Write ADC Register    0x08    0x01
    ${val}=    Read ADC Register    0x08
    Should Be Equal As Integers    ${val}    0x01

ADC1 DR Accessible
    Start STM32N6
    ${val}=    Read ADC Register    0x40
    Should Be True    int(${val}) >= 0

Boot Regression
    Start STM32N6
    Wait For NSH
