*** Settings ***
Resource        resources/stm32n6-common.robot

*** Variables ***
${XSPI_FLASH}   0x70000000

*** Test Cases ***
XSPI Flash Region Readable
    Start STM32N6
    ${val}=    Execute Command    sysbus ReadDoubleWord ${XSPI_FLASH}
    ${val}=    Strip String    ${val}
    # Should return 0 (zeroed memory)
    Should Be Equal As Integers    ${val}    0

XSPI Flash Region Writable
    Start STM32N6
    Execute Command    sysbus WriteDoubleWord ${XSPI_FLASH} 0x12345678
    ${val}=    Execute Command    sysbus ReadDoubleWord ${XSPI_FLASH}
    ${val}=    Strip String    ${val}
    Should Be Equal As Integers    ${val}    0x12345678

XSPI Flash Mid-Range Accessible
    Start STM32N6
    # Read at 16MB offset (within 32MB range)
    ${val}=    Execute Command    sysbus ReadDoubleWord 0x71000000
    ${val}=    Strip String    ${val}
    Should Be True    int(${val}) >= 0

Boot Regression
    Start STM32N6
    Wait For NSH
