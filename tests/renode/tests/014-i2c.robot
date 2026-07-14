*** Settings ***
Resource        resources/stm32n6-common.robot
Force Tags      i2c

*** Variables ***
${I2C1_BASE}    0x40005400
${CR1_OFFSET}   0x00
${CR2_OFFSET}   0x04
${TIMINGR_OFFSET}    0x10
${ISR_OFFSET}   0x18
${ICR_OFFSET}   0x1C
${RXDR_OFFSET}  0x24
${TXDR_OFFSET}  0x28
# CR2: SADD | [RD_WRN] | START | NBYTES | AUTOEND
# addr 0x50 << 1 = 0xA0; START bit13; RD_WRN bit10; AUTOEND bit25
${CR2_TX_1B}    0x020120A0
${CR2_RX_1B}    0x020124A0
${CR2_TX_2B}    0x020220A0
${CR1_PE}       0x1
${CR1_PE_STOPIE}    0x21
${ICR_STOPCF}   0x20

*** Keywords ***
Read I2C Register
    [Arguments]    ${offset}
    ${addr}=    Evaluate    ${I2C1_BASE} + ${offset}
    ${val}=     Execute Command    sysbus ReadDoubleWord ${addr}
    ${val}=     Strip String    ${val}
    RETURN    ${val}

Write I2C Register
    [Arguments]    ${offset}    ${value}
    ${addr}=    Evaluate    ${I2C1_BASE} + ${offset}
    Execute Command    sysbus WriteDoubleWord ${addr} ${value}

*** Test Cases ***
I2C1 CR1 Reset Value
    [Tags]    L1-register
    Start STM32N6
    ${val}=    Read I2C Register    ${CR1_OFFSET}
    Should Be Equal As Integers    ${val}    0

I2C1 TIMINGR Writable
    [Tags]    L1-register
    Start STM32N6
    Write I2C Register    ${TIMINGR_OFFSET}    0x10909CEC
    ${val}=    Read I2C Register    ${TIMINGR_OFFSET}
    Should Be Equal As Integers    ${val}    0x10909CEC

I2C1 ISR TXE Set
    [Tags]    L1-register
    Start STM32N6
    ${val}=    Read I2C Register    ${ISR_OFFSET}
    # TXE (bit 0) should be set at reset
    ${txe}=    Evaluate    ${val} & 1
    Should Be Equal As Integers    ${txe}    1

I2C1 ISR BUSY Clear
    [Tags]    L1-register
    Start STM32N6
    ${val}=    Read I2C Register    ${ISR_OFFSET}
    # BUSY (bit 15, CMSIS I2C_ISR_BUSY) should be clear
    ${busy}=    Evaluate    (${val} >> 15) & 1
    Should Be Equal As Integers    ${busy}    0

PE Enable Sets PE
    [Tags]    L2-state
    Start STM32N6
    Write I2C Register    ${CR1_OFFSET}    ${CR1_PE}
    ${val}=    Read I2C Register    ${CR1_OFFSET}
    ${val}=    Convert To Integer    ${val}
    Should Be True    (${val} & 0x1) == 0x1

START Write Sets TXIS And BUSY
    [Tags]    L2-state
    Start STM32N6
    Write I2C Register    ${CR1_OFFSET}    ${CR1_PE}
    # CR2: SADD=0xA0, NBYTES=1, AUTOEND, START (write dir)
    Write I2C Register    ${CR2_OFFSET}    ${CR2_TX_1B}
    ${isr}=    Read I2C Register    ${ISR_OFFSET}
    ${isr}=    Convert To Integer    ${isr}
    # TXIS bit 1
    Should Be True    (${isr} & 0x2) == 0x2
    # BUSY bit 15
    Should Be True    (${isr} & 0x8000) == 0x8000
    # START auto-cleared in CR2
    ${cr2}=    Read I2C Register    ${CR2_OFFSET}
    ${cr2}=    Convert To Integer    ${cr2}
    Should Be True    (${cr2} & 0x2000) == 0x0

