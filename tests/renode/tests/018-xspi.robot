*** Settings ***
Resource        resources/stm32n6-common.robot

*** Variables ***
${XSPI_BASE}    0x48025000

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
    Start STM32N6
    ${val}=    Read XSPI Register    0x20
    # SR: BUSY (bit 5) should be clear, TCF (bit 0) should be clear
    Should Be Equal As Integers    ${val}    0

XSPI1 CR Writable
    Start STM32N6
    Write XSPI Register    0x00    0x01
    ${val}=    Read XSPI Register    0x00
    Should Be Equal As Integers    ${val}    0x01

XSPI1 DCR1 Writable
    Start STM32N6
    Write XSPI Register    0x08    0x00FF0000
    ${val}=    Read XSPI Register    0x08
    Should Be Equal As Integers    ${val}    0x00FF0000

Boot Regression
    Start STM32N6
    Wait For NSH
