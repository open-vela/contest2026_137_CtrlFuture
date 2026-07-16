*** Settings ***
Resource        resources/stm32n6-common.robot
Force Tags      dts

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
    [Tags]    L1-register
    Start STM32N6
    ${val}=    Read DTS Register    0x00
    Should Be Equal As Integers    ${val}    0

DTS CFGR2 Writable
    [Tags]    L1-register
    Start STM32N6
    Write DTS Register    0x04    0x01
    ${val}=    Read DTS Register    0x04
    Should Be Equal As Integers    ${val}    0x01

DTS TSLPTR Writable
    [Tags]    L1-register
    Start STM32N6
    # NOTE: this is a team-defined placeholder register (see
    # STM32N6_DTS.cs header); write/read round-trip instead of the
    # previous "int(val) >= 0" check on the read-only T0VALR1
    # register, which is true for any 32-bit unsigned read.
    Write DTS Register    0x14    0x12345678
    ${val}=    Read DTS Register    0x14
    ${val}=    Convert To Integer    ${val}
    Should Be Equal As Integers    ${val}    0x12345678

Boot Regression
    [Tags]    boot-regression
    Start STM32N6
    Wait For NSH
