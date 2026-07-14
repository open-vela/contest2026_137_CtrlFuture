*** Settings ***
Resource        resources/stm32n6-common.robot
Force Tags      otg

*** Variables ***
${OTG_BASE}     0x48040000
${REG_GOTGCTL}  0x000
${REG_GAHBCFG}  0x008
${REG_GRSTCTL}  0x010
${REG_GINTSTS}  0x014

*** Keywords ***
Read OTG Register
    [Arguments]    ${offset}
    ${addr}=    Evaluate    ${OTG_BASE} + ${offset}
    ${val}=     Execute Command    sysbus ReadDoubleWord ${addr}
    ${val}=     Strip String    ${val}
    RETURN    ${val}

Write OTG Register
    [Arguments]    ${offset}    ${value}
    ${addr}=    Evaluate    ${OTG_BASE} + ${offset}
    Execute Command    sysbus WriteDoubleWord ${addr} ${value}

*** Test Cases ***
OTG GOTGCTL Reset Value
    [Tags]    L1-register
    Start STM32N6
    ${val}=    Read OTG Register    ${REG_GOTGCTL}
    Should Be Equal As Integers    ${val}    0

OTG GAHBCFG Writable
    [Tags]    L1-register
    Start STM32N6
    Write OTG Register    ${REG_GAHBCFG}    0x00000001
    ${val}=    Read OTG Register    ${REG_GAHBCFG}
    Should Be Equal As Integers    ${val}    0x00000001

OTG GINTSTS Accessible
    [Tags]    L1-register
    Start STM32N6
    ${val}=    Read OTG Register    ${REG_GINTSTS}
    Should Be True    int(${val}) >= 0

OTG CSRST Self Clears
    [Tags]    L2-state
    Start STM32N6
    Write OTG Register    ${REG_GRSTCTL}    0x01
    ${val}=    Read OTG Register    ${REG_GRSTCTL}
    ${val}=    Convert To Integer    ${val}
    # CSRST bit0 clear after soft reset
    Should Be True    (${val} & 0x1) == 0
    # AHBIDL bit31 set (core idle)
    Should Be True    (${val} & 0x80000000) == 0x80000000

OTG TX RX FIFO Flush Self Clears
    [Tags]    L2-state
    Start STM32N6
    # RXFFLSH bit4 | TXFFLSH bit5
    Write OTG Register    ${REG_GRSTCTL}    0x30
    ${val}=    Read OTG Register    ${REG_GRSTCTL}
    ${val}=    Convert To Integer    ${val}
    Should Be True    (${val} & 0x30) == 0

Boot Regression
    [Tags]    boot-regression
    Start STM32N6
    Wait For NSH
