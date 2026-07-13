*** Settings ***
Resource        resources/stm32n6-common.robot

*** Variables ***
${DTS_BASE}     0x4600A000

*** Keywords ***
Read DTS Register
    [Arguments]    ${offset}
    ${addr}=    Evaluate    ${DTS_BASE} + ${offset}
    ${val}=     Execute Command    sysbus ReadDoubleWord ${addr}
    ${val}=     Strip String    ${val}
    RETURN    ${val}

Write DTS Register
    [Arguments]    ${offset}    ${value}
    ${addr}=    Evaluate    ${DTS_BASE} + ${offset}
    Execute Command    sysbus WriteDoubleWord ${addr} ${value}

*** Test Cases ***
DTS CFGR1 Reset Value
    Start STM32N6
    ${val}=    Read DTS Register    0x00
    Should Be Equal As Integers    ${val}    0

DTS CFGR2 Writable
    Start STM32N6
    Write DTS Register    0x04    0x01
    ${val}=    Read DTS Register    0x04
    Should Be Equal As Integers    ${val}    0x01

DTS T0VALR1 Accessible
    Start STM32N6
    ${val}=    Read DTS Register    0x08
    Should Be True    int(${val}) >= 0

Boot Regression
    Start STM32N6
    Wait For NSH
