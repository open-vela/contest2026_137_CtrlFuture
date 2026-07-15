*** Settings ***
Resource        resources/stm32n6-common.robot

*** Test Cases ***
SRAM Region Accessible By CPU
    [Tags]    L2-state
    Start STM32N6
    # Write to SRAM region @ 0x34001000
    Execute Command    sysbus WriteDoubleWord 0x34001000 0xA5A5A5A5
    ${val}=    Execute Command    sysbus ReadDoubleWord 0x34001000
    ${val}=    Strip String    ${val}
    ${val}=    Convert To Integer    ${val}
    Should Be Equal As Integers    ${val}    0xA5A5A5A5

DMA Source Destination In SRAM Range
    [Tags]    L2-state
    Start STM32N6
    # Verify GPDMA channel CSAR/CDAR can point to SRAM region
    # GPDMA1 @ 0x40021000, Channel 0 base = 0x40021050
    # CSAR @ channel_base + 0x4C = 0x4002109C
    # CDAR @ channel_base + 0x50 = 0x400210A0
    # Write CSAR=0x34001000, CDAR=0x34002000
    Execute Command    sysbus WriteDoubleWord 0x4002109C 0x34001000
    Execute Command    sysbus WriteDoubleWord 0x400210A0 0x34002000
    ${sar}=    Execute Command    sysbus ReadDoubleWord 0x4002109C
    ${sar}=    Strip String    ${sar}
    ${sar}=    Convert To Integer    ${sar}
    ${dar}=    Execute Command    sysbus ReadDoubleWord 0x400210A0
    ${dar}=    Strip String    ${dar}
    ${dar}=    Convert To Integer    ${dar}
    Should Be Equal As Integers    ${sar}    0x34001000
    Should Be Equal As Integers    ${dar}    0x34002000

Boot Regression
    [Tags]    boot-regression
    Start STM32N6
    Wait For NSH
