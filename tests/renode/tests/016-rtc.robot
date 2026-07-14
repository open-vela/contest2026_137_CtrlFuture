*** Settings ***
Resource        resources/stm32n6-common.robot
Force Tags      rtc

*** Variables ***
${RTC_BASE}     0x46004000
${REG_TR}       0x00
${REG_DR}       0x04
${REG_ICSR}     0x0C
${REG_PRER}     0x10
${REG_CR}       0x18
${REG_WPR}      0x24

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

Unlock RTC Write Protection
    Write RTC Register    ${REG_WPR}    0xCA
    Write RTC Register    ${REG_WPR}    0x53

*** Test Cases ***
RTC ICSR Reset Value
    [Tags]    L1-register
    Start STM32N6
    ${val}=    Read RTC Register    ${REG_ICSR}
    # ICSR reset: WUTWF (bit 2) set
    Should Be Equal As Integers    ${val}    0x04

RTC Time Register Accessible
    [Tags]    L1-register
    Start STM32N6
    ${val}=    Read RTC Register    ${REG_TR}
    Should Be True    int(${val}) >= 0

RTC Init Mode Entry Requires Unlock
    [Tags]    L2-state
    Start STM32N6
    # Without unlock, INIT write is ignored
    Write RTC Register    ${REG_ICSR}    0x80
    ${isr}=    Read RTC Register    ${REG_ICSR}
    ${isr}=    Convert To Integer    ${isr}
    Should Be True    (${isr} & 0xC0) == 0
    # Unlock then set INIT
    Unlock RTC Write Protection
    Write RTC Register    ${REG_ICSR}    0x80
    ${isr2}=    Read RTC Register    ${REG_ICSR}
    ${isr2}=    Convert To Integer    ${isr2}
    # INITF (bit 6) and INIT (bit 7) set
    Should Be True    (${isr2} & 0xC0) == 0xC0

RTC Init Exit Sets RSF And INITS
    [Tags]    L2-state
    Start STM32N6
    Unlock RTC Write Protection
    Write RTC Register    ${REG_ICSR}    0x80
    # Program calendar in init mode
    Write RTC Register    ${REG_TR}    0x00123000
    Write RTC Register    ${REG_DR}    0x00240101
    # Exit init mode
    Write RTC Register    ${REG_ICSR}    0x00
    ${isr}=    Read RTC Register    ${REG_ICSR}
    ${isr}=    Convert To Integer    ${isr}
    # INITF clear, RSF (bit5) and INITS (bit4) set, WUTWF (bit2)
    Should Be True    (${isr} & 0x40) == 0
    Should Be True    (${isr} & 0x20) == 0x20
    Should Be True    (${isr} & 0x10) == 0x10
    Should Be True    (${isr} & 0x04) == 0x04
    ${tr}=    Read RTC Register    ${REG_TR}
    Should Be Equal As Integers    ${tr}    0x00123000

RTC CR Protected Until Unlock
    [Tags]    L2-state
    Start STM32N6
    Write RTC Register    ${REG_CR}    0x400
    ${cr}=    Read RTC Register    ${REG_CR}
    Should Be Equal As Integers    ${cr}    0
    Unlock RTC Write Protection
    Write RTC Register    ${REG_CR}    0x400
    ${cr2}=    Read RTC Register    ${REG_CR}
    Should Be Equal As Integers    ${cr2}    0x400

RTC WPR Relock
    [Tags]    L2-state
    Start STM32N6
    Unlock RTC Write Protection
    Write RTC Register    ${REG_CR}    0x100
    # Any non-sequence key re-locks
    Write RTC Register    ${REG_WPR}    0xFF
    Write RTC Register    ${REG_CR}    0x200
    ${cr}=    Read RTC Register    ${REG_CR}
    Should Be Equal As Integers    ${cr}    0x100

Boot Regression
    [Tags]    boot-regression
    Start STM32N6
    Wait For NSH
