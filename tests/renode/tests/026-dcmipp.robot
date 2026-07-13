*** Settings ***
Resource        resources/stm32n6-common.robot

*** Variables ***
${DCMIPP_BASE}  0x48002000

*** Keywords ***
Read DCMIPP Register
    [Arguments]    ${offset}
    ${addr}=    Evaluate    ${DCMIPP_BASE} + ${offset}
    ${val}=     Execute Command    sysbus ReadDoubleWord ${addr}
    ${val}=     Strip String    ${val}
    RETURN    ${val}

Write DCMIPP Register
    [Arguments]    ${offset}    ${value}
    ${addr}=    Evaluate    ${DCMIPP_BASE} + ${offset}
    Execute Command    sysbus WriteDoubleWord ${addr} ${value}

*** Test Cases ***
DCMIPP CR Reset Value
    Start STM32N6
    # CR @ 0x00
    ${val}=    Read DCMIPP Register    0x00
    Should Be Equal As Integers    ${val}    0

DCMIPP CR Writable
    Start STM32N6
    # CR @ 0x00
    Write DCMIPP Register    0x00    0x01
    ${val}=    Read DCMIPP Register    0x00
    Should Be Equal As Integers    ${val}    0x01

DCMIPP SR Accessible
    Start STM32N6
    # SR @ 0x04
    ${val}=    Read DCMIPP Register    0x04
    Should Be True    int(${val}) >= 0

Boot Regression
    Start STM32N6
    Wait For NSH
