*** Settings ***
Resource        resources/stm32n6-common.robot
Force Tags      iwdg

*** Variables ***
${IWDG_BASE}    0x46004800
${REG_KR}       0x00
${REG_PR}       0x04
${REG_RLR}      0x08
${REG_SR}       0x0C
${REG_WINR}     0x10

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
    [Tags]    L1-register
    Start STM32N6
    ${val}=    Read IWDG Register    ${REG_SR}
    # SR reset: all flags clear (0x00)
    Should Be Equal As Integers    ${val}    0

IWDG ONF After Enable
    [Tags]    L2-state
    Start STM32N6
    # Enable watchdog with key 0xCCCC
    Write IWDG Register    ${REG_KR}    0xCCCC
    ${sr}=    Read IWDG Register    ${REG_SR}
    # ONF (bit 8) should be set
    ${onf}=    Evaluate    (${sr} >> 8) & 1
    Should Be Equal As Integers    ${onf}    1

IWDG Unlock And Set Prescaler
    [Tags]    L2-state
    Start STM32N6
    # Unlock PR/RLR/WINR with key 0x5555
    Write IWDG Register    ${REG_KR}    0x5555
    # Set prescaler to 4
    Write IWDG Register    ${REG_PR}    4
    ${pr}=    Read IWDG Register    ${REG_PR}
    Should Be Equal As Integers    ${pr}    4

IWDG Set Reload
    [Tags]    L2-state
    Start STM32N6
    # Unlock
    Write IWDG Register    ${REG_KR}    0x5555
    # Set reload value
    Write IWDG Register    ${REG_RLR}    0xFFF
    ${rlr}=    Read IWDG Register    ${REG_RLR}
    Should Be Equal As Integers    ${rlr}    0xFFF

IWDG Window Register Accessible
    [Tags]    L2-state
    Start STM32N6
    # Unlock
    Write IWDG Register    ${REG_KR}    0x5555
    # WINR @ 0x10: write window value
    Write IWDG Register    ${REG_WINR}    0x800
    ${winr}=    Read IWDG Register    ${REG_WINR}
    Should Be Equal As Integers    ${winr}    0x800

IWDG PVU RVU Clear For Driver Wait
    [Tags]    L2-state
    Start STM32N6
    Write IWDG Register    ${REG_KR}    0x5555
    Write IWDG Register    ${REG_PR}    3
    Write IWDG Register    ${REG_RLR}    0xAAA
    ${sr}=    Read IWDG Register    ${REG_SR}
    ${sr}=    Convert To Integer    ${sr}
    # Driver polls !PVU and !RVU — must stay clear in L2 sim
    Should Be True    (${sr} & 0x3) == 0

IWDG Locked Write Ignored
    [Tags]    L2-state
    Start STM32N6
    # Without unlock, PR write is ignored
    Write IWDG Register    ${REG_PR}    5
    ${pr}=    Read IWDG Register    ${REG_PR}
    Should Be Equal As Integers    ${pr}    0

Boot Regression
    [Tags]    boot-regression
    Start STM32N6
    Wait For NSH
