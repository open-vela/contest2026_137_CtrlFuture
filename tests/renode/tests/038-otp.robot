*** Settings ***
Resource        resources/stm32n6-common.robot

*** Variables ***
${OTP_BASE}     0x46009000

*** Keywords ***
Read OTP Register
    [Arguments]    ${offset}
    ${addr}=    Evaluate    ${OTP_BASE} + ${offset}
    ${val}=     Execute Command    sysbus ReadDoubleWord ${addr}
    ${val}=     Strip String    ${val}
    RETURN    ${val}

Write OTP Register
    [Arguments]    ${offset}    ${value}
    ${addr}=    Evaluate    ${OTP_BASE} + ${offset}
    Execute Command    sysbus WriteDoubleWord ${addr} ${value}

*** Test Cases ***
OTP CR Reset Value
    Start STM32N6
    ${val}=    Read OTP Register    0x00
    Should Be Equal As Integers    ${val}    0

OTP AR Writable
    Start STM32N6
    Write OTP Register    0x08    0x01
    ${val}=    Read OTP Register    0x08
    Should Be Equal As Integers    ${val}    0x01

OTP SR Accessible
    Start STM32N6
    ${val}=    Read OTP Register    0x04
    Should Be True    int(${val}) >= 0

Boot Regression
    Start STM32N6
    Wait For NSH
