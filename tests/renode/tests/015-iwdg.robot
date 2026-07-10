*** Settings ***
Resource        resources/stm32n6-common.robot

*** Variables ***
${IWDG_BASE}    0x40003000

*** Keywords ***
Read IWDG Register
    [Arguments]    ${offset}
    ${addr}=    Evaluate    ${IWDG_BASE} + ${offset}
    ${val}=     Execute Command    sysbus ReadDoubleWord ${addr}
    ${val}=     Strip String    ${val}
    [Return]    ${val}

Write IWDG Register
    [Arguments]    ${offset}    ${value}
    ${addr}=    Evaluate    ${IWDG_BASE} + ${offset}
    Execute Command    sysbus WriteDoubleWord ${addr} ${value}

*** Test Cases ***
IWDG SR Reset Value
    Start STM32N6
    ${val}=    Read IWDG Register    0x0C
    # SR bits PVU/RVU/WVU should all be 0
    Should Be Equal As Integers    ${val}    0

IWDG Unlock And Set Prescaler
    Start STM32N6
    # Unlock PR/RLR with key 0x5555
    Write IWDG Register    0x00    0x5555
    # Set prescaler to 4
    Write IWDG Register    0x04    4
    ${pr}=    Read IWDG Register    0x04
    Should Be Equal As Integers    ${pr}    4

IWDG Set Reload
    Start STM32N6
    # Unlock
    Write IWDG Register    0x00    0x5555
    # Set reload value
    Write IWDG Register    0x08    0xFFF
    ${rlr}=    Read IWDG Register    0x08
    Should Be Equal As Integers    ${rlr}    0xFFF

Boot Regression
    Start STM32N6
    Wait For NSH
