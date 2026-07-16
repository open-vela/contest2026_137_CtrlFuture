*** Settings ***
Resource        resources/stm32n6-common.robot
Force Tags      otp

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
    [Tags]    L1-register
    Start STM32N6
    ${val}=    Read OTP Register    0x00
    Should Be Equal As Integers    ${val}    0

OTP AR Writable
    [Tags]    L1-register
    Start STM32N6
    Write OTP Register    0x08    0x01
    ${val}=    Read OTP Register    0x08
    Should Be Equal As Integers    ${val}    0x01

OTP DR Writable
    [Tags]    L1-register
    Start STM32N6
    # NOTE: this is a team-defined placeholder register (see
    # STM32N6_OTP.cs header); write/read round-trip instead of the
    # previous "int(val) >= 0" check on the read-only SR register,
    # which is true for any 32-bit unsigned read.
    Write OTP Register    0x0C    0x0000FFFF
    ${val}=    Read OTP Register    0x0C
    ${val}=    Convert To Integer    ${val}
    Should Be Equal As Integers    ${val}    0x0000FFFF

Boot Regression
    [Tags]    boot-regression
    Start STM32N6
    Wait For NSH
