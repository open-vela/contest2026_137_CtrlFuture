*** Settings ***
Resource        resources/stm32n6-common.robot
Force Tags      adc

*** Variables ***
${ADC_BASE}     0x40022000

*** Keywords ***
Read ADC Register
    [Arguments]    ${offset}
    ${addr}=    Evaluate    ${ADC_BASE} + ${offset}
    ${val}=     Execute Command    sysbus ReadDoubleWord ${addr}
    ${val}=     Strip String    ${val}
    RETURN    ${val}

Write ADC Register
    [Arguments]    ${offset}    ${value}
    ${addr}=    Evaluate    ${ADC_BASE} + ${offset}
    Execute Command    sysbus WriteDoubleWord ${addr} ${value}

*** Test Cases ***
ADC1 CR ADEN Sets ADRDY
    [Tags]    L2-state
    Start STM32N6
    # CR @ 0x08, ADEN = bit 0 (CMSIS ADC_CR_ADEN_Pos = 0). A
    # previous revision of this test wrote 0x1 to CR and called it
    # "ADSTART" -- that is CMSIS ADC_CR_ADSTART_Pos = 2 (0x4), not
    # bit 0. Corrected: ADEN sets ISR.ADRDY (bit 0).
    Write ADC Register    0x08    0x00000001
    ${isr}=    Read ADC Register    0x00
    ${isr}=    Convert To Integer    ${isr}
    Should Be True    (${isr} & 0x1) == 0x1

ADC1 CFGR1 Configurable
    [Tags]    L2-state
    Start STM32N6
    # CFGR1 @ 0x0C
    Write ADC Register    0x0C    0x0000ABCD
    ${val}=    Read ADC Register    0x0C
    ${val}=    Convert To Integer    ${val}
    Should Be Equal As Integers    ${val}    0x0000ABCD

ADC1 IER Configurable
    [Tags]    L2-state
    Start STM32N6
    # IER @ 0x04 (was missing from the model entirely)
    Write ADC Register    0x04    0x00000003
    ${val}=    Read ADC Register    0x04
    ${val}=    Convert To Integer    ${val}
    Should Be Equal As Integers    ${val}    0x00000003

ADC1 JSQR Configurable
    [Tags]    L2-state
    Start STM32N6
    # JSQR @ 0x4C (was previously placed at the wrong offset 0x70)
    Write ADC Register    0x4C    0x00000021
    ${val}=    Read ADC Register    0x4C
    ${val}=    Convert To Integer    ${val}
    Should Be Equal As Integers    ${val}    0x00000021

ADC1 AWD1 Thresholds Configurable
    [Tags]    L2-state
    Start STM32N6
    # AWD1LTR @ 0xA8, AWD1HTR @ 0xAC (real CMSIS watchdog thresholds;
    # a previous revision of this model used fictitious TR1/TR2 at
    # 0x20/0x24, which is CMSIS reserved space)
    Write ADC Register    0xA8    0x00000100
    Write ADC Register    0xAC    0x00000FFF
    ${low}=    Read ADC Register    0xA8
    ${low}=    Convert To Integer    ${low}
    ${high}=    Read ADC Register    0xAC
    ${high}=    Convert To Integer    ${high}
    Should Be Equal As Integers    ${low}    0x00000100
    Should Be Equal As Integers    ${high}    0x00000FFF

ADC1 SQR1 Sequence Register
    [Tags]    L2-state
    Start STM32N6
    # SQR1 @ 0x30
    Write ADC Register    0x30    0x00000010
    ${val}=    Read ADC Register    0x30
    ${val}=    Convert To Integer    ${val}
    Should Be Equal As Integers    ${val}    0x00000010

ADC1 DR Reads Zero Before Any Conversion
    [Tags]    L2-state
    Start STM32N6
    # DR @ 0x40 is read-only; before any ADSTART-triggered
    # conversion it reads back the reset value 0. Direct writes are
    # ignored (FieldMode.Read).
    Write ADC Register    0x40    0x00001234
    ${val}=    Read ADC Register    0x40
    ${val}=    Convert To Integer    ${val}
    Should Be Equal As Integers    ${val}    0

ADC1 SIMDR Injects Simulated Sample
    [Tags]    L1-register
    Start STM32N6
    # SIMDR @ 0x200: team-defined test-only register (not CMSIS,
    # see STM32N6_ADC.cs header). Plain read/write round-trip.
    Write ADC Register    0x200    0x00000ABC
    ${val}=    Read ADC Register    0x200
    ${val}=    Convert To Integer    ${val}
    Should Be Equal As Integers    ${val}    0x00000ABC

ADC1 ADSTART Conversion Returns Injected Sample
    [Tags]    L3-functional
    Create STM32N6 Machine
    # Inject a simulated analog sample via SIMDR, enable the ADC
    # (ADEN), then trigger a regular conversion (ADSTART) and
    # verify DR returns exactly the injected value -- i.e. the
    # "ADSTART -> wait for EOC -> read DR" driver sequence a real
    # NuttX ADC driver would use is fully exercised end-to-end,
    # not just individual register bits.
    Write ADC Register    0x200    0x00000777
    Write ADC Register    0x08    0x1
    # ADSTART = bit 2 (0x4)
    Write ADC Register    0x08    0x4
    ${dr}=    Read ADC Register    0x40
    ${dr}=    Convert To Integer    ${dr}
    Should Be Equal As Integers    ${dr}    0x00000777

ADC1 ADSTART Sets EOC
    [Tags]    L3-functional
    Create STM32N6 Machine
    Write ADC Register    0x200    0x00000042
    Write ADC Register    0x08    0x1
    Write ADC Register    0x08    0x4
    ${isr}=    Read ADC Register    0x00
    ${isr}=    Convert To Integer    ${isr}
    # EOC = bit 2 (0x4)
    Should Be True    (${isr} & 0x4) == 0x4

ADC1 EOCIE Raises IRQ On Conversion Complete
    [Tags]    L3-functional
    Create STM32N6 Machine
    # EOCIE = bit 2 (0x4)
    Write ADC Register    0x04    0x4
    Write ADC Register    0x200    0x00000055
    Write ADC Register    0x08    0x1
    Write ADC Register    0x08    0x4
    ${irq}=    Execute Command    sysbus.adc1 IRQ IsSet
    Should Contain    ${irq}    True

ADC1 ADSTART Without EOCIE Does Not Raise IRQ
    [Tags]    L3-functional
    Create STM32N6 Machine
    Write ADC Register    0x200    0x00000099
    Write ADC Register    0x08    0x1
    Write ADC Register    0x08    0x4
    ${irq}=    Execute Command    sysbus.adc1 IRQ IsSet
    Should Contain    ${irq}    False

ADC1 ADSTART Without ADEN Does Not Convert
    [Tags]    L3-functional
    Create STM32N6 Machine
    # ADSTART written without ADEN first must not trigger a
    # conversion (mirrors real hardware: ADSTART is only effective
    # when the ADC is enabled and ready).
    Write ADC Register    0x200    0x000000AA
    Write ADC Register    0x08    0x4
    ${dr}=    Read ADC Register    0x40
    ${dr}=    Convert To Integer    ${dr}
    Should Be Equal As Integers    ${dr}    0

Boot Regression
    [Tags]    boot-regression
    Start STM32N6
    Wait For NSH
