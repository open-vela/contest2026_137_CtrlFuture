*** Settings ***
Resource        resources/stm32n6-common.robot
Force Tags      ltdc

*** Variables ***
${LTDC_BASE}    0x48001000
${REG_GCR}      0x18
${REG_SRCR}     0x24
${REG_ISR}      0x38
${REG_L0_CR}    0x80

*** Keywords ***
Read LTDC Register
    [Arguments]    ${offset}
    ${addr}=    Evaluate    ${LTDC_BASE} + ${offset}
    ${val}=     Execute Command    sysbus ReadDoubleWord ${addr}
    ${val}=     Strip String    ${val}
    RETURN    ${val}

Write LTDC Register
    [Arguments]    ${offset}    ${value}
    ${addr}=    Evaluate    ${LTDC_BASE} + ${offset}
    Execute Command    sysbus WriteDoubleWord ${addr} ${value}

*** Test Cases ***
LTDC GCR Reset Value
    [Tags]    L1-register
    Start STM32N6
    ${val}=    Read LTDC Register    ${REG_GCR}
    Should Be Equal As Integers    ${val}    0

LTDC GCR Writable
    [Tags]    L1-register
    Start STM32N6
    Write LTDC Register    ${REG_GCR}    0x01
    ${val}=    Read LTDC Register    ${REG_GCR}
    Should Be Equal As Integers    ${val}    0x01

LTDC ISR Accessible
    [Tags]    L1-register
    Start STM32N6
    ${val}=    Read LTDC Register    ${REG_ISR}
    Should Be True    int(${val}) >= 0

LTDC GCR LTDCEN Sticks
    [Tags]    L2-state
    Start STM32N6
    Write LTDC Register    ${REG_GCR}    0x01
    ${val}=    Read LTDC Register    ${REG_GCR}
    ${val}=    Convert To Integer    ${val}
    Should Be True    (${val} & 0x1) == 0x1

LTDC SRCR Self Clears
    [Tags]    L2-state
    Start STM32N6
    # IMR bit0 | VBR bit1
    Write LTDC Register    ${REG_SRCR}    0x03
    ${val}=    Read LTDC Register    ${REG_SRCR}
    Should Be Equal As Integers    ${val}    0

LTDC Layer0 Enable Sticks
    [Tags]    L2-state
    Start STM32N6
    Write LTDC Register    ${REG_L0_CR}    0x01
    ${val}=    Read LTDC Register    ${REG_L0_CR}
    Should Be Equal As Integers    ${val}    0x01

Boot Regression
    [Tags]    boot-regression
    Start STM32N6
    Wait For NSH
