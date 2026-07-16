*** Settings ***
Resource        resources/stm32n6-common.robot
Force Tags      cache

*** Variables ***
${SCB_BASE}     0xE000ED00
${SCB_CCR}      0xE000ED14

*** Keywords ***
Write SCB CCR
    [Arguments]    ${value}
    ${cmd}=    Catenate    SEPARATOR=${SPACE}    sysbus    WriteDoubleWord    ${SCB_CCR}    ${value}
    Execute Command    ${cmd}

*** Test Cases ***
SCB CCR DC Enable Bit Writable
    [Tags]    L2-state
    Start STM32N6
    # ARMv8-M SCB_CCR.DC (bit 16) enables the data cache. Verify it
    # is actually writable instead of the previous "int(val) >= 0"
    # check, which is true for any 32-bit unsigned read and never
    # verifies cache control actually works.
    #
    # NOTE: SCB_CCR.IC (bit 17, instruction cache) is intentionally
    # NOT exercised here -- Renode's Cortex-M55 CPU model logs
    # "Trying to enable instruction cache, but it's not supported"
    # and silently drops that bit, so a test asserting IC is set
    # would fail against the simulator itself, not against this
    # board's code.
    ${orig}=    Execute Command    sysbus ReadDoubleWord ${SCB_CCR}
    ${orig}=    Strip String    ${orig}
    ${orig}=    Convert To Integer    ${orig}
    ${enabled}=    Evaluate    ${orig} | 0x10000
    Write SCB CCR    ${enabled}
    ${val}=    Execute Command    sysbus ReadDoubleWord ${SCB_CCR}
    ${val}=    Strip String    ${val}
    ${val}=    Convert To Integer    ${val}
    Should Be True    (${val} & 0x10000) == 0x10000

Boot Regression
    [Tags]    boot-regression
    Start STM32N6
    Wait For NSH
