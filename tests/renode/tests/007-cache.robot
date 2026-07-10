*** Settings ***
Resource        resources/stm32n6-common.robot

*** Variables ***
${SCB_BASE}     0xE000ED00
${SCB_CCR}      0xE000ED14

*** Test Cases ***
SCB CCR Accessible
    Start STM32N6
    ${val}=    Execute Command    sysbus ReadDoubleWord ${SCB_CCR}
    ${val}=    Strip String    ${val}
    # CCR should be readable (cache control register)
    Should Be True    int(${val}) >= 0

Boot Regression
    Start STM32N6
    Wait For NSH
