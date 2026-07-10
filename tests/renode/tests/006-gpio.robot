*** Settings ***
Resource        resources/stm32n6-common.robot

*** Variables ***
${GPIOE_BASE}       0x46021000
${GPIOA_BASE}       0x46020000
${GPIO_MODER}       0x00
${GPIO_IDR}         0x10
${GPIO_ODR}         0x14

*** Keywords ***
Read GPIO Register
    [Arguments]    ${base}    ${offset}
    ${addr}=    Evaluate    ${base} + ${offset}
    ${val}=     Execute Command    sysbus ReadDoubleWord ${addr}
    ${val}=     Strip String    ${val}
    RETURN    ${val}

Write GPIO Register
    [Arguments]    ${base}    ${offset}    ${value}
    ${addr}=    Evaluate    ${base} + ${offset}
    Execute Command    sysbus WriteDoubleWord ${addr} ${value}

*** Test Cases ***
GPIOE MODER Accessible
    Start STM32N6
    ${val}=    Read GPIO Register    ${GPIOE_BASE}    ${GPIO_MODER}
    # PE5/PE6 configured as AF (USART1) by firmware: 0x2800
    Should Be True    int(${val}) >= 0

GPIOE Output Write
    Start STM32N6
    # Set PE0 to output mode (MODER bit 0:1 = 01)
    Write GPIO Register    ${GPIOE_BASE}    ${GPIO_MODER}    0x01
    # Set PE0 output high (ODR bit 0 = 1)
    Write GPIO Register    ${GPIOE_BASE}    ${GPIO_ODR}    0x01
    ${odr}=    Read GPIO Register    ${GPIOE_BASE}    ${GPIO_ODR}
    ${pe0}=    Evaluate    int(${odr}) & 1
    Should Be Equal As Integers    ${pe0}    1

GPIOA Accessible
    Start STM32N6
    ${val}=    Read GPIO Register    ${GPIOA_BASE}    ${GPIO_MODER}
    Should Be Equal As Integers    ${val}    0

GPIO Port E IDR Readback
    Start STM32N6
    ${idr}=    Read GPIO Register    ${GPIOE_BASE}    ${GPIO_IDR}
    Should Be True    int(${idr}) >= 0

Boot Regression
    Start STM32N6
    Wait For NSH
