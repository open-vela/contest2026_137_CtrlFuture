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
TIM1 CR1 CEN Starts Counter
    [Tags]    L2-state
    Start STM32N6
    # CR1 @ 0x00, CEN = bit 0
    Write TIM Register    0x00    0x00000001
    ${val}=    Read TIM Register    0x00
    ${val}=    Convert To Integer    ${val}
    Should Be Equal As Integers    ${val}    0x00000001

TIM1 EGR Generates Update Event
    [Tags]    L2-state
    Start STM32N6
    # EGR @ 0x14, UG = bit 0
    Write TIM Register    0x14    0x01
    ${sr}=    Read TIM Register    0x10
    ${sr}=    Convert To Integer    ${sr}
    Should Be True    ${sr} & 0x01 == 1

TIM1 CNT Register Writable And Readable
    [Tags]    L2-state
    Start STM32N6
    # CNT @ 0x24
    Write TIM Register    0x24    0x1234
    ${val}=    Read TIM Register    0x24
    ${val}=    Convert To Integer    ${val}
    Should Be Equal As Integers    ${val}    0x1234

TIM1 PSC Prescaler Configurable
    [Tags]    L2-state
    Start STM32N6
    # PSC @ 0x28
    Write TIM Register    0x28    0x00FF
    ${val}=    Read TIM Register    0x28
    ${val}=    Convert To Integer    ${val}
    Should Be Equal As Integers    ${val}    0x00FF

TIM1 CCR1 Capture/Compare Configurable
    [Tags]    L2-state
    Start STM32N6
    # CCR1 @ 0x34
    Write TIM Register    0x34    0xABCD
    ${val}=    Read TIM Register    0x34
    ${val}=    Convert To Integer    ${val}
    Should Be Equal As Integers    ${val}    0xABCD

TIM1 BDTR Break Configuration
    [Tags]    L2-state
    Start STM32N6
    # BDTR @ 0x44
    Write TIM Register    0x44    0x0080
    ${val}=    Read TIM Register    0x44
    ${val}=    Convert To Integer    ${val}
    Should Be Equal As Integers    ${val}    0x0080

Boot Regression
    [Tags]    boot-regression
    Start STM32N6
    Wait For NSH
