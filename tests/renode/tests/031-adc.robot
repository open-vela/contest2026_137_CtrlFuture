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

ADC1 CFGR1 Configurable
    [Tags]    L2-state
    Start STM32N6
    # CFGR1 @ 0x0C
    Write ADC Register    0x0C    0x0000ABCD
    ${val}=    Read ADC Register    0x0C
    ${val}=    Convert To Integer    ${val}
    Should Be Equal As Integers    ${val}    0x0000ABCD

ADC1 IER Configurable
    [Tags]    L2-state
    Start STM32N6
    # IER @ 0x04 (was missing from the model entirely)
    Write ADC Register    0x04    0x00000003
    ${val}=    Read ADC Register    0x04
    ${val}=    Convert To Integer    ${val}
    Should Be Equal As Integers    ${val}    0x00000003

ADC1 JSQR Configurable
    [Tags]    L2-state
    Start STM32N6
    # JSQR @ 0x4C (was previously placed at the wrong offset 0x70)
    Write ADC Register    0x4C    0x00000021
    ${val}=    Read ADC Register    0x4C
    ${val}=    Convert To Integer    ${val}
    Should Be Equal As Integers    ${val}    0x00000021

ADC1 AWD1 Thresholds Configurable
    [Tags]    L2-state
    Start STM32N6
    # AWD1LTR @ 0xA8, AWD1HTR @ 0xAC (real CMSIS watchdog thresholds;
    # a previous revision of this model used fictitious TR1/TR2 at
    # 0x20/0x24, which is CMSIS reserved space)
    Write ADC Register    0xA8    0x00000100
    Write ADC Register    0xAC    0x00000FFF
    ${low}=    Read ADC Register    0xA8
    ${low}=    Convert To Integer    ${low}
    ${high}=    Read ADC Register    0xAC
    ${high}=    Convert To Integer    ${high}
    Should Be Equal As Integers    ${low}    0x00000100
    Should Be Equal As Integers    ${high}    0x00000FFF

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
