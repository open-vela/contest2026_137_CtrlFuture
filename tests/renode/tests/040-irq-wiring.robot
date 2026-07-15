# Copyright (c) 2026 CtrlFuture
#
# SPDX-License-Identifier: Apache-2.0

*** Settings ***
Resource        resources/stm32n6-common.robot
Force Tags      irq-wiring

*** Variables ***
${SPI1_BASE}    0x42003000
${I2C1_BASE}    0x40005400
${GPDMA_BASE}   0x40021000
${EXTI_BASE}    0x46025000
${SDMMC_BASE}   0x48027000
${RNG_BASE}     0x44020000
${SRAM_BASE}    0x34005000

*** Keywords ***
Write Raw DoubleWord
    [Arguments]    ${address}    ${value}
    Execute Command    sysbus WriteDoubleWord ${address} ${value}

*** Test Cases ***
RNG IRQ Still Wired
    [Tags]    L2-state
    Create STM32N6 Machine
    # RNGEN=bit2, IE=bit3 → CR = 0x4 | 0x8 = 0xC
    Write Raw DoubleWord    ${RNG_BASE}    0xC
    ${irq}=    Execute Command    sysbus.rng IRQ IsSet
    Should Contain    ${irq}    True

SPI1 IRQ Wired
    [Tags]    L2-state
    Create STM32N6 Machine
    # SPE + SSI
    Write Raw DoubleWord    ${SPI1_BASE}    0x1001
    # IER.EOTIE bit3 = 0x8
    ${ierAddr}=    Evaluate    ${SPI1_BASE} + 0x10
    Write Raw DoubleWord    ${ierAddr}    0x8
    # TXDR triggers EOT
    ${txdrAddr}=    Evaluate    ${SPI1_BASE} + 0x20
    Write Raw DoubleWord    ${txdrAddr}    0xA5
    ${irq}=    Execute Command    sysbus.spi1 IRQ IsSet
    Should Contain    ${irq}    True

GPDMA CH0 IRQ Wired
    [Tags]    L2-state
    Create STM32N6 Machine
    # Write source data
    Write Raw DoubleWord    ${SRAM_BASE}    0x42
    # Program GPDMA CH0
    ${ch0_csar}=    Evaluate    ${GPDMA_BASE} + 0x9C
    ${ch0_cdar}=    Evaluate    ${GPDMA_BASE} + 0xA0
    ${ch0_bndt}=    Evaluate    ${GPDMA_BASE} + 0x98
    ${ch0_ctr1}=    Evaluate    ${GPDMA_BASE} + 0x90
    ${ch0_ccr}=     Evaluate    ${GPDMA_BASE} + 0x64
    ${dstAddr}=     Evaluate    ${SRAM_BASE} + 4
    Write Raw DoubleWord    ${ch0_csar}    ${SRAM_BASE}
    Write Raw DoubleWord    ${ch0_cdar}    ${dstAddr}
    Write Raw DoubleWord    ${ch0_bndt}    0x1
    Write Raw DoubleWord    ${ch0_ctr1}    0x00080008
    # CCR: EN=bit0, TCIE=bit8 = 0x101
    Write Raw DoubleWord    ${ch0_ccr}    0x101
    ${irq}=    Execute Command    sysbus.gpdma1 IRQ IsSet
    Should Contain    ${irq}    True

EXTI0 IRQ Pending
    [Tags]    L2-state
    Create STM32N6 Machine
    ${imrAddr}=    Evaluate    ${EXTI_BASE} + 0x80
    ${rtsrAddr}=   Evaluate    ${EXTI_BASE} + 0x00
    ${swierAddr}=  Evaluate    ${EXTI_BASE} + 0x08
    ${rprAddr}=    Evaluate    ${EXTI_BASE} + 0x0C
    # IMR1 bit0 = 1
    Write Raw DoubleWord    ${imrAddr}    0x1
    # RTSR1 bit0 = 1
    Write Raw DoubleWord    ${rtsrAddr}    0x1
    # SWIER1 bit0 = 1 → sets RPR1 bit0
    Write Raw DoubleWord    ${swierAddr}    0x1
    # Verify RPR1 has bit0 set
    ${rpr}=    Execute Command    sysbus ReadDoubleWord ${rprAddr}
    ${rpr}=    Strip String    ${rpr}
    ${rpr}=    Convert To Integer    ${rpr}
    Should Be True    (${rpr} & 0x1) == 1

SDMMC IRQ Wired
    [Tags]    L2-state
    Create STM32N6 Machine
    ${pwrAddr}=     Evaluate    ${SDMMC_BASE} + 0x00
    ${clkcrAddr}=   Evaluate    ${SDMMC_BASE} + 0x04
    ${maskAddr}=    Evaluate    ${SDMMC_BASE} + 0x3C
    ${cmdAddr}=     Evaluate    ${SDMMC_BASE} + 0x0C
    # POWER ON
    Write Raw DoubleWord    ${pwrAddr}    0x3
    # CLKCR
    Write Raw DoubleWord    ${clkcrAddr}    0x100FA
    # MASK: enable CMDSENT bit7 = 0x80
    Write Raw DoubleWord    ${maskAddr}    0x80
    # CMD0: no-resp, CPSMEN
    Write Raw DoubleWord    ${cmdAddr}    0x1000
    ${irq}=    Execute Command    sysbus.sdmmc1 IRQ IsSet
    Should Contain    ${irq}    True

I2C1 IRQ Wired
    [Tags]    L2-state
    Create STM32N6 Machine
    ${cr1Addr}=     Evaluate    ${I2C1_BASE} + 0x00
    ${cr2Addr}=     Evaluate    ${I2C1_BASE} + 0x04
    ${txdrAddr}=    Evaluate    ${I2C1_BASE} + 0x28
    # PE + TCIE (bit6) → CR1 = 0x1 | 0x40 = 0x41
    Write Raw DoubleWord    ${cr1Addr}    0x41
    # CR2: START+NBYTES=1+AUTOEND+SADD (write)
    Write Raw DoubleWord    ${cr2Addr}    0x020120A0
    # TXDR triggers transfer complete
    Write Raw DoubleWord    ${txdrAddr}    0xAA
    ${irq}=    Execute Command    sysbus.i2c1 IRQ IsSet
    Should Contain    ${irq}    True

Boot Regression
    [Tags]    boot-regression
    Start STM32N6
    Wait For NSH
