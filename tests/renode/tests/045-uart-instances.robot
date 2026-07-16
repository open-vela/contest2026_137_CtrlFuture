*** Settings ***
Resource        resources/stm32n6-common.robot
Force Tags      uart-instances

*** Variables ***
${USART3_BASE}     0x40004800
${UART4_BASE}       0x40004C00
${UART5_BASE}       0x40005000
${UART7_BASE}       0x40007800
${UART8_BASE}       0x40007C00
${UART9_BASE}       0x42001800
${USART10_BASE}    0x42001C00

# STM32F7_USART (CMSIS USART_TypeDef) register offsets, shared by
# every instance in stm32n647x0.repl (see Renode STM32F7_USART.cs).
${REG_CR1}    0x00
${REG_BRR}    0x0C
${REG_ISR}    0x1C
${REG_RDR}    0x24
${REG_TDR}    0x28

*** Keywords ***
Read UART Register
    [Arguments]    ${base}    ${offset}
    ${addr}=    Evaluate    ${base} + ${offset}
    ${val}=     Execute Command    sysbus ReadDoubleWord ${addr}
    ${val}=     Strip String    ${val}
    RETURN    ${val}

Write UART Register
    [Arguments]    ${base}    ${offset}    ${value}
    ${addr}=    Evaluate    ${base} + ${offset}
    Execute Command    sysbus WriteDoubleWord ${addr} ${value}

*** Test Cases ***
USART3 Register Accessible
    [Tags]    L1-register
    Start STM32N6
    ${val}=    Read UART Register    ${USART3_BASE}    ${REG_CR1}
    Should Be Equal As Integers    ${val}    0

UART4 Register Accessible
    [Tags]    L1-register
    Start STM32N6
    ${val}=    Read UART Register    ${UART4_BASE}    ${REG_CR1}
    Should Be Equal As Integers    ${val}    0

UART5 Register Accessible
    [Tags]    L1-register
    Start STM32N6
    ${val}=    Read UART Register    ${UART5_BASE}    ${REG_CR1}
    Should Be Equal As Integers    ${val}    0

UART7 Register Accessible
    [Tags]    L1-register
    Start STM32N6
    ${val}=    Read UART Register    ${UART7_BASE}    ${REG_CR1}
    Should Be Equal As Integers    ${val}    0

UART8 Register Accessible
    [Tags]    L1-register
    Start STM32N6
    ${val}=    Read UART Register    ${UART8_BASE}    ${REG_CR1}
    Should Be Equal As Integers    ${val}    0

UART9 Register Accessible
    [Tags]    L1-register
    Start STM32N6
    ${val}=    Read UART Register    ${UART9_BASE}    ${REG_CR1}
    Should Be Equal As Integers    ${val}    0

USART10 Register Accessible
    [Tags]    L1-register
    Start STM32N6
    ${val}=    Read UART Register    ${USART10_BASE}    ${REG_CR1}
    Should Be Equal As Integers    ${val}    0

USART3 CR1 UE/TE Writable
    [Tags]    L2-state
    Create STM32N6 Machine
    Write UART Register    ${USART3_BASE}    ${REG_CR1}    0x9
    ${val}=    Read UART Register    ${USART3_BASE}    ${REG_CR1}
    ${val}=    Convert To Integer    ${val}
    Should Be True    (${val} & 0x9) == 0x9

UART4 BRR Writable
    [Tags]    L2-state
    Create STM32N6 Machine
    Write UART Register    ${UART4_BASE}    ${REG_BRR}    0x0341
    ${val}=    Read UART Register    ${UART4_BASE}    ${REG_BRR}
    Should Be Equal As Integers    ${val}    0x0341

UART5 CR1 UE/TE Writable
    [Tags]    L2-state
    Create STM32N6 Machine
    Write UART Register    ${UART5_BASE}    ${REG_CR1}    0x9
    ${val}=    Read UART Register    ${UART5_BASE}    ${REG_CR1}
    ${val}=    Convert To Integer    ${val}
    Should Be True    (${val} & 0x9) == 0x9

UART7 CR1 UE/TE Writable
    [Tags]    L2-state
    Create STM32N6 Machine
    Write UART Register    ${UART7_BASE}    ${REG_CR1}    0x9
    ${val}=    Read UART Register    ${UART7_BASE}    ${REG_CR1}
    ${val}=    Convert To Integer    ${val}
    Should Be True    (${val} & 0x9) == 0x9

