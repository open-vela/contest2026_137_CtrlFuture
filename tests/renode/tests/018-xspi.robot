*** Settings ***
Resource        resources/stm32n6-common.robot
Force Tags      xspi

*** Variables ***
${XSPI_BASE}    0x48025000
${REG_CR}       0x00
${REG_DCR1}     0x08
${REG_SR}       0x20
${REG_FCR}      0x24
${REG_CCR}      0x100
${REG_IR}       0x110

*** Keywords ***
Read XSPI Register
    [Arguments]    ${offset}
    ${addr}=    Evaluate    ${XSPI_BASE} + ${offset}
    ${val}=     Execute Command    sysbus ReadDoubleWord ${addr}
    ${val}=     Strip String    ${val}
    RETURN    ${val}

Write XSPI Register
    [Arguments]    ${offset}    ${value}
    ${addr}=    Evaluate    ${XSPI_BASE} + ${offset}
    Execute Command    sysbus WriteDoubleWord ${addr} ${value}

*** Test Cases ***
XSPI1 SR Reset Value
    [Tags]    L1-register
    Start STM32N6
    ${val}=    Read XSPI Register    ${REG_SR}
    # SR: BUSY (bit 5) should be clear, TCF clear
    Should Be Equal As Integers    ${val}    0

XSPI1 CR Writable
    [Tags]    L1-register
    Start STM32N6
    Write XSPI Register    ${REG_CR}    0x01
    ${val}=    Read XSPI Register    ${REG_CR}
    Should Be Equal As Integers    ${val}    0x01

XSPI1 DCR1 Writable
    [Tags]    L1-register
    Start STM32N6
    Write XSPI Register    ${REG_DCR1}    0x00FF0000
    ${val}=    Read XSPI Register    ${REG_DCR1}
    Should Be Equal As Integers    ${val}    0x00FF0000

XSPI1 BUSY Always Clear
    [Tags]    L2-state
    Start STM32N6
    Write XSPI Register    ${REG_CR}    0x01
    Write XSPI Register    ${REG_IR}    0x9F
    ${sr}=    Read XSPI Register    ${REG_SR}
    ${sr}=    Convert To Integer    ${sr}
    Should Be True    (${sr} & 0x20) == 0

XSPI1 IR Completes With TCF
    [Tags]    L2-state
    Start STM32N6
    Write XSPI Register    ${REG_CR}    0x01
    Write XSPI Register    ${REG_IR}    0x05
    ${sr}=    Read XSPI Register    ${REG_SR}
    ${sr}=    Convert To Integer    ${sr}
    # TCF bit 1
    Should Be True    (${sr} & 0x02) == 0x02

XSPI1 FCR Clears TCF
    [Tags]    L2-state
    Start STM32N6
    Write XSPI Register    ${REG_CR}    0x01
    Write XSPI Register    ${REG_CCR}    0x01
    ${sr}=    Read XSPI Register    ${REG_SR}
    ${sr}=    Convert To Integer    ${sr}
    Should Be True    (${sr} & 0x02) == 0x02
    Write XSPI Register    ${REG_FCR}    0x02
    ${sr2}=    Read XSPI Register    ${REG_SR}
    ${sr2}=    Convert To Integer    ${sr2}
    Should Be True    (${sr2} & 0x02) == 0

Boot Regression
    [Tags]    boot-regression
    Start STM32N6
    Wait For NSH
