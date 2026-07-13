*** Settings ***
Resource        resources/stm32n6-common.robot

*** Variables ***
${IWDG_BASE}    0x46004800

*** Keywords ***
Read IWDG Register
    [Arguments]    ${offset}
    ${addr}=    Evaluate    ${IWDG_BASE} + ${offset}
    ${val}=     Execute Command    sysbus ReadDoubleWord ${addr}
    ${val}=     Strip String    ${val}
    RETURN    ${val}

Write IWDG Register
    [Arguments]    ${offset}    ${value}
    ${addr}=    Evaluate    ${IWDG_BASE} + ${offset}
    Execute Command    sysbus WriteDoubleWord ${addr} ${value}

*** Test Cases ***
IWDG SR Reset Value
    Start STM32N6
    ${val}=    Read IWDG Register    0x0C
    # SR reset: all flags clear (0x00)
    Should Be Equal As Integers    ${val}    0

IWDG ONF After Enable
    Start STM32N6
    # Enable watchdog with key 0xCCCC
    Write IWDG Register    0x00    0xCCCC
    ${sr}=    Read IWDG Register    0x0C
    # ONF (bit 8) should be set
    ${onf}=    Evaluate    (${sr} >> 8) & 1
    Should Be Equal As Integers    ${onf}    1

IWDG Unlock And Set Prescaler
    Start STM32N6
    # Unlock PR/RLR/WINR with key 0x5555
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

IWDG Window Register Accessible
    Start STM32N6
    # Unlock
    Write IWDG Register    0x00    0x5555
    # WINR @ 0x10: write window value
    Write IWDG Register    0x10    0x800
    ${winr}=    Read IWDG Register    0x10
    Should Be Equal As Integers    ${winr}    0x800

Boot Regression
    Start STM32N6
    Wait For NSH
