*** Settings ***
Resource        resources/stm32n6-common.robot

*** Variables ***
${LTDC_BASE}    0x50001000

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
    Start STM32N6
    # GCR @ 0x18
    ${val}=    Read LTDC Register    0x18
    Should Be Equal As Integers    ${val}    0

LTDC GCR Writable
    Start STM32N6
    # GCR @ 0x18
    Write LTDC Register    0x18    0x01
    ${val}=    Read LTDC Register    0x18
    Should Be Equal As Integers    ${val}    0x01

LTDC ISR Accessible
    Start STM32N6
    # ISR @ 0x38
    ${val}=    Read LTDC Register    0x38
    Should Be True    int(${val}) >= 0

Boot Regression
    Start STM32N6
    Wait For NSH
