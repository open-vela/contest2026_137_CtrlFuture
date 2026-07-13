*** Settings ***
Resource        resources/stm32n6-common.robot

*** Variables ***
${NPU_BASE}     0x48050000

*** Keywords ***
Read NPU Register
    [Arguments]    ${offset}
    ${addr}=    Evaluate    ${NPU_BASE} + ${offset}
    ${val}=     Execute Command    sysbus ReadDoubleWord ${addr}
    ${val}=     Strip String    ${val}
    RETURN    ${val}

Write NPU Register
    [Arguments]    ${offset}    ${value}
    ${addr}=    Evaluate    ${NPU_BASE} + ${offset}
    Execute Command    sysbus WriteDoubleWord ${addr} ${value}

*** Test Cases ***
NPU CR Reset Value
    Start STM32N6
    ${val}=    Read NPU Register    0x00
    Should Be Equal As Integers    ${val}    0

NPU CFG Writable
    Start STM32N6
    Write NPU Register    0x10    0x01
    ${val}=    Read NPU Register    0x10
    Should Be Equal As Integers    ${val}    0x01

NPU SR Accessible
    Start STM32N6
    ${val}=    Read NPU Register    0x04
    Should Be True    int(${val}) >= 0

Boot Regression
    Start STM32N6
    Wait For NSH
