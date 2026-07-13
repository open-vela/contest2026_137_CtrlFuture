*** Settings ***
Resource        resources/stm32n6-common.robot

*** Variables ***
${DMA2D_BASE}   0x48021000

*** Keywords ***
Read DMA2D Register
    [Arguments]    ${offset}
    ${addr}=    Evaluate    ${DMA2D_BASE} + ${offset}
    ${val}=     Execute Command    sysbus ReadDoubleWord ${addr}
    ${val}=     Strip String    ${val}
    RETURN    ${val}

Write DMA2D Register
    [Arguments]    ${offset}    ${value}
    ${addr}=    Evaluate    ${DMA2D_BASE} + ${offset}
    Execute Command    sysbus WriteDoubleWord ${addr} ${value}

*** Test Cases ***
DMA2D CR Reset Value
    Start STM32N6
    ${val}=    Read DMA2D Register    0x00
    Should Be Equal As Integers    ${val}    0

DMA2D FGMAR Writable
    Start STM32N6
    Write DMA2D Register    0x0C    0x20000000
    ${val}=    Read DMA2D Register    0x0C
    Should Be Equal As Integers    ${val}    0x20000000

DMA2D ISR Accessible
    Start STM32N6
    ${val}=    Read DMA2D Register    0x04
    Should Be True    int(${val}) >= 0

Boot Regression
    Start STM32N6
    Wait For NSH
