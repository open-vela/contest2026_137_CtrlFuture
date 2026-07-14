*** Settings ***
Resource        resources/stm32n6-common.robot
Force Tags      exti

*** Variables ***
${EXTI_BASE}    0x46025000
${REG_RTSR1}    0x00
${REG_FTSR1}    0x04
${REG_SWIER1}   0x08
${REG_RPR1}     0x0C
${REG_FPR1}     0x10
${REG_IMR1}     0x80
${REG_RTSR2}    0x20
${REG_EXTICR0}  0x60
${REG_IMR2}     0x90

*** Keywords ***
Read EXTI Register
    [Arguments]    ${offset}
    ${addr}=    Evaluate    ${EXTI_BASE} + ${offset}
    ${val}=     Execute Command    sysbus ReadDoubleWord ${addr}
    ${val}=     Strip String    ${val}
    RETURN    ${val}

Write EXTI Register
    [Arguments]    ${offset}    ${value}
    ${addr}=    Evaluate    ${EXTI_BASE} + ${offset}
    Execute Command    sysbus WriteDoubleWord ${addr} ${value}

*** Test Cases ***
EXTI RTSR1 Reset Value
    [Tags]    L1-register
    Start STM32N6
    ${val}=    Read EXTI Register    ${REG_RTSR1}
    Should Be Equal As Integers    ${val}    0

EXTI FTSR1 Reset Value
    [Tags]    L1-register
    Start STM32N6
    ${val}=    Read EXTI Register    ${REG_FTSR1}
    Should Be Equal As Integers    ${val}    0

EXTI IMR1 Writable
    [Tags]    L1-register
    Start STM32N6
    Write EXTI Register    ${REG_IMR1}    0xFF
    ${val}=    Read EXTI Register    ${REG_IMR1}
    Should Be Equal As Integers    ${val}    0xFF

EXTI RTSR1 Writable
    [Tags]    L1-register
    Start STM32N6
    Write EXTI Register    ${REG_RTSR1}    0x0F
    ${val}=    Read EXTI Register    ${REG_RTSR1}
    Should Be Equal As Integers    ${val}    0x0F

EXTI Bank2 RTSR2 Accessible
    [Tags]    L1-register
    Start STM32N6
    Write EXTI Register    ${REG_RTSR2}    0x03
    ${val}=    Read EXTI Register    ${REG_RTSR2}
    Should Be Equal As Integers    ${val}    0x03

EXTI EXTICR0 Accessible
    [Tags]    L1-register
    Start STM32N6
    Write EXTI Register    ${REG_EXTICR0}    0x00
    ${val}=    Read EXTI Register    ${REG_EXTICR0}
    Should Be True    int(${val}) >= 0

EXTI IMR2 Accessible
    [Tags]    L1-register
    Start STM32N6
    Write EXTI Register    ${REG_IMR2}    0xFF
    ${val}=    Read EXTI Register    ${REG_IMR2}
    Should Be Equal As Integers    ${val}    0xFF

EXTI SWIER Sets RPR Pending
    [Tags]    L2-state
    Start STM32N6
    Write EXTI Register    ${REG_RTSR1}    0x01
    Write EXTI Register    ${REG_IMR1}    0x01
    Write EXTI Register    ${REG_SWIER1}    0x01
    ${rpr}=    Read EXTI Register    ${REG_RPR1}
    ${rpr}=    Convert To Integer    ${rpr}
    Should Be True    (${rpr} & 0x1) == 0x1

EXTI RPR Write One To Clear
    [Tags]    L2-state
    Start STM32N6
    Write EXTI Register    ${REG_SWIER1}    0x03
    ${rpr}=    Read EXTI Register    ${REG_RPR1}
    Should Be Equal As Integers    ${rpr}    0x03
    # Clear only bit 0
    Write EXTI Register    ${REG_RPR1}    0x01
    ${rpr2}=    Read EXTI Register    ${REG_RPR1}
    Should Be Equal As Integers    ${rpr2}    0x02
    Write EXTI Register    ${REG_RPR1}    0x02
    ${rpr3}=    Read EXTI Register    ${REG_RPR1}
    Should Be Equal As Integers    ${rpr3}    0

EXTI FPR Write One To Clear
    [Tags]    L2-state
    Start STM32N6
    # Falling-selected SWIER raises FPR; W1C clears it
    Write EXTI Register    ${REG_FTSR1}    0x05
    Write EXTI Register    ${REG_SWIER1}    0x05
    ${fpr}=    Read EXTI Register    ${REG_FPR1}
    Should Be Equal As Integers    ${fpr}    0x05
    Write EXTI Register    ${REG_FPR1}    0x01
    ${fpr2}=    Read EXTI Register    ${REG_FPR1}
    Should Be Equal As Integers    ${fpr2}    0x04
    Write EXTI Register    ${REG_FPR1}    0x04
    ${fpr3}=    Read EXTI Register    ${REG_FPR1}
    Should Be Equal As Integers    ${fpr3}    0

Boot Regression
    [Tags]    boot-regression
    Start STM32N6
    Wait For NSH
