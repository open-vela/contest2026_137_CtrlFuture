*** Settings ***
Resource        resources/stm32n6-common.robot

*** Variables ***
${RTC_BASE}     0x46004000

*** Keywords ***
Read RTC Register
    [Arguments]    ${offset}
    ${addr}=    Evaluate    ${RTC_BASE} + ${offset}
    ${val}=     Execute Command    sysbus ReadDoubleWord ${addr}
    ${val}=     Strip String    ${val}
    RETURN    ${val}

Write RTC Register
    [Arguments]    ${offset}    ${value}
    ${addr}=    Evaluate    ${RTC_BASE} + ${offset}
    Execute Command    sysbus WriteDoubleWord ${addr} ${value}

*** Test Cases ***
RTC ICSR Reset Value
    Start STM32N6
    ${val}=    Read RTC Register    0x0C
    # ICSR reset: WUTWF (bit 2) set
    Should Be Equal As Integers    ${val}    0x04

RTC Init Mode Entry
    Start STM32N6
    # Set INIT bit (bit 7) to enter init mode
    Write RTC Register    0x0C    0x87
    ${isr}=    Read RTC Register    0x0C
    # INITF (bit 6) should be set
    ${initf}=    Evaluate    (${isr} >> 6) & 1
    Should Be Equal As Integers    ${initf}    1

RTC Time Register Accessible
    Start STM32N6
    ${val}=    Read RTC Register    0x00
    Should Be True    int(${val}) >= 0

Boot Regression
    Start STM32N6
    Wait For NSH
