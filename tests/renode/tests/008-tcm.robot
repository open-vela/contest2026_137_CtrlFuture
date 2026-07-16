*** Settings ***
Resource        resources/stm32n6-common.robot
Force Tags      tcm

*** Variables ***
${ITCM_BASE}    0x00000000
${DTCM_BASE}    0x20000000

*** Test Cases ***
ITCM Accessible
    [Tags]    L1-register
    Start STM32N6
    Execute Command    sysbus WriteDoubleWord ${ITCM_BASE} 0xDEADBEEF
    ${val}=    Execute Command    sysbus ReadDoubleWord ${ITCM_BASE}
    ${val}=    Strip String    ${val}
    Should Be Equal As Integers    ${val}    0xDEADBEEF

DTCM Accessible
    [Tags]    L1-register
    Start STM32N6
    Execute Command    sysbus WriteDoubleWord ${DTCM_BASE} 0xCAFEBABE
    ${val}=    Execute Command    sysbus ReadDoubleWord ${DTCM_BASE}
    ${val}=    Strip String    ${val}
    Should Be Equal As Integers    ${val}    0xCAFEBABE

ITCM Range Readable
    [Tags]    L1-register
    Start STM32N6
    # Read at ITCM base + 0x100 (within 64KB range). Uses the
    # ${ITCM_BASE} variable instead of a hardcoded literal so the
    # test still targets the correct address if ITCM_BASE ever
    # changes (e.g. a future Secure/Non-secure alias).
    ${addr}=    Evaluate    ${ITCM_BASE} + 0x100
    Execute Command    sysbus WriteDoubleWord ${addr} 0x12345678
    ${val}=    Execute Command    sysbus ReadDoubleWord ${addr}
    ${val}=    Strip String    ${val}
    ${val}=    Convert To Integer    ${val}
    Should Be Equal As Integers    ${val}    0x12345678

Boot Regression
    [Tags]    boot-regression
    Start STM32N6
    Wait For NSH
