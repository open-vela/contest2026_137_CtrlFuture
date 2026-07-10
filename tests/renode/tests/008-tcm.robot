*** Settings ***
Resource        resources/stm32n6-common.robot

*** Variables ***
${ITCM_BASE}    0x00000000
${DTCM_BASE}    0x20000000

*** Test Cases ***
ITCM Accessible
    Start STM32N6
    Execute Command    sysbus WriteDoubleWord ${ITCM_BASE} 0xDEADBEEF
    ${val}=    Execute Command    sysbus ReadDoubleWord ${ITCM_BASE}
    ${val}=    Strip String    ${val}
    Should Be Equal As Integers    ${val}    0xDEADBEEF

DTCM Accessible
    Start STM32N6
    Execute Command    sysbus WriteDoubleWord ${DTCM_BASE} 0xCAFEBABE
    ${val}=    Execute Command    sysbus ReadDoubleWord ${DTCM_BASE}
    ${val}=    Strip String    ${val}
    Should Be Equal As Integers    ${val}    0xCAFEBABE

ITCM Range Readable
    Start STM32N6
    # Read at ITCM base + 0x100 (within 64KB range)
    ${val}=    Execute Command    sysbus ReadDoubleWord 0x100
    ${val}=    Strip String    ${val}
    Should Be True    int(${val}) >= 0

Boot Regression
    Start STM32N6
    Wait For NSH
