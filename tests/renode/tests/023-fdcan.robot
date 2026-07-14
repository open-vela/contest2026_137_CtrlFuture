*** Settings ***
Resource        resources/stm32n6-common.robot
Force Tags      fdcan

*** Variables ***
${FDCAN_BASE}   0x4000A000
${REG_CREL}     0x00
${REG_ENDN}     0x04
${REG_CCCR}     0x18

*** Keywords ***
Read FDCAN Register
    [Arguments]    ${offset}
    ${addr}=    Evaluate    ${FDCAN_BASE} + ${offset}
    ${val}=     Execute Command    sysbus ReadDoubleWord ${addr}
    ${val}=     Strip String    ${val}
    RETURN    ${val}

Write FDCAN Register
    [Arguments]    ${offset}    ${value}
    ${addr}=    Evaluate    ${FDCAN_BASE} + ${offset}
    Execute Command    sysbus WriteDoubleWord ${addr} ${value}

*** Test Cases ***
FDCAN1 CREL Reset Value
    [Tags]    L1-register
    Start STM32N6
    ${val}=    Read FDCAN Register    ${REG_CREL}
    Should Be Equal As Integers    ${val}    0

FDCAN1 ENDN Accessible
    [Tags]    L1-register
    Start STM32N6
    ${val}=    Read FDCAN Register    ${REG_ENDN}
    Should Be True    int(${val}) >= 0

FDCAN1 CCCR Reset In INIT
    [Tags]    L2-state
    Start STM32N6
    ${val}=    Read FDCAN Register    ${REG_CCCR}
    ${val}=    Convert To Integer    ${val}
    # Hardware powers up with INIT set
    Should Be True    (${val} & 0x1) == 0x1

FDCAN1 CCCR INIT Enter Leave
    [Tags]    L2-state
    Start STM32N6
    # Set INIT|CCE
    Write FDCAN Register    ${REG_CCCR}    0x00000003
    ${val}=    Read FDCAN Register    ${REG_CCCR}
    Should Be Equal As Integers    ${val}    0x00000003
    # Leave INIT — CCE forced clear
    Write FDCAN Register    ${REG_CCCR}    0x00000000
    ${val2}=    Read FDCAN Register    ${REG_CCCR}
    ${val2}=    Convert To Integer    ${val2}
    Should Be True    (${val2} & 0x3) == 0

FDCAN1 CCCR Writable
    [Tags]    L1-register
    Start STM32N6
    Write FDCAN Register    ${REG_CCCR}    0x00000001
    ${val}=    Read FDCAN Register    ${REG_CCCR}
    Should Be Equal As Integers    ${val}    0x00000001

Boot Regression
    [Tags]    boot-regression
    Start STM32N6
    Wait For NSH
