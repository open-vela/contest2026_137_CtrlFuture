*** Settings ***
Resource        resources/stm32n6-common.robot
Force Tags      spi

*** Variables ***
${SPI1_BASE}    0x42003000
${SPI6_BASE}    0x46001400
${CR1_OFFSET}   0x00
${CFG2_OFFSET}  0x0C
${SR_OFFSET}    0x14
${IFCR_OFFSET}  0x18
${TXDR_OFFSET}  0x20
${RXDR_OFFSET}  0x30

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

Read SPI6 Register
    [Arguments]    ${offset}
    ${addr}=    Evaluate    ${SPI6_BASE} + ${offset}
    ${val}=     Execute Command    sysbus ReadDoubleWord ${addr}
    ${val}=     Strip String    ${val}
    RETURN    ${val}

Write SPI6 Register
    [Arguments]    ${offset}    ${value}
    ${addr}=    Evaluate    ${SPI6_BASE} + ${offset}
    Execute Command    sysbus WriteDoubleWord ${addr} ${value}

*** Test Cases ***
SPI1 CR1 Accessible
    [Tags]    L1-register
    Start STM32N6
    ${val}=    Read SPI1 Register    ${CR1_OFFSET}
    ${val}=    Convert To Integer    ${val}
    Should Be True    ${val} >= 0

SPI1 Enable Sets SPE
    [Tags]    L2-state
    Start STM32N6
    Write SPI1 Register    ${CR1_OFFSET}    0x1
    ${val}=    Read SPI1 Register    ${CR1_OFFSET}
    ${val}=    Convert To Integer    ${val}
    Should Be True    (${val} & 0x1) == 0x1

SPI1 Enable Asserts TXP
    [Tags]    L2-state
    Start STM32N6
    Write SPI1 Register    ${CR1_OFFSET}    0x1
    ${sr}=    Read SPI1 Register    ${SR_OFFSET}
    ${sr}=    Convert To Integer    ${sr}
    # TXP bit 1
    Should Be True    (${sr} & 0x2) == 0x2

SPI1 Loopback TXDR To RXDR
    [Tags]    L3-functional
    Start STM32N6
    Write SPI1 Register    ${CR1_OFFSET}    0x1
    Write SPI1 Register    ${TXDR_OFFSET}    0xA5
    ${sr}=    Read SPI1 Register    ${SR_OFFSET}
    ${sr}=    Convert To Integer    ${sr}
    # RXP bit 0
    Should Be True    (${sr} & 0x1) == 0x1
    ${rx}=    Read SPI1 Register    ${RXDR_OFFSET}
    ${rx}=    Convert To Integer    ${rx}
    Should Be Equal As Integers    ${rx}    0xA5

SPI1 TXC After Transfer
    [Tags]    L3-functional
    Start STM32N6
    # Match NuttX path: SPE|SSI then CSTART then TXDR
    Write SPI1 Register    ${CR1_OFFSET}    0x1001
    Write SPI1 Register    ${CR1_OFFSET}    0x1201
    Write SPI1 Register    ${TXDR_OFFSET}    0x5A
    ${sr}=    Read SPI1 Register    ${SR_OFFSET}
    ${sr}=    Convert To Integer    ${sr}
    # TXC bit 12
    Should Be True    (${sr} & 0x1000) == 0x1000
    ${rx}=    Read SPI1 Register    ${RXDR_OFFSET}
    ${rx}=    Convert To Integer    ${rx}
    Should Be Equal As Integers    ${rx}    0x5A

SPI1 Multi-Byte Loopback
    [Tags]    L3-functional
    Start STM32N6
    Write SPI1 Register    ${CR1_OFFSET}    0x1
    Write SPI1 Register    ${TXDR_OFFSET}    0x11
    Write SPI1 Register    ${TXDR_OFFSET}    0x22
    ${r0}=    Read SPI1 Register    ${RXDR_OFFSET}
    ${r0}=    Convert To Integer    ${r0}
    ${r1}=    Read SPI1 Register    ${RXDR_OFFSET}
    ${r1}=    Convert To Integer    ${r1}
    Should Be Equal As Integers    ${r0}    0x11
    Should Be Equal As Integers    ${r1}    0x22

SPI1 Disable Clears RXP
    [Tags]    L2-state
    Start STM32N6
    Write SPI1 Register    ${CR1_OFFSET}    0x1
    Write SPI1 Register    ${TXDR_OFFSET}    0xA5
    Write SPI1 Register    ${CR1_OFFSET}    0x0
    ${sr}=    Read SPI1 Register    ${SR_OFFSET}
    ${sr}=    Convert To Integer    ${sr}
    # RXP and TXP should be clear when SPE=0
    Should Be True    (${sr} & 0x3) == 0x0

SPI6 Accessible After Tag Split
    [Tags]    L1-register
    Start STM32N6
    ${val}=    Read SPI6 Register    ${CR1_OFFSET}
    ${val}=    Convert To Integer    ${val}
    Should Be True    ${val} >= 0
    Write SPI6 Register    ${CR1_OFFSET}    0x1
    ${sr}=    Read SPI6 Register    ${SR_OFFSET}
    ${sr}=    Convert To Integer    ${sr}
    Should Be True    (${sr} & 0x2) == 0x2

Boot Regression
    [Tags]    boot-regression
    Start STM32N6
    Wait For NSH
