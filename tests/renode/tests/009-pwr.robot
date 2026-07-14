*** Settings ***
Resource        resources/stm32n6-common.robot
Force Tags      pwr

*** Variables ***
${PWR_BASE}     0x46024800
${VOSCR}        0x020

*** Keywords ***
Read PWR Register
    [Arguments]    ${offset}
    ${addr}=    Evaluate    ${PWR_BASE} + ${offset}
    ${val}=     Execute Command    sysbus ReadDoubleWord ${addr}
    ${val}=     Strip String    ${val}
    RETURN    ${val}

Write PWR Register
    [Arguments]    ${offset}    ${value}
    ${addr}=    Evaluate    ${PWR_BASE} + ${offset}
    Execute Command    sysbus WriteDoubleWord ${addr} ${value}

*** Test Cases ***
PWR VOSCR Reset Value
    [Documentation]    VOSCR should reset to 0
    [Tags]    L1-register
    Create STM32N6 Machine
    ${val}=    Read PWR Register    ${VOSCR}
    Should Be Equal As Integers    ${val}    0

PWR Set VOS Scale 0
    [Documentation]    Writing VOS=0 should set VOSRDY and ACTVOS
    [Tags]    L2-state
    Create STM32N6 Machine
    Write PWR Register    ${VOSCR}    0x00
    ${val}=    Read PWR Register    ${VOSCR}
    # VOSRDY (bit 1) and ACTVOSRDY (bit 17) should be set
    Should Be Equal As Integers    ${val}    0x00020002

PWR Set VOS Scale 1
    [Documentation]    Writing VOS=1 should set VOSRDY and ACTVOS
    [Tags]    L2-state
    Create STM32N6 Machine
    Write PWR Register    ${VOSCR}    0x01
    ${val}=    Read PWR Register    ${VOSCR}
    # VOS (bit 0)=1, VOSRDY (bit 1)=1, ACTVOS (bit 16)=1, ACTVOSRDY (bit 17)=1
    Should Be Equal As Integers    ${val}    0x00030003

Boot Regression
    [Documentation]    NSH prompt should still work after PWR model addition
    [Tags]    boot-regression
    Start STM32N6
    Wait For NSH
