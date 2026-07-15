*** Settings ***
Resource        resources/stm32n6-common.robot
Force Tags      multi-peripheral

*** Variables ***
${SPI1_BASE}    0x42003000
${GPDMA_BASE}   0x40021000
${SRAM_BASE}    0x34003000
${CR1_OFFSET}   0x00
${TXDR_OFFSET}  0x20
${RXDR_OFFSET}  0x30
${SR_OFFSET}    0x14

*** Keywords ***
Read SPI1 Register
    [Arguments]    ${offset}
    ${addr}=    Evaluate    ${SPI1_BASE} + ${offset}
    ${val}=     Execute Command    sysbus ReadDoubleWord ${addr}
    ${val}=     Strip String    ${val}
    RETURN    ${val}

Write SPI1 Register
    [Arguments]    ${offset}    ${value}
    ${addr}=    Evaluate    ${SPI1_BASE} + ${offset}
    Execute Command    sysbus WriteDoubleWord ${addr} ${value}

Write Raw DoubleWord
    [Arguments]    ${address}    ${value}
    Execute Command    sysbus WriteDoubleWord ${address} ${value}

Read Raw DoubleWord
    [Arguments]    ${address}
    ${val}=     Execute Command    sysbus ReadDoubleWord ${address}
    ${val}=     Strip String    ${val}
    RETURN    ${val}

*** Test Cases ***
SPI Loopback Then GPDMA Copy Buffer
    [Tags]    L3-functional
    Create STM32N6 Machine
    # 1) SPI1 loopback 4 bytes into RX FIFO
    Write SPI1 Register    ${CR1_OFFSET}    0x1
    Write SPI1 Register    ${TXDR_OFFSET}    0x11223344
    ${sr}=    Read SPI1 Register    ${SR_OFFSET}
    ${sr}=    Convert To Integer    ${sr}
    Should Be True    (${sr} & 0x1) == 0x1
    ${rx}=    Read SPI1 Register    ${RXDR_OFFSET}
    ${rx}=    Convert To Integer    ${rx}
    Should Be Equal As Integers    ${rx}    0x11223344
    # Write SPI data to SRAM as GPDMA source
    Execute Command    sysbus WriteDoubleWord ${SRAM_BASE} ${rx}
    # 2) GPDMA CH0 copy SRAM src -> SRAM dst
    ${ch0_clbar}=    Evaluate    ${GPDMA_BASE} + 0x50
    ${ch0_ccid}=     Evaluate    ${GPDMA_BASE} + 0x54
    ${ch0_ctr1}=     Evaluate    ${GPDMA_BASE} + 0x90
    ${ch0_bndt}=     Evaluate    ${GPDMA_BASE} + 0x98
    ${ch0_csar}=     Evaluate    ${GPDMA_BASE} + 0x9C
    ${ch0_cdar}=     Evaluate    ${GPDMA_BASE} + 0xA0
    ${ch0_ccr}=      Evaluate    ${GPDMA_BASE} + 0x64
    ${ch0_csr}=      Evaluate    ${GPDMA_BASE} + 0x60
    # CLBAR = 0
    Write Raw DoubleWord    ${ch0_clbar}    0x0
    # CCIDCFGR = 0
    Write Raw DoubleWord    ${ch0_ccid}    0x0
    # CTR1: SDW=0 byte, SINC=1, DDW=0 byte, DINC=1 = 0x00080008
    Write Raw DoubleWord    ${ch0_ctr1}    0x00080008
    # CBR1 = BNDT = 4 bytes
    Write Raw DoubleWord    ${ch0_bndt}    0x4
    # CSAR = src SRAM
    Write Raw DoubleWord    ${ch0_csar}    ${SRAM_BASE}
    # CDAR = dst SRAM+0x100
    ${dstAddr}=    Evaluate    ${SRAM_BASE} + 0x100
    Write Raw DoubleWord    ${ch0_cdar}    ${dstAddr}
    # CCR EN = 0x1
    Write Raw DoubleWord    ${ch0_ccr}    0x1
    # Verify GPDMA completed: CSR TCF bit8
    ${csr}=    Read Raw DoubleWord    ${ch0_csr}
    ${csr}=    Convert To Integer    ${csr}
    Should Be True    (${csr} & 0x100) == 0x100
    # Verify data copied
    ${out}=    Read Raw DoubleWord    ${dstAddr}
    ${out}=    Convert To Integer    ${out}
    Should Be Equal As Integers    ${out}    0x11223344

Boot Regression
    [Tags]    boot-regression
    Start STM32N6
    Wait For NSH
