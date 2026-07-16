*** Settings ***
Resource        resources/stm32n6-common.robot
Force Tags      gpio-instances

*** Variables ***
${GPIOB_BASE}    0x46020400
${GPIOC_BASE}    0x46020800
${GPIOD_BASE}    0x46020C00
${GPIOF_BASE}    0x46021400
${GPIOG_BASE}    0x46021800
${GPIOH_BASE}    0x46021C00
${GPION_BASE}    0x46023400
${GPIOO_BASE}    0x46023800
${GPIOP_BASE}    0x46023C00
${GPIOQ_BASE}    0x46024000
${GPIO_MODER}    0x00
${GPIO_IDR}      0x10
${GPIO_ODR}      0x14

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

Verify GPIO Port Output Write
    [Documentation]    Mirrors the "GPIOE Output Write" pattern in
    ...                006-gpio.robot: configure pin 0 as output
    ...                (MODER bits [1:0] = 01), drive ODR bit 0
    ...                high, and read it back. This is the L2-state
    ...                ceiling for STM32_GPIOPort in this platform:
    ...                the .repl has no Connections wiring any GPIO
    ...                port pin to another peripheral or to itself,
    ...                so there is no way to drive a real external
    ...                signal into an input pin and observe it on
    ...                IDR -- true pin-level L3 is not reachable
    ...                without adding such wiring.
    [Arguments]    ${base}
    Write GPIO Register    ${base}    ${GPIO_MODER}    0x01
    Write GPIO Register    ${base}    ${GPIO_ODR}    0x01
    ${odr}=    Read GPIO Register    ${base}    ${GPIO_ODR}
    ${pin0}=    Evaluate    int(${odr}) & 1
    Should Be Equal As Integers    ${pin0}    1

*** Test Cases ***
GPIOB Accessible
    [Tags]    L1-register
    Start STM32N6
    ${val}=    Read GPIO Register    ${GPIOB_BASE}    ${GPIO_MODER}
    Should Be Equal As Integers    ${val}    0

GPIOC Accessible
    [Tags]    L1-register
    Start STM32N6
    ${val}=    Read GPIO Register    ${GPIOC_BASE}    ${GPIO_MODER}
    Should Be Equal As Integers    ${val}    0

GPIOD Accessible
    [Tags]    L1-register
    Start STM32N6
    ${val}=    Read GPIO Register    ${GPIOD_BASE}    ${GPIO_MODER}
    Should Be Equal As Integers    ${val}    0

GPIOF Accessible
    [Tags]    L1-register
    Start STM32N6
    ${val}=    Read GPIO Register    ${GPIOF_BASE}    ${GPIO_MODER}
    Should Be Equal As Integers    ${val}    0

GPIOG Accessible
    [Tags]    L1-register
    Start STM32N6
    ${val}=    Read GPIO Register    ${GPIOG_BASE}    ${GPIO_MODER}
    Should Be Equal As Integers    ${val}    0

GPIOH Accessible
    [Tags]    L1-register
    Start STM32N6
    ${val}=    Read GPIO Register    ${GPIOH_BASE}    ${GPIO_MODER}
    Should Be Equal As Integers    ${val}    0

GPION Accessible
    [Tags]    L1-register
    Start STM32N6
    ${val}=    Read GPIO Register    ${GPION_BASE}    ${GPIO_MODER}
    Should Be Equal As Integers    ${val}    0

GPIOO Accessible
    [Tags]    L1-register
    Start STM32N6
    ${val}=    Read GPIO Register    ${GPIOO_BASE}    ${GPIO_MODER}
    Should Be Equal As Integers    ${val}    0

GPIOP Accessible
    [Tags]    L1-register
    Start STM32N6
    ${val}=    Read GPIO Register    ${GPIOP_BASE}    ${GPIO_MODER}
    Should Be Equal As Integers    ${val}    0

GPIOQ Accessible
    [Tags]    L1-register
    Start STM32N6
    ${val}=    Read GPIO Register    ${GPIOQ_BASE}    ${GPIO_MODER}
    Should Be Equal As Integers    ${val}    0

GPIOB IDR Readback
    [Tags]    L1-register
    Start STM32N6
    ${idr}=    Read GPIO Register    ${GPIOB_BASE}    ${GPIO_IDR}
    Should Be True    int(${idr}) >= 0

GPIOB Output Write
    [Tags]    L2-state
    Create STM32N6 Machine
    Verify GPIO Port Output Write    ${GPIOB_BASE}

GPIOC Output Write
    [Tags]    L2-state
    Create STM32N6 Machine
    Verify GPIO Port Output Write    ${GPIOC_BASE}

GPIOD Output Write
    [Tags]    L2-state
    Create STM32N6 Machine
    Verify GPIO Port Output Write    ${GPIOD_BASE}

GPIOF Output Write
    [Tags]    L2-state
    Create STM32N6 Machine
    Verify GPIO Port Output Write    ${GPIOF_BASE}

GPIOG Output Write
    [Tags]    L2-state
    Create STM32N6 Machine
    Verify GPIO Port Output Write    ${GPIOG_BASE}

GPIOH Output Write
    [Tags]    L2-state
    Create STM32N6 Machine
    Verify GPIO Port Output Write    ${GPIOH_BASE}

GPION Output Write
    [Tags]    L2-state
    Create STM32N6 Machine
    Verify GPIO Port Output Write    ${GPION_BASE}

GPIOO Output Write
    [Tags]    L2-state
    Create STM32N6 Machine
    Verify GPIO Port Output Write    ${GPIOO_BASE}

GPIOP Output Write
    [Tags]    L2-state
    Create STM32N6 Machine
    Verify GPIO Port Output Write    ${GPIOP_BASE}

GPIOQ Output Write
    [Tags]    L2-state
    Create STM32N6 Machine
    Verify GPIO Port Output Write    ${GPIOQ_BASE}

Boot Regression
    [Tags]    boot-regression
    Start STM32N6
    Wait For NSH
