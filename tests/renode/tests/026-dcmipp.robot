*** Settings ***
Resource        resources/stm32n6-common.robot
Force Tags      dcmipp

*** Variables ***
${DCMIPP_BASE}  0x48002000
${REG_IPGR1}    0x00
${REG_SR}       0x04
${REG_PRCR}     0x104
${REG_P0FCTCR}  0x500
${REG_P0SR}     0x5F8

*** Keywords ***
Read DCMIPP Register
    [Arguments]    ${offset}
    ${addr}=    Evaluate    ${DCMIPP_BASE} + ${offset}
    ${val}=     Execute Command    sysbus ReadDoubleWord ${addr}
    ${val}=     Strip String    ${val}
    RETURN    ${val}

Write DCMIPP Register
    [Arguments]    ${offset}    ${value}
    ${addr}=    Evaluate    ${DCMIPP_BASE} + ${offset}
    Execute Command    sysbus WriteDoubleWord ${addr} ${value}

*** Test Cases ***
DCMIPP IPGR1 Reset Value
    [Tags]    L1-register
    Start STM32N6
    ${val}=    Read DCMIPP Register    ${REG_IPGR1}
    Should Be Equal As Integers    ${val}    0

DCMIPP IPGR1 Writable
    [Tags]    L1-register
    Start STM32N6
    Write DCMIPP Register    ${REG_IPGR1}    0x01
    ${val}=    Read DCMIPP Register    ${REG_IPGR1}
    Should Be Equal As Integers    ${val}    0x01

DCMIPP SR Accessible
    [Tags]    L1-register
    Start STM32N6
    ${val}=    Read DCMIPP Register    ${REG_SR}
    Should Be True    int(${val}) >= 0

DCMIPP PRCR ENABLE Sticks
    [Tags]    L2-state
    Start STM32N6
    # ENABLE bit14 = 0x4000
    Write DCMIPP Register    ${REG_PRCR}    0x4000
    ${val}=    Read DCMIPP Register    ${REG_PRCR}
    ${val}=    Convert To Integer    ${val}
    Should Be True    (${val} & 0x4000) == 0x4000

DCMIPP P0FCTCR CPTREQ Sticks
    [Tags]    L2-state
    Start STM32N6
    # CPTREQ bit3 = 0x8
    Write DCMIPP Register    ${REG_P0FCTCR}    0x08
    ${val}=    Read DCMIPP Register    ${REG_P0FCTCR}
    Should Be Equal As Integers    ${val}    0x08

DCMIPP P0SR Accessible
    [Tags]    L2-state
    Start STM32N6
    ${val}=    Read DCMIPP Register    ${REG_P0SR}
    Should Be Equal As Integers    ${val}    0

Boot Regression
    [Tags]    boot-regression
    Start STM32N6
    Wait For NSH
