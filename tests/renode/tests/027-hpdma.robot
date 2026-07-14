*** Settings ***
Resource        resources/stm32n6-common.robot
Force Tags      hpdma

*** Variables ***
${HPDMA_BASE}   0x48020000

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
    [Tags]    L1-register
    Start STM32N6
    # SECCFGR @ 0x00
    ${val}=    Read HPDMA Register    0x00
    Should Be Equal As Integers    ${val}    0

HPDMA SECCFGR Writable
    [Tags]    L1-register
    Start STM32N6
    # SECCFGR @ 0x00
    Write HPDMA Register    0x00    0x01
    ${val}=    Read HPDMA Register    0x00
    Should Be Equal As Integers    ${val}    0x01

HPDMA Channel 0 CLBAR Accessible
    [Tags]    L1-register
    Start STM32N6
    # Channel 0: CLBAR @ 0x50
    ${val}=    Read HPDMA Register    0x50
    Should Be True    int(${val}) >= 0

Boot Regression
    [Tags]    boot-regression
    Start STM32N6
    Wait For NSH

HPDMA Base Is Not GPDMA
    [Tags]    L1-register
    Create STM32N6 Machine
    # Writing HPDMA SECCFGR must not change GPDMA SECCFGR
    Execute Command    sysbus WriteDoubleWord 0x48020000 0xA5A5A5A5
    ${g}=    Execute Command    sysbus ReadDoubleWord 0x40021000
    ${g}=    Strip String    ${g}
    Should Not Be Equal As Integers    ${g}    0xA5A5A5A5
    ${h}=    Execute Command    sysbus ReadDoubleWord 0x48020000
    ${h}=    Strip String    ${h}
    Should Be Equal As Integers    ${h}    0xA5A5A5A5
