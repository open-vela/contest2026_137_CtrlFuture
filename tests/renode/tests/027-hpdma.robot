*** Settings ***
Resource        resources/stm32n6-common.robot
Force Tags      hpdma

*** Variables ***
${HPDMA_BASE}   0x48020000

*** Keywords ***
Read HPDMA Register
    [Arguments]    ${offset}
    ${addr}=    Evaluate    ${HPDMA_BASE} + ${offset}
    ${val}=     Execute Command    sysbus ReadDoubleWord ${addr}
    ${val}=     Strip String    ${val}
    RETURN    ${val}

Write HPDMA Register
    [Arguments]    ${offset}    ${value}
    ${addr}=    Evaluate    ${HPDMA_BASE} + ${offset}
    Execute Command    sysbus WriteDoubleWord ${addr} ${value}

*** Test Cases ***
HPDMA SECCFGR Reset Value
    [Tags]    L1-register
    Start STM32N6
    # SECCFGR @ 0x00
    ${val}=    Read HPDMA Register    0x00
    Should Be Equal As Integers    ${val}    0

HPDMA SECCFGR Writable
    [Tags]    L1-register
    Start STM32N6
    # SECCFGR @ 0x00
    Write HPDMA Register    0x00    0x01
    ${val}=    Read HPDMA Register    0x00
    Should Be Equal As Integers    ${val}    0x01

HPDMA Channel 0 CLBAR Accessible
    [Tags]    L1-register
    Start STM32N6
    # Channel 0: CLBAR @ 0x50 (CMSIS DMA_Channel_TypeDef +0x00)
    ${val}=    Read HPDMA Register    0x50
    Should Be True    int(${val}) >= 0

Boot Regression
    [Tags]    boot-regression
    Start STM32N6
    Wait For NSH

HPDMA Base Is Not GPDMA
    [Tags]    L1-register
    Create STM32N6 Machine
    # Writing HPDMA SECCFGR must not change GPDMA SECCFGR
    Execute Command    sysbus WriteDoubleWord 0x48020000 0xA5A5A5A5
    ${g}=    Execute Command    sysbus ReadDoubleWord 0x40021000
    ${g}=    Strip String    ${g}
    Should Not Be Equal As Integers    ${g}    0xA5A5A5A5
    ${h}=    Execute Command    sysbus ReadDoubleWord 0x48020000
    ${h}=    Strip String    ${h}
    Should Be Equal As Integers    ${h}    0xA5A5A5A5

HPDMA CH0 TCIE Writable
    [Tags]    L2-state
    Create STM32N6 Machine
    # Set BNDT first (CBR1 @ 0x98, CMSIS DMA_Channel_TypeDef +0x48)
    Write HPDMA Register    0x98    0x1
    # Write TCIE=bit8 without EN so transfer doesn't start
    # (CCR @ 0x64, CMSIS DMA_Channel_TypeDef +0x14)
    Write HPDMA Register    0x64    0x100
    ${val}=    Read HPDMA Register    0x64
    ${val}=    Convert To Integer    ${val}
    Should Be True    (${val} & 0x100) == 0x100

HPDMA CH0 TCIE Sets IRQ
    [Tags]    L2-state
    Create STM32N6 Machine
    # Write source data
    ${src}=    Evaluate    0x34001000
    ${dst}=    Evaluate    0x34002000
    Execute Command    sysbus WriteDoubleWord ${src} 0x11223344
    # CSAR @ 0x9C (CMSIS DMA_Channel_TypeDef +0x4C)
    Write HPDMA Register    0x9C    ${src}
    # CDAR @ 0xA0 (CMSIS DMA_Channel_TypeDef +0x50)
    Write HPDMA Register    0xA0    ${dst}
    # BNDT @ 0x98 = 1 (byte count)
    Write HPDMA Register    0x98    0x1
    # CTR2 @ 0x94: SDW=word(2), DINC=1, DDW=word(2)
    # (CMSIS DMA_Channel_TypeDef +0x44)
    Write HPDMA Register    0x94    0x00080008
    # CCR @ 0x64: EN=bit0, TCIE=bit8 = 0x101
    Write HPDMA Register    0x64    0x101
    ${irq}=    Execute Command    sysbus.hpdma1 IRQ IsSet
    Should Contain    ${irq}    True

HPDMA CH0 Mem2Mem Copy
    [Tags]    L3-functional
    Create STM32N6 Machine
    ${src}=    Evaluate    0x34001000
    ${dst}=    Evaluate    0x34002000
    Execute Command    sysbus WriteDoubleWord ${src} 0xDEADBEEF
    Write HPDMA Register    0x9C    ${src}
    Write HPDMA Register    0xA0    ${dst}
    Write HPDMA Register    0x98    0x4
    Write HPDMA Register    0x94    0x00080008
    Write HPDMA Register    0x64    0x101
    ${out}=    Execute Command    sysbus ReadDoubleWord ${dst}
    ${out}=    Strip String    ${out}
    ${out}=    Convert To Integer    ${out}
    Should Be Equal As Integers    ${out}    0xDEADBEEF
