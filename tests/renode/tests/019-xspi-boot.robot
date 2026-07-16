*** Settings ***
Resource        resources/stm32n6-common.robot
Force Tags      xspi-boot

*** Variables ***
${XSPI_FLASH}   0x70000000

*** Test Cases ***
XSPI Flash Region Readable
    [Tags]    L1-register
    Start STM32N6
    ${val}=    Execute Command    sysbus ReadDoubleWord ${XSPI_FLASH}
    ${val}=    Strip String    ${val}
    # Should return 0 (zeroed memory)
    Should Be Equal As Integers    ${val}    0

XSPI Flash Region Writable
    [Tags]    L1-register
    Start STM32N6
    Execute Command    sysbus WriteDoubleWord ${XSPI_FLASH} 0x12345678
    ${val}=    Execute Command    sysbus ReadDoubleWord ${XSPI_FLASH}
    ${val}=    Strip String    ${val}
    Should Be Equal As Integers    ${val}    0x12345678

XSPI Flash Mid-Range Accessible
    [Tags]    L1-register
    Start STM32N6
    # Read at 16MB offset (within 32MB range); write/read round-trip
    # instead of the previous "int(val) >= 0" check, which is true
    # for any 32-bit unsigned read and never verifies the mapped
    # region actually works at that offset.
    Execute Command    sysbus WriteDoubleWord 0x71000000 0x0BAD0BAD
    ${val}=    Execute Command    sysbus ReadDoubleWord 0x71000000
    ${val}=    Strip String    ${val}
    ${val}=    Convert To Integer    ${val}
    Should Be Equal As Integers    ${val}    0x0BAD0BAD

Boot Regression
    [Tags]    boot-regression
    Start STM32N6
    Wait For NSH
