*** Settings ***
Resource        resources/stm32n6-common.robot

*** Variables ***
${SDMMC_BASE}   0x48040000

*** Keywords ***
Read SDMMC Register
    [Arguments]    ${offset}
    ${addr}=    Evaluate    ${SDMMC_BASE} + ${offset}
    ${val}=     Execute Command    sysbus ReadDoubleWord ${addr}
    ${val}=     Strip String    ${val}
    [Return]    ${val}

Write SDMMC Register
    [Arguments]    ${offset}    ${value}
    ${addr}=    Evaluate    ${SDMMC_BASE} + ${offset}
    Execute Command    sysbus WriteDoubleWord ${addr} ${value}

*** Test Cases ***
SDMMC1 POWER Reset Value
    Start STM32N6
    ${val}=    Read SDMMC Register    0x00
    Should Be Equal As Integers    ${val}    0

SDMMC1 CLKCR Writable
    Start STM32N6
    Write SDMMC Register    0x04    0x0100
    ${val}=    Read SDMMC Register    0x04
    Should Be Equal As Integers    ${val}    0x0100

SDMMC1 STA Accessible
    Start STM32N6
    ${val}=    Read SDMMC Register    0x34
    Should Be True    int(${val}) >= 0

Boot Regression
    Start STM32N6
    Wait For NSH
