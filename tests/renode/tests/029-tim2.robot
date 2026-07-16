*** Settings ***
Resource        resources/stm32n6-common.robot
Force Tags      tim2

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
    [Tags]    L1-register
    Start STM32N6
    ${val}=    Read TIM2 Register    0x00
    Should Be Equal As Integers    ${val}    0

TIM2 CR1 Writable
    [Tags]    L1-register
    Start STM32N6
    Write TIM2 Register    0x00    0x0001
    ${val}=    Read TIM2 Register    0x00
    Should Be Equal As Integers    ${val}    0x0001

TIM2 ARR Writable
    [Tags]    L1-register
    Start STM32N6
    # ARR (Auto-Reload Register) @ 0x2C is a real, writable TIM2
    # register; write/read round-trip instead of the previous
    # "int(val) >= 0" check on SR, which is true for any 32-bit
    # unsigned read and never verifies the register actually works.
    Write TIM2 Register    0x2C    0x0000FFFF
    ${val}=    Read TIM2 Register    0x2C
    ${val}=    Convert To Integer    ${val}
    Should Be Equal As Integers    ${val}    0x0000FFFF

Boot Regression
    [Tags]    boot-regression
    Start STM32N6
    Wait For NSH
