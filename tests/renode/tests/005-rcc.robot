*** Settings ***
Resource        resources/stm32n6-common.robot

*** Variables ***
${RCC_BASE}     0x46028000
${CR_OFFSET}    0x0000
${SR_OFFSET}    0x0004
${AHB4ENR}      0x025C
${APB2ENR}      0x026C

*** Keywords ***
Read RCC Register
    [Arguments]    ${offset}
    ${addr}=    Evaluate    ${RCC_BASE} + ${offset}
    ${val}=     Execute Command    sysbus ReadDoubleWord ${addr}
    ${val}=     Strip String    ${val}
    RETURN    ${val}

Write RCC Register
    [Arguments]    ${offset}    ${value}
    ${addr}=    Evaluate    ${RCC_BASE} + ${offset}
    Execute Command    sysbus WriteDoubleWord ${addr} ${value}

*** Test Cases ***
CR After Boot HSI Enabled
    Start STM32N6
    ${val}=    Read RCC Register    ${CR_OFFSET}
    # Firmware sets HSION (bit 3) during boot -> 0x08
    Should Be Equal As Integers    ${val}    0x08

HSION Sets HSIRDY
    Start STM32N6
    # Set HSION (bit 3) in CR
    Write RCC Register    ${CR_OFFSET}    0x08
    # Read SR: HSIRDY (bit 3) should mirror HSION
    ${sr}=    Read RCC Register    ${SR_OFFSET}
    ${hsirdy}=    Evaluate    (${sr} >> 3) & 1
    Should Be Equal As Integers    ${hsirdy}    1

HSEON Sets HSERDY
    Start STM32N6
    # Set HSEON (bit 4) in CR
    Write RCC Register    ${CR_OFFSET}    0x10
    # Read SR: HSERDY (bit 4) should mirror HSEON
    ${sr}=    Read RCC Register    ${SR_OFFSET}
    ${hserdy}=    Evaluate    (${sr} >> 4) & 1
    Should Be Equal As Integers    ${hserdy}    1

PLL1ON Sets PLL1RDY
    Start STM32N6
    # Set PLL1ON (bit 8) in CR
    Write RCC Register    ${CR_OFFSET}    0x100
    # Read SR: PLL1RDY (bit 8) should mirror PLL1ON
    ${sr}=    Read RCC Register    ${SR_OFFSET}
    ${pll1rdy}=    Evaluate    (${sr} >> 8) & 1
    Should Be Equal As Integers    ${pll1rdy}    1

Peripheral Clock GPIOE
    Start STM32N6
    # AHB4ENR: GPIOEEN is bit 4
    Write RCC Register    ${AHB4ENR}    0x10
    ${val}=    Read RCC Register    ${AHB4ENR}
    ${gpioeen}=    Evaluate    (${val} >> 4) & 1
    Should Be Equal As Integers    ${gpioeen}    1

Peripheral Clock USART1
    Start STM32N6
    # APB2ENR: USART1EN is bit 4
    Write RCC Register    ${APB2ENR}    0x10
    ${val}=    Read RCC Register    ${APB2ENR}
    ${usart1en}=    Evaluate    (${val} >> 4) & 1
    Should Be Equal As Integers    ${usart1en}    1

Boot Regression
    Start STM32N6
    Wait For NSH
