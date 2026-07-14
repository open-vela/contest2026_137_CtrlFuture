*** Settings ***
Resource        resources/stm32n6-common.robot
Force Tags      sai

*** Variables ***
${SAI_BASE}     0x42005800
${REG_CR1}      0x04
${REG_IMR}      0x14
${REG_SR}       0x18
${REG_CLRFR}    0x1C
${REG_DR}       0x20

*** Keywords ***
Read SAI Register
    [Arguments]    ${offset}
    ${addr}=    Evaluate    ${SAI_BASE} + ${offset}
    ${val}=     Execute Command    sysbus ReadDoubleWord ${addr}
    ${val}=     Strip String    ${val}
    RETURN    ${val}

Write SAI Register
    [Arguments]    ${offset}    ${value}
    ${addr}=    Evaluate    ${SAI_BASE} + ${offset}
    Execute Command    sysbus WriteDoubleWord ${addr} ${value}

*** Test Cases ***
SAI1 CR1 Reset Value
    [Tags]    L1-register
    Start STM32N6
    ${val}=    Read SAI Register    ${REG_CR1}
    Should Be Equal As Integers    ${val}    0

SAI1 CR1 Writable
    [Tags]    L1-register
    Start STM32N6
    Write SAI Register    ${REG_CR1}    0x01000000
    ${val}=    Read SAI Register    ${REG_CR1}
    Should Be Equal As Integers    ${val}    0x01000000

SAI1 SR Accessible
    [Tags]    L1-register
    Start STM32N6
    ${val}=    Read SAI Register    ${REG_SR}
    Should Be True    int(${val}) >= 0

SAI1 IMR Writable
    [Tags]    L1-register
    Start STM32N6
    Write SAI Register    ${REG_IMR}    0x08
    ${val}=    Read SAI Register    ${REG_IMR}
    Should Be Equal As Integers    ${val}    0x08

SAI1 SAIEN Sticks
    [Tags]    L2-state
    Start STM32N6
    # SAIEN bit 16
    Write SAI Register    ${REG_CR1}    0x00010000
    ${val}=    Read SAI Register    ${REG_CR1}
    Should Be Equal As Integers    ${val}    0x00010000

SAI1 FREQ When Enabled
    [Tags]    L2-state
    Start STM32N6
    ${sr0}=    Read SAI Register    ${REG_SR}
    ${sr0}=    Convert To Integer    ${sr0}
    Should Be True    (${sr0} & 0x8) == 0
    Write SAI Register    ${REG_CR1}    0x00010000
    ${sr1}=    Read SAI Register    ${REG_SR}
    ${sr1}=    Convert To Integer    ${sr1}
    # FREQ bit 3 at SR@0x18
    Should Be True    (${sr1} & 0x8) == 0x8

SAI1 DR Writable When Enabled
    [Tags]    L2-state
    Start STM32N6
    Write SAI Register    ${REG_CR1}    0x00010000
    Write SAI Register    ${REG_DR}    0xA5A5A5A5
    ${dr}=    Read SAI Register    ${REG_DR}
    Should Be Equal As Integers    ${dr}    0xA5A5A5A5

Boot Regression
    [Tags]    boot-regression
    Start STM32N6
    Wait For NSH
