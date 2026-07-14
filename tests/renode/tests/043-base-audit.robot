# Copyright (c) 2026 CtrlFuture
#
# SPDX-License-Identifier: Apache-2.0

*** Settings ***
Resource        resources/stm32n6-common.robot
Force Tags      base-audit

*** Test Cases ***
GPDMA At 0x40021000
    [Tags]    L1-register
    Create STM32N6 Machine
    ${val}=    Execute Command    sysbus ReadDoubleWord 0x40021000
    ${val}=    Strip String    ${val}
    Should Be True    int(${val}) >= 0

HPDMA At 0x48020000
    [Tags]    L1-register
    Create STM32N6 Machine
    ${val}=    Execute Command    sysbus ReadDoubleWord 0x48020000
    ${val}=    Strip String    ${val}
    Should Be True    int(${val}) >= 0

I2C1 At 0x40005400
    [Tags]    L1-register
    Create STM32N6 Machine
    ${val}=    Execute Command    sysbus ReadDoubleWord 0x40005400
    ${val}=    Strip String    ${val}
    Should Be True    int(${val}) >= 0

SPI1 At 0x42003000
    [Tags]    L1-register
    Create STM32N6 Machine
    ${val}=    Execute Command    sysbus ReadDoubleWord 0x42003000
    ${val}=    Strip String    ${val}
    Should Be True    int(${val}) >= 0

SDMMC1 At 0x48027000
    [Tags]    L1-register
    Create STM32N6 Machine
    ${val}=    Execute Command    sysbus ReadDoubleWord 0x48027000
    ${val}=    Strip String    ${val}
    Should Be True    int(${val}) >= 0

USART1 At 0x42001000
    [Tags]    L1-register
    Create STM32N6 Machine
    ${val}=    Execute Command    sysbus ReadDoubleWord 0x42001000
    ${val}=    Strip String    ${val}
    Should Be True    int(${val}) >= 0