Master TX One Byte Completes With STOPF
    [Tags]    L3-functional
    Start STM32N6
    Write I2C Register    ${CR1_OFFSET}    ${CR1_PE}
    Write I2C Register    ${CR2_OFFSET}    ${CR2_TX_1B}
    Write I2C Register    ${TXDR_OFFSET}    0x5A
    ${isr}=    Read I2C Register    ${ISR_OFFSET}
    ${isr}=    Convert To Integer    ${isr}
    # STOPF bit 5
    Should Be True    (${isr} & 0x20) == 0x20
    # BUSY clear after AUTOEND
    Should Be True    (${isr} & 0x8000) == 0x0
    # Clear STOPF via ICR.STOPCF
    Write I2C Register    ${ICR_OFFSET}    ${ICR_STOPCF}
    ${isr2}=    Read I2C Register    ${ISR_OFFSET}
    ${isr2}=    Convert To Integer    ${isr2}
    Should Be True    (${isr2} & 0x20) == 0x0

Master RX One Byte Sets RXNE
    [Tags]    L3-functional
    Start STM32N6
    Write I2C Register    ${CR1_OFFSET}    ${CR1_PE}
    # CR2: SADD=0xA0, RD_WRN, NBYTES=1, AUTOEND, START
    Write I2C Register    ${CR2_OFFSET}    ${CR2_RX_1B}
    ${isr}=    Read I2C Register    ${ISR_OFFSET}
    ${isr}=    Convert To Integer    ${isr}
    # RXNE bit 2
    Should Be True    (${isr} & 0x4) == 0x4
    # BUSY set during transfer
    Should Be True    (${isr} & 0x8000) == 0x8000
    ${rx}=    Read I2C Register    ${RXDR_OFFSET}
    ${rx}=    Convert To Integer    ${rx}
    # Canned pattern when no prior TX: 0xA5
    Should Be Equal As Integers    ${rx}    0xA5
    ${isr2}=    Read I2C Register    ${ISR_OFFSET}
    ${isr2}=    Convert To Integer    ${isr2}
    # STOPF after last RX + AUTOEND
    Should Be True    (${isr2} & 0x20) == 0x20
    Should Be True    (${isr2} & 0x8000) == 0x0

Master TX Then RX Loopback
    [Tags]    L3-functional
    Start STM32N6
    Write I2C Register    ${CR1_OFFSET}    ${CR1_PE}
    # TX 0x3C into loopback buffer
    Write I2C Register    ${CR2_OFFSET}    ${CR2_TX_1B}
    Write I2C Register    ${TXDR_OFFSET}    0x3C
    Write I2C Register    ${ICR_OFFSET}    ${ICR_STOPCF}
    # RX should return the looped-back byte
    Write I2C Register    ${CR2_OFFSET}    ${CR2_RX_1B}
    ${rx}=    Read I2C Register    ${RXDR_OFFSET}
    ${rx}=    Convert To Integer    ${rx}
    Should Be Equal As Integers    ${rx}    0x3C

Master TX Two Bytes Completes
    [Tags]    L3-functional
    Start STM32N6
    Write I2C Register    ${CR1_OFFSET}    ${CR1_PE}
    Write I2C Register    ${CR2_OFFSET}    ${CR2_TX_2B}
    ${isr0}=    Read I2C Register    ${ISR_OFFSET}
    ${isr0}=    Convert To Integer    ${isr0}
    Should Be True    (${isr0} & 0x2) == 0x2
    Write I2C Register    ${TXDR_OFFSET}    0x11
    ${isr1}=    Read I2C Register    ${ISR_OFFSET}
    ${isr1}=    Convert To Integer    ${isr1}
    # Still TXIS, not yet STOPF
    Should Be True    (${isr1} & 0x2) == 0x2
    Should Be True    (${isr1} & 0x20) == 0x0
    Write I2C Register    ${TXDR_OFFSET}    0x22
    ${isr2}=    Read I2C Register    ${ISR_OFFSET}
    ${isr2}=    Convert To Integer    ${isr2}
    Should Be True    (${isr2} & 0x20) == 0x20
    Should Be True    (${isr2} & 0x8000) == 0x0

STOPIE Asserts IRQ On STOPF
    [Tags]    L3-functional
    Start STM32N6
    Write I2C Register    ${CR1_OFFSET}    ${CR1_PE_STOPIE}
    Write I2C Register    ${CR2_OFFSET}    ${CR2_TX_1B}
    Write I2C Register    ${TXDR_OFFSET}    0xAA
    ${irq}=    Execute Command    sysbus.i2c1 IRQ IsSet
    Should Contain    ${irq}    True
    Write I2C Register    ${ICR_OFFSET}    ${ICR_STOPCF}
    ${irq2}=    Execute Command    sysbus.i2c1 IRQ IsSet
    Should Contain    ${irq2}    False

Boot Regression
    [Tags]    boot-regression
    Start STM32N6
    Wait For NSH