UART8 CR1 UE/TE Writable
    [Tags]    L2-state
    Create STM32N6 Machine
    Write UART Register    ${UART8_BASE}    ${REG_CR1}    0x9
    ${val}=    Read UART Register    ${UART8_BASE}    ${REG_CR1}
    ${val}=    Convert To Integer    ${val}
    Should Be True    (${val} & 0x9) == 0x9

UART9 CR1 UE/TE Writable
    [Tags]    L2-state
    Create STM32N6 Machine
    Write UART Register    ${UART9_BASE}    ${REG_CR1}    0x9
    ${val}=    Read UART Register    ${UART9_BASE}    ${REG_CR1}
    ${val}=    Convert To Integer    ${val}
    Should Be True    (${val} & 0x9) == 0x9

USART10 CR1 UE/TE Writable
    [Tags]    L2-state
    Create STM32N6 Machine
    Write UART Register    ${USART10_BASE}    ${REG_CR1}    0x9
    ${val}=    Read UART Register    ${USART10_BASE}    ${REG_CR1}
    ${val}=    Convert To Integer    ${val}
    Should Be True    (${val} & 0x9) == 0x9

USART3 TDR Write Reaches Renode UART Backend
    [Tags]    L3-functional
    Create STM32N6 Machine
    Create Terminal Tester    sysbus.usart3
    Write UART Register    ${USART3_BASE}    ${REG_CR1}    0x9
    Write UART Register    ${USART3_BASE}    ${REG_TDR}    0x41
    Wait For Line On Uart    A    treatAsRegex=true    includeUnfinishedLine=true

UART4 TDR Write Reaches Renode UART Backend
    [Tags]    L3-functional
    Create STM32N6 Machine
    Create Terminal Tester    sysbus.uart4
    Write UART Register    ${UART4_BASE}    ${REG_CR1}    0x9
    Write UART Register    ${UART4_BASE}    ${REG_TDR}    0x42
    Wait For Line On Uart    B    treatAsRegex=true    includeUnfinishedLine=true

UART5 TDR Write Reaches Renode UART Backend
    [Tags]    L3-functional
    Create STM32N6 Machine
    Create Terminal Tester    sysbus.uart5
    Write UART Register    ${UART5_BASE}    ${REG_CR1}    0x9
    Write UART Register    ${UART5_BASE}    ${REG_TDR}    0x43
    Wait For Line On Uart    C    treatAsRegex=true    includeUnfinishedLine=true

UART7 TDR Write Reaches Renode UART Backend
    [Tags]    L3-functional
    Create STM32N6 Machine
    Create Terminal Tester    sysbus.uart7
    Write UART Register    ${UART7_BASE}    ${REG_CR1}    0x9
    Write UART Register    ${UART7_BASE}    ${REG_TDR}    0x44
    Wait For Line On Uart    D    treatAsRegex=true    includeUnfinishedLine=true

UART8 TDR Write Reaches Renode UART Backend
    [Tags]    L3-functional
    Create STM32N6 Machine
    Create Terminal Tester    sysbus.uart8
    Write UART Register    ${UART8_BASE}    ${REG_CR1}    0x9
    Write UART Register    ${UART8_BASE}    ${REG_TDR}    0x45
    Wait For Line On Uart    E    treatAsRegex=true    includeUnfinishedLine=true

UART9 TDR Write Reaches Renode UART Backend
    [Tags]    L3-functional
    Create STM32N6 Machine
    Create Terminal Tester    sysbus.uart9
    Write UART Register    ${UART9_BASE}    ${REG_CR1}    0x9
    Write UART Register    ${UART9_BASE}    ${REG_TDR}    0x46
    Wait For Line On Uart    F    treatAsRegex=true    includeUnfinishedLine=true

USART10 TDR Write Reaches Renode UART Backend
    [Tags]    L3-functional
    Create STM32N6 Machine
    Create Terminal Tester    sysbus.usart10
    Write UART Register    ${USART10_BASE}    ${REG_CR1}    0x9
    Write UART Register    ${USART10_BASE}    ${REG_TDR}    0x47
    Wait For Line On Uart    G    treatAsRegex=true    includeUnfinishedLine=true

Boot Regression
    [Tags]    boot-regression
    Start STM32N6
    Wait For NSH
