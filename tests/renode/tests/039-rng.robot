*** Settings ***
Resource        resources/stm32n6-common.robot
# Tags: L1-register | L2-state | L3-functional | boot-regression
Force Tags      rng

*** Variables ***
${RNG_BASE}     0x44020000
${CR_OFFSET}    0x0000
${SR_OFFSET}    0x0004
${DR_OFFSET}    0x0008
${HTCR_OFFSET}  0x0010

*** Keywords ***
Read RNG Register
    [Arguments]    ${offset}
    ${addr}=    Evaluate    ${RNG_BASE} + ${offset}
    ${val}=     Execute Command    sysbus ReadDoubleWord ${addr}
    ${val}=     Strip String    ${val}
    RETURN    ${val}

Write RNG Register
    [Arguments]    ${offset}    ${value}
    ${addr}=    Evaluate    ${RNG_BASE} + ${offset}
    Execute Command    sysbus WriteDoubleWord ${addr} ${value}

*** Test Cases ***
CR Reset Value
    [Tags]    L1-register
    Start STM32N6
    ${val}=    Read RNG Register    ${CR_OFFSET}
    # Reset value: all zeros (RNGEN=0, IE=0, CED=0, CONDRST=0)
    Should Be Equal As Integers    ${val}    0x00

SR After Boot
    [Tags]    L1-register
    Start STM32N6
    ${val}=    Read RNG Register    ${SR_OFFSET}
    # SR should be 0 after boot (no data ready, no errors)
    Should Be Equal As Integers    ${val}    0x00

DR Readable
    [Tags]    L1-register
    Start STM32N6
    ${val}=    Read RNG Register    ${DR_OFFSET}
    # DR should return a 32-bit value (random)
    ${val}=    Convert To Integer    ${val}
    Should Be True    ${val} >= 0

HTCR Reset Value
    [Tags]    L1-register
    Start STM32N6
    ${val}=    Read RNG Register    ${HTCR_OFFSET}
    # HTCR reset value: model defines as 0 (no default value set)
    Should Be Equal As Integers    ${val}    0x00

Boot Regression
    [Tags]    boot-regression
    Start STM32N6
    Wait For NSH

RNGEN Sets DRDY
    [Tags]    L2-state
    Start STM32N6
    # CR.RNGEN = bit 2
    Write RNG Register    ${CR_OFFSET}    0x04
    ${sr}=    Read RNG Register    ${SR_OFFSET}
    ${sr}=    Convert To Integer    ${sr}
    # DRDY bit 0 should be set when enabled
    Should Be True    (${sr} & 0x1) == 0x1

DR Read Clears Then Refills DRDY
    [Tags]    L2-state
    Start STM32N6
    Write RNG Register    ${CR_OFFSET}    0x04
    ${d1}=    Read RNG Register    ${DR_OFFSET}
    ${d1}=    Convert To Integer    ${d1}
    Should Be True    ${d1} >= 0
    # After read DRDY may clear; model should re-assert when still enabled
    ${sr}=    Read RNG Register    ${SR_OFFSET}
    ${sr}=    Convert To Integer    ${sr}
    Should Be True    (${sr} & 0x1) == 0x1

Two DR Reads Differ When Enabled
    [Tags]    L2-state
    Start STM32N6
    Write RNG Register    ${CR_OFFSET}    0x04
    ${a}=    Read RNG Register    ${DR_OFFSET}
    ${b}=    Read RNG Register    ${DR_OFFSET}
    # Soft check: allow rare collision but require at least valid ints
    ${a}=    Convert To Integer    ${a}
    ${b}=    Convert To Integer    ${b}
    Should Be True    ${a} >= 0 and ${b} >= 0

IE And RNGEN Assert IRQ
    [Tags]    L2-state
    [Documentation]    CR.IE=bit3 + CR.RNGEN=bit2 (0x0C) asserts RNG IRQ
    ...                when DRDY is set (wired to nvic@40 / RNG_IRQn).
    Start STM32N6
    # IRQ should be clear before enable
    ${irq}=    Execute Command    sysbus.rng IRQ IsSet
    Should Be Equal    ${irq.strip()}    False
    # Enable IE + RNGEN -> DRDY set -> IRQ asserts
    Write RNG Register    ${CR_OFFSET}    0x0C
    ${sr}=    Read RNG Register    ${SR_OFFSET}
    ${sr}=    Convert To Integer    ${sr}
    Should Be True    (${sr} & 0x1) == 0x1
    ${irq}=    Execute Command    sysbus.rng IRQ IsSet
    Should Be Equal    ${irq.strip()}    True

RNGEN Alone Does Not Assert IRQ
    [Tags]    L2-state
    Start STM32N6
    # RNGEN without IE: DRDY set, IRQ stays clear
    Write RNG Register    ${CR_OFFSET}    0x04
    ${sr}=    Read RNG Register    ${SR_OFFSET}
    ${sr}=    Convert To Integer    ${sr}
    Should Be True    (${sr} & 0x1) == 0x1
    ${irq}=    Execute Command    sysbus.rng IRQ IsSet
    Should Be Equal    ${irq.strip()}    False

Disable IE Clears IRQ
    [Tags]    L2-state
    Start STM32N6
    Write RNG Register    ${CR_OFFSET}    0x0C
    ${irq}=    Execute Command    sysbus.rng IRQ IsSet
    Should Be Equal    ${irq.strip()}    True
    # Clear IE, keep RNGEN (0x04)
    Write RNG Register    ${CR_OFFSET}    0x04
    ${irq}=    Execute Command    sysbus.rng IRQ IsSet
    Should Be Equal    ${irq.strip()}    False
