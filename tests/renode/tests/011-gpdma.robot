*** Settings ***
Resource        resources/stm32n6-common.robot

*** Variables ***
${GPDMA_BASE}   0x40020000

*** Keywords ***
Read GPDMA Register
    [Arguments]    ${offset}
    ${addr}=    Evaluate    ${GPDMA_BASE} + ${offset}
    ${val}=     Execute Command    sysbus ReadDoubleWord ${addr}
    ${val}=     Strip String    ${val}
    RETURN    ${val}

Write GPDMA Register
    [Arguments]    ${offset}    ${value}
    ${addr}=    Evaluate    ${GPDMA_BASE} + ${offset}
    Execute Command    sysbus WriteDoubleWord ${addr} ${value}

*** Test Cases ***
GPDMA SECCFGR Accessible
    Start STM32N6
    # SECCFGR @ 0x00 (global register)
    ${val}=    Read GPDMA Register    0x00
    Should Be True    int(${val}) >= 0

GPDMA Channel 0 CCR Accessible
    Start STM32N6
    # Channel 0: base=0x50, CCR=base+0x14=0x64
    ${val}=    Read GPDMA Register    0x64
    Should Be True    int(${val}) >= 0

GPDMA Channel 0 CSR Accessible
    Start STM32N6
    # Channel 0: base=0x50, CSR=base+0x10=0x60
    ${val}=    Read GPDMA Register    0x60
    Should Be True    int(${val}) >= 0

GPDMA Channel 0 CLBAR Writable
    Start STM32N6
    # Channel 0: base=0x50, CLBAR=base+0x00=0x50
    Write GPDMA Register    0x50    0x20000000
    ${val}=    Read GPDMA Register    0x50
    Should Be Equal As Integers    ${val}    0x20000000

GPDMA Channel 1 CLBAR Writable
    Start STM32N6
    # Channel 1: base=0x50+0x80=0xD0, CLBAR=0xD0
    Write GPDMA Register    0xD0    0x34000000
    ${val}=    Read GPDMA Register    0xD0
    Should Be Equal As Integers    ${val}    0x34000000

Boot Regression
    Start STM32N6
    Wait For NSH
