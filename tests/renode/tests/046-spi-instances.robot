*** Settings ***
Resource        resources/stm32n6-common.robot
Force Tags      spi-instances

*** Variables ***
${SPI3_BASE}    0x40003C00
${SPI4_BASE}    0x42003400
${SPI5_BASE}    0x42005000
${CR1_OFFSET}   0x00
${SR_OFFSET}    0x14
${IFCR_OFFSET}  0x18
${IER_OFFSET}   0x10
${TXDR_OFFSET}  0x20
${RXDR_OFFSET}  0x30

*** Keywords ***
Read SPI Register
    [Arguments]    ${base}    ${offset}
    ${addr}=    Evaluate    ${base} + ${offset}
    ${val}=     Execute Command    sysbus ReadDoubleWord ${addr}
    ${val}=     Strip String    ${val}
    RETURN    ${val}

Write SPI Register
    [Arguments]    ${base}    ${offset}    ${value}
    ${addr}=    Evaluate    ${base} + ${offset}
    Execute Command    sysbus WriteDoubleWord ${addr} ${value}

*** Test Cases ***
SPI3 CR1 Accessible
    [Tags]    L1-register
    Start STM32N6
    ${val}=    Read SPI Register    ${SPI3_BASE}    ${CR1_OFFSET}
    ${val}=    Convert To Integer    ${val}
    Should Be True    ${val} >= 0

SPI4 CR1 Accessible
    [Tags]    L1-register
    Start STM32N6
    ${val}=    Read SPI Register    ${SPI4_BASE}    ${CR1_OFFSET}
    ${val}=    Convert To Integer    ${val}
    Should Be True    ${val} >= 0

SPI5 CR1 Accessible
    [Tags]    L1-register
    Start STM32N6
    ${val}=    Read SPI Register    ${SPI5_BASE}    ${CR1_OFFSET}
    ${val}=    Convert To Integer    ${val}
    Should Be True    ${val} >= 0

SPI3 Enable Asserts TXP
    [Tags]    L2-state
    Create STM32N6 Machine
    Write SPI Register    ${SPI3_BASE}    ${CR1_OFFSET}    0x1
    ${sr}=    Read SPI Register    ${SPI3_BASE}    ${SR_OFFSET}
    ${sr}=    Convert To Integer    ${sr}
    # TXP bit 1
    Should Be True    (${sr} & 0x2) == 0x2

SPI4 Enable Asserts TXP
    [Tags]    L2-state
    Create STM32N6 Machine
    Write SPI Register    ${SPI4_BASE}    ${CR1_OFFSET}    0x1
    ${sr}=    Read SPI Register    ${SPI4_BASE}    ${SR_OFFSET}
    ${sr}=    Convert To Integer    ${sr}
    Should Be True    (${sr} & 0x2) == 0x2

SPI5 Enable Asserts TXP
    [Tags]    L2-state
    Create STM32N6 Machine
    Write SPI Register    ${SPI5_BASE}    ${CR1_OFFSET}    0x1
    ${sr}=    Read SPI Register    ${SPI5_BASE}    ${SR_OFFSET}
    ${sr}=    Convert To Integer    ${sr}
    Should Be True    (${sr} & 0x2) == 0x2

SPI3 Disable Clears RXP
    [Tags]    L2-state
    Create STM32N6 Machine
    Write SPI Register    ${SPI3_BASE}    ${CR1_OFFSET}    0x1
    Write SPI Register    ${SPI3_BASE}    ${TXDR_OFFSET}    0xA5
    Write SPI Register    ${SPI3_BASE}    ${CR1_OFFSET}    0x0
    ${sr}=    Read SPI Register    ${SPI3_BASE}    ${SR_OFFSET}
    ${sr}=    Convert To Integer    ${sr}
    Should Be True    (${sr} & 0x3) == 0x0

SPI3 Loopback TXDR To RXDR
    [Tags]    L3-functional
    Create STM32N6 Machine
    Write SPI Register    ${SPI3_BASE}    ${CR1_OFFSET}    0x1
    Write SPI Register    ${SPI3_BASE}    ${TXDR_OFFSET}    0xA5
    ${sr}=    Read SPI Register    ${SPI3_BASE}    ${SR_OFFSET}
    ${sr}=    Convert To Integer    ${sr}
    # RXP bit 0
    Should Be True    (${sr} & 0x1) == 0x1
    ${rx}=    Read SPI Register    ${SPI3_BASE}    ${RXDR_OFFSET}
    ${rx}=    Convert To Integer    ${rx}
    Should Be Equal As Integers    ${rx}    0xA5

SPI4 Loopback TXDR To RXDR
    [Tags]    L3-functional
    Create STM32N6 Machine
    Write SPI Register    ${SPI4_BASE}    ${CR1_OFFSET}    0x1
    Write SPI Register    ${SPI4_BASE}    ${TXDR_OFFSET}    0x5A
    ${sr}=    Read SPI Register    ${SPI4_BASE}    ${SR_OFFSET}
    ${sr}=    Convert To Integer    ${sr}
    Should Be True    (${sr} & 0x1) == 0x1
    ${rx}=    Read SPI Register    ${SPI4_BASE}    ${RXDR_OFFSET}
    ${rx}=    Convert To Integer    ${rx}
    Should Be Equal As Integers    ${rx}    0x5A

SPI5 Loopback TXDR To RXDR
    [Tags]    L3-functional
    Create STM32N6 Machine
    Write SPI Register    ${SPI5_BASE}    ${CR1_OFFSET}    0x1
    Write SPI Register    ${SPI5_BASE}    ${TXDR_OFFSET}    0x3C
    ${sr}=    Read SPI Register    ${SPI5_BASE}    ${SR_OFFSET}
    ${sr}=    Convert To Integer    ${sr}
    Should Be True    (${sr} & 0x1) == 0x1
    ${rx}=    Read SPI Register    ${SPI5_BASE}    ${RXDR_OFFSET}
    ${rx}=    Convert To Integer    ${rx}
    Should Be Equal As Integers    ${rx}    0x3C

SPI3 Multi-Byte Loopback
    [Tags]    L3-functional
    Create STM32N6 Machine
    Write SPI Register    ${SPI3_BASE}    ${CR1_OFFSET}    0x1
    Write SPI Register    ${SPI3_BASE}    ${TXDR_OFFSET}    0x11
    Write SPI Register    ${SPI3_BASE}    ${TXDR_OFFSET}    0x22
    ${r0}=    Read SPI Register    ${SPI3_BASE}    ${RXDR_OFFSET}
    ${r0}=    Convert To Integer    ${r0}
    ${r1}=    Read SPI Register    ${SPI3_BASE}    ${RXDR_OFFSET}
    ${r1}=    Convert To Integer    ${r1}
    Should Be Equal As Integers    ${r0}    0x11
    Should Be Equal As Integers    ${r1}    0x22

SPI4 EOT Raises IRQ When EOTIE
    [Tags]    L2-state
    Create STM32N6 Machine
    # SPE + SSI (0x1001)
    Write SPI Register    ${SPI4_BASE}    ${CR1_OFFSET}    0x1001
    Write SPI Register    ${SPI4_BASE}    ${IFCR_OFFSET}    0
    # IER.EOTIE bit3 = 0x8
    Write SPI Register    ${SPI4_BASE}    ${IER_OFFSET}    0x8
    Write SPI Register    ${SPI4_BASE}    ${TXDR_OFFSET}    0xA5
    ${irq}=    Execute Command    sysbus.spi4 IRQ IsSet
    Should Contain    ${irq}    True

Boot Regression
    [Tags]    boot-regression
    Start STM32N6
    Wait For NSH
