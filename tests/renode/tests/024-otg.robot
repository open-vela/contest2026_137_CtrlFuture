*** Settings ***
Resource        resources/stm32n6-common.robot

*** Variables ***
${OTG_BASE}     0x48040000

*** Keywords ***
Read OTG Register
    [Arguments]    ${offset}
    ${addr}=    Evaluate    ${OTG_BASE} + ${offset}
    ${val}=     Execute Command    sysbus ReadDoubleWord ${addr}
    ${val}=     Strip String    ${val}
    RETURN    ${val}

Write OTG Register
    [Arguments]    ${offset}    ${value}
    ${addr}=    Evaluate    ${OTG_BASE} + ${offset}
    Execute Command    sysbus WriteDoubleWord ${addr} ${value}

*** Test Cases ***
OTG GOTGCTL Reset Value
    Start STM32N6
    ${val}=    Read OTG Register    0x000
    Should Be Equal As Integers    ${val}    0

OTG GAHBCFG Writable
    Start STM32N6
    Write OTG Register    0x008    0x00000001
    ${val}=    Read OTG Register    0x008
    Should Be Equal As Integers    ${val}    0x00000001

OTG GINTSTS Accessible
    Start STM32N6
    ${val}=    Read OTG Register    0x014
    Should Be True    int(${val}) >= 0

Boot Regression
    Start STM32N6
    Wait For NSH
