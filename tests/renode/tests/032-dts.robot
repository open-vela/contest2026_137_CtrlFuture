*** Settings ***
Resource        resources/stm32n6-common.robot
Force Tags      dts

*** Variables ***
${DTS_BASE}     0x4600A000

*** Keywords ***
Read DTS Register
    [Arguments]    ${offset}
    ${addr}=    Evaluate    ${DTS_BASE} + ${offset}
    ${val}=     Execute Command    sysbus ReadDoubleWord ${addr}
    ${val}=     Strip String    ${val}
    RETURN    ${val}

Write DTS Register
    [Arguments]    ${offset}    ${value}
    ${addr}=    Evaluate    ${DTS_BASE} + ${offset}
    Execute Command    sysbus WriteDoubleWord ${addr} ${value}

*** Test Cases ***
DTS CFGR1 Reset Value
    [Tags]    L1-register
    Start STM32N6
    ${val}=    Read DTS Register    0x00
    Should Be Equal As Integers    ${val}    0

DTS CFGR2 Writable
    [Tags]    L1-register
    Start STM32N6
    Write DTS Register    0x04    0x01
    ${val}=    Read DTS Register    0x04
    Should Be Equal As Integers    ${val}    0x01

DTS TSLPTR Writable
    [Tags]    L1-register
    Start STM32N6
    # NOTE: this is a team-defined placeholder register (see
    # STM32N6_DTS.cs header); write/read round-trip instead of the
    # previous "int(val) >= 0" check on the read-only T0VALR1
    # register, which is true for any 32-bit unsigned read.
    Write DTS Register    0x14    0x12345678
    ${val}=    Read DTS Register    0x14
    ${val}=    Convert To Integer    ${val}
    Should Be Equal As Integers    ${val}    0x12345678

DTS TEMPHYSR Reads Zero Before Any Measurement
    [Tags]    L2-state
    Start STM32N6
    # TEMPHYSR @ 0x18: before CFGR1.START is ever triggered, both
    # the sample bits [30:0] and the VALID bit [31] must read 0.
    ${val}=    Read DTS Register    0x18
    Should Be Equal As Integers    ${val}    0

DTS SIMTEMPR Injects Simulated Temperature
    [Tags]    L1-register
    Start STM32N6
    # SIMTEMPR @ 0x1F8: team-defined test-only register (not
    # CMSIS, see STM32N6_DTS.cs header). Plain read/write
    # round-trip.
    Write DTS Register    0x1F8    0x00001234
    ${val}=    Read DTS Register    0x1F8
    ${val}=    Convert To Integer    ${val}
    Should Be Equal As Integers    ${val}    0x00001234

DTS CFGR1 START Loads TEMPHYSR With Injected Sample
    [Tags]    L3-functional
    Create STM32N6 Machine
    # Inject a simulated temperature sample, then trigger a
    # measurement (CFGR1.START, bit 0) and verify TEMPHYSR returns
    # exactly that value with VALID set -- i.e. the
    # "inject -> start -> wait -> read" sequence a real DTS driver
    # would use is exercised end-to-end.
    Write DTS Register    0x1F8    0x00000019
    Write DTS Register    0x00    0x1
    ${temphysr}=    Read DTS Register    0x18
    ${temphysr}=    Convert To Integer    ${temphysr}
    # bits [30:0] = sample, bit 31 = VALID
    ${sample}=    Evaluate    ${temphysr} & 0x7FFFFFFF
    ${valid}=    Evaluate    (${temphysr} >> 31) & 0x1
    Should Be Equal As Integers    ${sample}    0x19
    Should Be Equal As Integers    ${valid}    1

DTS CFGR1 START Without Prior Injection Reads Zero
    [Tags]    L3-functional
    Create STM32N6 Machine
    # Triggering START without ever writing SIMTEMPR must load the
    # reset value (0), not some stale or undefined value -- proves
    # the model does not silently retain garbage across a fresh
    # Create STM32N6 Machine.
    Write DTS Register    0x00    0x1
    ${temphysr}=    Read DTS Register    0x18
    ${temphysr}=    Convert To Integer    ${temphysr}
    ${sample}=    Evaluate    ${temphysr} & 0x7FFFFFFF
    Should Be Equal As Integers    ${sample}    0

DTS Second Measurement Overwrites First Sample
    [Tags]    L3-functional
    Create STM32N6 Machine
    Write DTS Register    0x1F8    0x00000032
    Write DTS Register    0x00    0x1
    Write DTS Register    0x1F8    0x00000064
    Write DTS Register    0x00    0x1
    ${temphysr}=    Read DTS Register    0x18
    ${temphysr}=    Convert To Integer    ${temphysr}
    ${sample}=    Evaluate    ${temphysr} & 0x7FFFFFFF
    Should Be Equal As Integers    ${sample}    0x64

Boot Regression
    [Tags]    boot-regression
    Start STM32N6
    Wait For NSH
