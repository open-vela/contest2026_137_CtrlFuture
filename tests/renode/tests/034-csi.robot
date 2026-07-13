*** Settings ***
Resource        resources/stm32n6-common.robot

*** Variables ***
${CSI_BASE}     0x48006000

*** Keywords ***
Read CSI Register
    [Arguments]    ${offset}
    ${addr}=    Evaluate    ${CSI_BASE} + ${offset}
    ${val}=     Execute Command    sysbus ReadDoubleWord ${addr}
    ${val}=     Strip String    ${val}
    RETURN    ${val}

Write CSI Register
    [Arguments]    ${offset}    ${value}
    ${addr}=    Evaluate    ${CSI_BASE} + ${offset}
    Execute Command    sysbus WriteDoubleWord ${addr} ${value}

*** Test Cases ***
CSI CR Reset Value
    Start STM32N6
    ${val}=    Read CSI Register    0x00
    Should Be Equal As Integers    ${val}    0

CSI IER Writable
    Start STM32N6
    Write CSI Register    0x08    0x01
    ${val}=    Read CSI Register    0x08
    Should Be Equal As Integers    ${val}    0x01

CSI SR Accessible
    Start STM32N6
    ${val}=    Read CSI Register    0x04
    Should Be True    int(${val}) >= 0

Boot Regression
    Start STM32N6
    Wait For NSH
