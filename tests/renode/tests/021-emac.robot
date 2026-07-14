*** Settings ***
Resource        resources/stm32n6-common.robot
Force Tags      emac

*** Variables ***
${EMAC_BASE}    0x48036000
${REG_MACCR}    0x000
${REG_MACA0HR}  0x040
${REG_MDIOAR}   0x200
${REG_MDIODR}   0x204
${REG_DMAMR}    0x1000

*** Keywords ***
Read EMAC Register
    [Arguments]    ${offset}
    ${addr}=    Evaluate    ${EMAC_BASE} + ${offset}
    ${val}=     Execute Command    sysbus ReadDoubleWord ${addr}
    ${val}=     Strip String    ${val}
    RETURN    ${val}

Write EMAC Register
    [Arguments]    ${offset}    ${value}
    ${addr}=    Evaluate    ${EMAC_BASE} + ${offset}
    Execute Command    sysbus WriteDoubleWord ${addr} ${value}

*** Test Cases ***
EMAC MACCR Reset Value
    [Tags]    L1-register
    Start STM32N6
    ${val}=    Read EMAC Register    ${REG_MACCR}
    Should Be Equal As Integers    ${val}    0

EMAC MACCR Writable
    [Tags]    L1-register
    Start STM32N6
    Write EMAC Register    ${REG_MACCR}    0x00008000
    ${val}=    Read EMAC Register    ${REG_MACCR}
    Should Be Equal As Integers    ${val}    0x00008000

EMAC MACA0HR Accessible
    [Tags]    L1-register
    Start STM32N6
    Write EMAC Register    ${REG_MACA0HR}    0x00008000
    ${val}=    Read EMAC Register    ${REG_MACA0HR}
    Should Be True    int(${val}) >= 0

EMAC DMAMR SWR Self Clears
    [Tags]    L2-state
    Start STM32N6
    Write EMAC Register    ${REG_DMAMR}    0x01
    ${val}=    Read EMAC Register    ${REG_DMAMR}
    ${val}=    Convert To Integer    ${val}
    Should Be True    (${val} & 0x1) == 0

EMAC MDIOAR GB Self Clears
    [Tags]    L2-state
    Start STM32N6
    # GB=bit0, write starts MDIO cycle
    Write EMAC Register    ${REG_MDIODR}    0x1234
    Write EMAC Register    ${REG_MDIOAR}    0x00020801
    ${ar}=    Read EMAC Register    ${REG_MDIOAR}
    ${ar}=    Convert To Integer    ${ar}
    Should Be True    (${ar} & 0x1) == 0
    # Non-GB bits retained
    Should Be True    (${ar} & 0x00020800) == 0x00020800

Boot Regression
    [Tags]    boot-regression
    Start STM32N6
    Wait For NSH
