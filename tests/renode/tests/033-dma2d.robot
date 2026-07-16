*** Settings ***
Resource        resources/stm32n6-common.robot
Force Tags      dma2d

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
    [Tags]    L1-register
    Start STM32N6
    ${val}=    Read DMA2D Register    0x00
    Should Be Equal As Integers    ${val}    0

DMA2D FGMAR Writable
    [Tags]    L1-register
    Start STM32N6
    Write DMA2D Register    0x0C    0x20000000
    ${val}=    Read DMA2D Register    0x0C
    Should Be Equal As Integers    ${val}    0x20000000

DMA2D NLR Writable
    [Tags]    L1-register
    Start STM32N6
    # NLR (Number of Line Register) @ 0x40 is a real, writable
    # DMA2D register; write/read round-trip instead of the previous
    # "int(val) >= 0" check on ISR, which is true for any 32-bit
    # unsigned read and never verifies the register actually works.
    Write DMA2D Register    0x40    0x00000A0A
    ${val}=    Read DMA2D Register    0x40
    ${val}=    Convert To Integer    ${val}
    Should Be Equal As Integers    ${val}    0x00000A0A

Boot Regression
    [Tags]    boot-regression
    Start STM32N6
    Wait For NSH
