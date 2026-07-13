*** Settings ***
Resource        resources/stm32n6-common.robot

*** Variables ***
${SAI_BASE}     0x42005800

*** Keywords ***
Read SAI Register
    [Arguments]    ${offset}
    ${addr}=    Evaluate    ${SAI_BASE} + ${offset}
    ${val}=     Execute Command    sysbus ReadDoubleWord ${addr}
    ${val}=     Strip String    ${val}
    RETURN    ${val}

Write SAI Register
    [Arguments]    ${offset}    ${value}
    ${addr}=    Evaluate    ${SAI_BASE} + ${offset}
    Execute Command    sysbus WriteDoubleWord ${addr} ${value}

*** Test Cases ***
SAI1 CR1 Reset Value
    Start STM32N6
    ${val}=    Read SAI Register    0x04
    Should Be Equal As Integers    ${val}    0

SAI1 CR1 Writable
    Start STM32N6
    Write SAI Register    0x04    0x01000000
    ${val}=    Read SAI Register    0x04
    Should Be Equal As Integers    ${val}    0x01000000

SAI1 SR Accessible
    Start STM32N6
    ${val}=    Read SAI Register    0x14
    Should Be True    int(${val}) >= 0

Boot Regression
    Start STM32N6
    Wait For NSH
