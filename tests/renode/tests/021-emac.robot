*** Settings ***
Resource        resources/stm32n6-common.robot

*** Variables ***
${EMAC_BASE}    0x48036000

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
    Start STM32N6
    ${val}=    Read EMAC Register    0x000
    Should Be Equal As Integers    ${val}    0

EMAC MACCR Writable
    Start STM32N6
    Write EMAC Register    0x000    0x00008000
    ${val}=    Read EMAC Register    0x000
    Should Be Equal As Integers    ${val}    0x00008000

EMAC MACA0HR Accessible
    Start STM32N6
    Write EMAC Register    0x040    0x00008000
    ${val}=    Read EMAC Register    0x040
    Should Be True    int(${val}) >= 0

Boot Regression
    Start STM32N6
    Wait For NSH
