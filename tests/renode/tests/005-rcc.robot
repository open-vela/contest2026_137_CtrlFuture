*** Settings ***
Resource        resources/stm32n6-common.robot
Force Tags      rcc

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
HSION Sets HSIRDY
    [Tags]    L2-state
    Start STM32N6
    # Set HSION (bit 3) in CR
    Write RCC Register    ${CR_OFFSET}    0x08
    # Read SR: HSIRDY (bit 3) should mirror HSION
    ${sr}=    Read RCC Register    ${SR_OFFSET}
    ${hsirdy}=    Evaluate    (${sr} >> 3) & 1
    Should Be Equal As Integers    ${hsirdy}    1

HSEON Sets HSERDY
    [Tags]    L2-state
    Start STM32N6
    # Set HSEON (bit 4) in CR
    Write RCC Register    ${CR_OFFSET}    0x10
    # Read SR: HSERDY (bit 4) should mirror HSEON
    ${sr}=    Read RCC Register    ${SR_OFFSET}
    ${hserdy}=    Evaluate    (${sr} >> 4) & 1
    Should Be Equal As Integers    ${hserdy}    1

PLL1ON Sets PLL1RDY
    [Tags]    L2-state
    Start STM32N6
    # Set PLL1ON (bit 8) in CR
    Write RCC Register    ${CR_OFFSET}    0x100
    # Read SR: PLL1RDY (bit 8) should mirror PLL1ON
    ${sr}=    Read RCC Register    ${SR_OFFSET}
    ${pll1rdy}=    Evaluate    (${sr} >> 8) & 1
    Should Be Equal As Integers    ${pll1rdy}    1

Peripheral Clock GPIOE
    [Tags]    L2-state
    Start STM32N6
    # AHB4ENR: GPIOEEN is bit 4
    Write RCC Register    ${AHB4ENR}    0x10
    ${val}=    Read RCC Register    ${AHB4ENR}
    ${gpioeen}=    Evaluate    (${val} >> 4) & 1
    Should Be Equal As Integers    ${gpioeen}    1

Peripheral Clock USART1
    [Tags]    L2-state
    Start STM32N6
    # APB2ENR: USART1EN is bit 4
    Write RCC Register    ${APB2ENR}    0x10
    ${val}=    Read RCC Register    ${APB2ENR}
    ${usart1en}=    Evaluate    (${val} >> 4) & 1
    Should Be Equal As Integers    ${usart1en}    1

CSR Sets PLL1ON Via Atomic Alias
    [Tags]    L2-state
    Start STM32N6
    # CSR @ +0x0800: write-1 to bit 8 sets CR.PLL1ON atomically
    Write RCC Register    0x0800    0x100
    ${cr}=    Read RCC Register    ${CR_OFFSET}
    ${pll1on}=    Evaluate    (${cr} >> 8) & 1
    Should Be Equal As Integers    ${pll1on}    1
    # SR.PLL1RDY should mirror it
    ${sr}=    Read RCC Register    ${SR_OFFSET}
    ${pll1rdy}=    Evaluate    (${sr} >> 8) & 1
    Should Be Equal As Integers    ${pll1rdy}    1

CCR Clears PLL1ON Via Atomic Alias
    [Tags]    L2-state
    Start STM32N6
    Write RCC Register    0x0800    0x100
    # CCR @ +0x1000: write-1 to bit 8 clears CR.PLL1ON atomically
    Write RCC Register    0x1000    0x100
    ${cr}=    Read RCC Register    ${CR_OFFSET}
    ${pll1on}=    Evaluate    (${cr} >> 8) & 1
    Should Be Equal As Integers    ${pll1on}    0

CFGR1 Switches To IC1 IC2 IC6 IC11 Group
    [Tags]    L2-state
    Start STM32N6
    # CPUSW[17:16]=0b11 selects IC1; SYSSW[25:24]=0b11 selects the
    # IC2/IC6/IC11 group. Written together per hardware note.
    Write RCC Register    0x0020    0x03030000
    ${cfgr1}=    Read RCC Register    0x0020
    ${cpusws}=    Evaluate    (${cfgr1} >> 20) & 0x3
    ${syssws}=    Evaluate    (${cfgr1} >> 28) & 0x3
    Should Be Equal As Integers    ${cpusws}    3
    Should Be Equal As Integers    ${syssws}    3

CFGR1 Ignores Second Write After Switch
    [Tags]    L2-state
    Start STM32N6
    # First write switches to IC1/IC2+IC6+IC11 (locks CFGR1)
    Write RCC Register    0x0020    0x03030000
    # Second write attempting to switch back to reset-value mux
    # selection must be ignored (models the hardware lock)
    Write RCC Register    0x0020    0x00000000
    ${cfgr1}=    Read RCC Register    0x0020
    ${cpusws}=    Evaluate    (${cfgr1} >> 20) & 0x3
    Should Be Equal As Integers    ${cpusws}    3

IC1CFGR And DIVENR Are Writable
    [Tags]    L1-register
    Start STM32N6
    # IC1CFGR @ 0x00C4: SEL=PLL1(0), INT=0 (divide by 1)
    Write RCC Register    0x00C4    0x00000000
    ${ic1cfgr}=    Read RCC Register    0x00C4
    Should Be Equal As Integers    ${ic1cfgr}    0x00000000
    # DIVENR @ 0x0240: enable IC1
    Write RCC Register    0x0240    0x00000001
    ${divenr}=    Read RCC Register    0x0240
    Should Be Equal As Integers    ${divenr}    0x00000001

PLL1CFGR1 Is Writable
    [Tags]    L1-register
    Start STM32N6
    # PLL1CFGR1 @ 0x0080: SEL=HSI(0), DIVM=3 (<<20), DIVN=49 (<<8)
    ${val}=    Evaluate    (3 << 20) | (49 << 8)
    Write RCC Register    0x0080    ${val}
    ${readback}=    Read RCC Register    0x0080
    Should Be Equal As Integers    ${readback}    ${val}

Boot Regression
    [Tags]    boot-regression
    Start STM32N6
    Wait For Prompt On Uart    nsh>    timeout=120
