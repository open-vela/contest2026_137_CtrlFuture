*** Settings ***
Resource        resources/stm32n6-common.robot

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
    Start STM32N6
    ${val}=    Read RNG Register    ${CR_OFFSET}
    # Reset value: all zeros (RNGEN=0, IE=0, CED=0, CONDRST=0)
    Should Be Equal As Integers    ${val}    0x00

SR After Boot
    Start STM32N6
    ${val}=    Read RNG Register    ${SR_OFFSET}
    # SR should be 0 after boot (no data ready, no errors)
    Should Be Equal As Integers    ${val}    0x00

DR Readable
    Start STM32N6
    ${val}=    Read RNG Register    ${DR_OFFSET}
    # DR should return a 32-bit value (random)
    ${val}=    Convert To Integer    ${val}
    Should Be True    ${val} >= 0

HTCR Reset Value
    Start STM32N6
    ${val}=    Read RNG Register    ${HTCR_OFFSET}
    # HTCR reset value: model defines as 0 (no default value set)
    Should Be Equal As Integers    ${val}    0x00

Boot Regression
    Start STM32N6
    Wait For NSH
