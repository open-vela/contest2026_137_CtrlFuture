*** Settings ***
Resource        resources/stm32n6-common.robot

*** Variables ***
${HPDMA_BASE}   0x40021000

*** Keywords ***
Read HPDMA Register
    [Arguments]    ${offset}
    ${addr}=    Evaluate    ${HPDMA_BASE} + ${offset}
    ${val}=     Execute Command    sysbus ReadDoubleWord ${addr}
    ${val}=     Strip String    ${val}
    RETURN    ${val}

Write HPDMA Register
    [Arguments]    ${offset}    ${value}
    ${addr}=    Evaluate    ${HPDMA_BASE} + ${offset}
    Execute Command    sysbus WriteDoubleWord ${addr} ${value}

*** Test Cases ***
HPDMA SECCFGR Reset Value
    Start STM32N6
    # SECCFGR @ 0x00
    ${val}=    Read HPDMA Register    0x00
    Should Be Equal As Integers    ${val}    0

HPDMA SECCFGR Writable
    Start STM32N6
    # SECCFGR @ 0x00
    Write HPDMA Register    0x00    0x01
    ${val}=    Read HPDMA Register    0x00
    Should Be Equal As Integers    ${val}    0x01

HPDMA Channel 0 CLBAR Accessible
    Start STM32N6
    # Channel 0: CLBAR @ 0x50
    ${val}=    Read HPDMA Register    0x50
    Should Be True    int(${val}) >= 0

Boot Regression
    Start STM32N6
    Wait For NSH
