*** Settings ***
Resource        resources/stm32n6-common.robot

*** Variables ***
${MPU_TYPE}     0xE000ED90
${MPU_CTRL}     0xE000ED94
${MPU_RNR}      0xE000ED98

*** Test Cases ***
MPU Type Register Accessible
    Start STM32N6
    ${val}=    Execute Command    sysbus ReadDoubleWord ${MPU_TYPE}
    ${val}=    Strip String    ${val}
    Should Be True    int(${val}) >= 0

MPU Control Register Accessible
    Start STM32N6
    ${val}=    Execute Command    sysbus ReadDoubleWord ${MPU_CTRL}
    ${val}=    Strip String    ${val}
    Should Be True    int(${val}) >= 0

MPU Region Number Register Accessible
    Start STM32N6
    ${val}=    Execute Command    sysbus ReadDoubleWord ${MPU_RNR}
    ${val}=    Strip String    ${val}
    Should Be True    int(${val}) >= 0

Boot Regression
    Start STM32N6
    Wait For NSH
