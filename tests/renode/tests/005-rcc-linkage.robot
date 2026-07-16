# Copyright (c) 2026 CtrlFuture
#
# SPDX-License-Identifier: Apache-2.0

*** Settings ***
Resource        resources/stm32n6-common.robot
Force Tags      rcc-linkage

*** Variables ***
${RCC_BASE}     0x46028000
${AHB1ENR}      0x0250
${AHB4ENR}      0x025C
${AHB5ENR}      0x0260
${APB1ENR1}     0x0264
${APB2ENR}      0x026C
${APB4ENR1}     0x0274

*** Keywords ***
Write RCC ENR
    [Arguments]    ${offset}    ${value}
    ${addr}=    Evaluate    ${RCC_BASE} + ${offset}
    Execute Command    sysbus WriteDoubleWord ${addr} ${value}

Read RCC ENR
    [Arguments]    ${offset}
    ${addr}=    Evaluate    ${RCC_BASE} + ${offset}
    ${val}=    Execute Command    sysbus ReadDoubleWord ${addr}
    ${val}=    Strip String    ${val}
    ${val}=    Convert To Integer    ${val}
    RETURN    ${val}

*** Test Cases ***
RCC AHB1ENR Enables GPDMA1 Clock
    [Tags]    L2-state
    Start STM32N6
    # GPDMA1EN = bit 0
    Write RCC ENR    ${AHB1ENR}    0x00000001
    ${val}=    Read RCC ENR    ${AHB1ENR}
    Should Be Equal As Integers    ${val}    0x00000001

RCC AHB4ENR Enables GPIOE Clock
    [Tags]    L2-state
    Start STM32N6
    # GPIOEEN = bit 4
    Write RCC ENR    ${AHB4ENR}    0x00000010
    ${val}=    Read RCC ENR    ${AHB4ENR}
    Should Be Equal As Integers    ${val}    0x00000010

RCC APB2ENR Enables USART1 Clock
    [Tags]    L2-state
    Start STM32N6
    # USART1EN = bit 4
    Write RCC ENR    ${APB2ENR}    0x00000010
    ${val}=    Read RCC ENR    ${APB2ENR}
    Should Be Equal As Integers    ${val}    0x00000010

RCC AHB5ENR Enables HPDMA Clock
    [Tags]    L2-state
    Start STM32N6
    # DMA2DEN = bit 0
    Write RCC ENR    ${AHB5ENR}    0x00000001
    ${val}=    Read RCC ENR    ${AHB5ENR}
    Should Be Equal As Integers    ${val}    0x00000001

RCC APB1ENR1 Enables USART2 Clock
    [Tags]    L2-state
    Start STM32N6
    # USART2EN = bit 17
    Write RCC ENR    ${APB1ENR1}    0x00020000
    ${val}=    Read RCC ENR    ${APB1ENR1}
    Should Be Equal As Integers    ${val}    0x00020000

RCC APB4ENR1 Enables RTC Clock
    [Tags]    L2-state
    Start STM32N6
    # RTCEN = bit 16 (CMSIS RCC_APB4ENR1_RTCEN; there is no EXTIEN bit
    # on STM32N6 -- EXTI has no software clock gate in this RCC map)
    Write RCC ENR    ${APB4ENR1}    0x00010000
    ${val}=    Read RCC ENR    ${APB4ENR1}
    Should Be Equal As Integers    ${val}    0x00010000
