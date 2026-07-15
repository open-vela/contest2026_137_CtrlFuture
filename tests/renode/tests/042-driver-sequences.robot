# Copyright (c) 2026 CtrlFuture
#
# SPDX-License-Identifier: Apache-2.0

*** Settings ***
Resource        resources/stm32n6-common.robot
Force Tags      driver-sequence

*** Variables ***
${SPI1_BASE}    0x42003000
${I2C1_BASE}    0x40005400
${RNG_BASE}     0x44020000
${GPDMA_BASE}   0x40021000
${SRAM_BASE}    0x34004000

${CR1_OFFSET}   0x00
${IER_OFFSET}   0x10
${TXDR_OFFSET}  0x20
${RXDR_OFFSET}  0x30
${SR_OFFSET}    0x14
${IFCR_OFFSET}  0x18

*** Keywords ***
Write SPI1 Register
    [Arguments]    ${offset}    ${value}
    ${addr}=    Evaluate    ${SPI1_BASE} + ${offset}
    Execute Command    sysbus WriteDoubleWord ${addr} ${value}

Read SPI1 Register
    [Arguments]    ${offset}
    ${addr}=    Evaluate    ${SPI1_BASE} + ${offset}
    ${val}=     Execute Command    sysbus ReadDoubleWord ${addr}
    ${val}=     Strip String    ${val}
    RETURN    ${val}

Write Raw DoubleWord
    [Arguments]    ${address}    ${value}
    Execute Command    sysbus WriteDoubleWord ${address} ${value}

Read Raw DoubleWord
    [Arguments]    ${address}
    ${val}=     Execute Command    sysbus ReadDoubleWord ${address}
    ${val}=     Strip String    ${val}
    RETURN    ${val}

*** Test Cases ***
NuttX SPI Transfer Sequence
    [Tags]    L3-functional
    Create STM32N6 Machine
    # stm32n6_spi.c send: disable SPE → CFG1/CFG2 → SSI+SPE → CSTART → TXDR → wait TXC
    # 1) Disable
    Write SPI1 Register    ${CR1_OFFSET}    0x0
    # 2) Enable SPE+SSI
    Write SPI1 Register    ${CR1_OFFSET}    0x1001
    # 3) CSTART
    Write SPI1 Register    ${CR1_OFFSET}    0x1201
    # 4) TXDR triggers loopback
    Write SPI1 Register    ${TXDR_OFFSET}    0xDEADBEEF
    # 5) Wait TXC bit12
    ${sr}=    Read SPI1 Register    ${SR_OFFSET}
    ${sr}=    Convert To Integer    ${sr}
    Should Be True    (${sr} & 0x1000) == 0x1000
    # 6) Drain RXDR
    ${rx}=    Read SPI1 Register    ${RXDR_OFFSET}
    ${rx}=    Convert To Integer    ${rx}
    Should Be Equal As Integers    ${rx}    0xDEADBEEF

NuttX I2C Master TX Sequence
    [Tags]    L3-functional
    Create STM32N6 Machine
    # stm32n6_i2c.c: PE → TIMINGR → CR2 START → TXDR → STOPF → ICR
    ${timingAddr}=    Evaluate    ${I2C1_BASE} + 0x10
    ${cr2Addr}=       Evaluate    ${I2C1_BASE} + 0x04
    ${txdrAddr}=      Evaluate    ${I2C1_BASE} + 0x28
    ${icrAddr}=       Evaluate    ${I2C1_BASE} + 0x1C
    ${isrAddr}=       Evaluate    ${I2C1_BASE} + 0x18
    # PE enable
    Write Raw DoubleWord    ${I2C1_BASE}    0x1
    # TIMINGR
    Write Raw DoubleWord    ${timingAddr}    0x10909CEC
    # CR2: START+NBYTES+AUTOEND+SADD
    Write Raw DoubleWord    ${cr2Addr}    0x020120A0
    # TXDR
    Write Raw DoubleWord    ${txdrAddr}    0x5A
    # ISR: STOPF should be set, BUSY cleared
    ${isr}=    Read Raw DoubleWord    ${isrAddr}
    ${isr}=    Convert To Integer    ${isr}
    Should Be True    (${isr} & 0x20) == 0x20
    # Clear STOPF
    Write Raw DoubleWord    ${icrAddr}    0x20

NuttX RNG Enable Sequence
    [Tags]    L2-state
    Create STM32N6 Machine
    # CONDRST pulse → RNGEN → DRDY → read DR
    # CR @ 0x00: CONDRST=bit31, RNGEN=bit2
    # SR @ 0x04: DRDY=bit0
    ${crAddr}=      Evaluate    ${RNG_BASE}
    ${srAddr}=      Evaluate    ${RNG_BASE} + 0x04
    # CR: CONDRST + RNGEN
    Write Raw DoubleWord    ${crAddr}    0x80000004
    # CONDRST pulse: clear CONDRST, keep RNGEN
    Write Raw DoubleWord    ${crAddr}    0x00000004
    # DRDY is bit0 in SR
    ${sr}=    Read Raw DoubleWord    ${srAddr}
    ${sr}=    Convert To Integer    ${sr}
    Should Be True    (${sr} & 0x1) == 0x1

NuttX GPDMA Mem2Mem Sequence
    [Tags]    L3-functional
    Create STM32N6 Machine
    # CLBAR → CTR1 → CTR2 → CBR1 → CSAR → CDAR → CCR.EN → CSR.TCF
    ${ch0_csarAddr}=    Evaluate    ${GPDMA_BASE} + 0x9C
    ${ch0_cdarAddr}=    Evaluate    ${GPDMA_BASE} + 0xA0
    ${ch0_bndtAddr}=    Evaluate    ${GPDMA_BASE} + 0x98
    ${ch0_ctr1Addr}=    Evaluate    ${GPDMA_BASE} + 0x90
    ${ch0_ccrAddr}=     Evaluate    ${GPDMA_BASE} + 0x64
    ${ch0_csrAddr}=     Evaluate    ${GPDMA_BASE} + 0x60
    # Write source data
    Write Raw DoubleWord    ${SRAM_BASE}    0xCAFEBABE
    # Ch0: CSAR
    Write Raw DoubleWord    ${ch0_csarAddr}    ${SRAM_BASE}
    # Ch0: CDAR
    ${dstAddr}=    Evaluate    ${SRAM_BASE} + 0x100
    Write Raw DoubleWord    ${ch0_cdarAddr}    ${dstAddr}
    # Ch0: BNDT=4 (1 word)
    Write Raw DoubleWord    ${ch0_bndtAddr}    0x4
    # Ch0: CTR1 SDW=0 SINC=1 DDW=0 DINC=1 = 0x00080008
    Write Raw DoubleWord    ${ch0_ctr1Addr}    0x00080008
    # Ch0: CCR EN=1
    Write Raw DoubleWord    ${ch0_ccrAddr}    0x1
    # CSR TCF
    ${csr}=    Read Raw DoubleWord    ${ch0_csrAddr}
    ${csr}=    Convert To Integer    ${csr}
    Should Be True    (${csr} & 0x100) == 0x100
    # Data copied
    ${dst}=    Read Raw DoubleWord    ${dstAddr}
    ${dst}=    Convert To Integer    ${dst}
    Should Be Equal As Integers    ${dst}    0xCAFEBABE

Boot Regression
    [Tags]    boot-regression
    Start STM32N6
    Wait For NSH
