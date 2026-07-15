*** Settings ***
Resource        resources/stm32n6-common.robot

*** Variables ***
${ADC_BASE}     0x40022000

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
ADC1 CR ADSTART Bit
    [Tags]    L2-state
    Start STM32N6
    # CR @ 0x08, ADSTART = bit 0
    Write ADC Register    0x08    0x00000001
    ${val}=    Read ADC Register    0x08
    ${val}=    Convert To Integer    ${val}
    Should Be Equal As Integers    ${val}    0x00000001

ADC1 CFGR Configurable
    [Tags]    L2-state
    Start STM32N6
    # CFGR @ 0x0C
    Write ADC Register    0x0C    0x0000ABCD
    ${val}=    Read ADC Register    0x0C
    ${val}=    Convert To Integer    ${val}
    Should Be Equal As Integers    ${val}    0x0000ABCD

ADC1 SQR1 Sequence Register
    [Tags]    L2-state
    Start STM32N6
    # SQR1 @ 0x30
    Write ADC Register    0x30    0x00000010
    ${val}=    Read ADC Register    0x30
    ${val}=    Convert To Integer    ${val}
    Should Be Equal As Integers    ${val}    0x00000010

ADC1 DR Read-Only
    [Tags]    L2-state
    Start STM32N6
    # DR @ 0x40 is read-only in model; verify write is ignored
    Write ADC Register    0x40    0x00001234
    ${val}=    Read ADC Register    0x40
    ${val}=    Convert To Integer    ${val}
    # DR remains 0 since write is ignored (read-only)
    Should Be Equal As Integers    ${val}    0

Boot Regression
    [Tags]    boot-regression
    Start STM32N6
    Wait For NSH
