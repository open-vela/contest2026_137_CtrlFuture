*** Settings ***
Resource        resources/stm32n6-common.robot

*** Variables ***
${I2C1_BASE}    0x40005400

*** Keywords ***
Read I2C Register
    [Arguments]    ${offset}
    ${addr}=    Evaluate    ${I2C1_BASE} + ${offset}
    ${val}=     Execute Command    sysbus ReadDoubleWord ${addr}
    ${val}=     Strip String    ${val}
    [Return]    ${val}

Write I2C Register
    [Arguments]    ${offset}    ${value}
    ${addr}=    Evaluate    ${I2C1_BASE} + ${offset}
    Execute Command    sysbus WriteDoubleWord ${addr} ${value}

*** Test Cases ***
I2C1 CR1 Reset Value
    Start STM32N6
    ${val}=    Read I2C Register    0x00
    Should Be Equal As Integers    ${val}    0

I2C1 TIMINGR Writable
    Start STM32N6
    Write I2C Register    0x10    0x10909CEC
    ${val}=    Read I2C Register    0x10
    Should Be Equal As Integers    ${val}    0x10909CEC

I2C1 ISR TXE Set
    Start STM32N6
    ${val}=    Read I2C Register    0x18
    # TXE (bit 0) should be set at reset
    ${txe}=    Evaluate    ${val} & 1
    Should Be Equal As Integers    ${txe}    1

I2C1 ISR BUSY Clear
    Start STM32N6
    ${val}=    Read I2C Register    0x18
    # BUSY (bit 14) should be clear
    ${busy}=    Evaluate    (${val} >> 14) & 1
    Should Be Equal As Integers    ${busy}    0

Boot Regression
    Start STM32N6
    Wait For NSH
