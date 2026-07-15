*** Settings ***
Resource        resources/stm32n6-common.robot

*** Variables ***
${HPDMA_BASE}   0x48020000

*** Keywords ***
Write HPDMA Register
    [Arguments]    ${offset}    ${value}
    ${addr}=    Evaluate    ${HPDMA_BASE} + ${offset}
    Execute Command    sysbus WriteDoubleWord ${addr} ${value}

*** Test Cases ***
HPDMA CH0 Full Mem2Mem Word Transfer
    [Tags]    L3-functional
    Create STM32N6 Machine
    # Set up source data in SRAM
    ${src}=    Evaluate    0x34001000
    ${dst}=    Evaluate    0x34002000
    Execute Command    sysbus WriteDoubleWord ${src} 0xCAFEBABE
    # Write second word at src+4
    ${src4}=    Evaluate    ${src} + 4
    Execute Command    sysbus WriteDoubleWord ${src4} 0xDEADBEEF
    # Set CSAR @ 0x60
    Write HPDMA Register    0x60    ${src}
    # Set CDAR @ 0x64
    Write HPDMA Register    0x64    ${dst}
    # Set BNDT @ 0x5C = 8 bytes (2 words)
    Write HPDMA Register    0x5C    0x8
    # Set CTR2 @ 0x58: SDW=word(2), SINC=1, DDW=word(2), DINC=1
    # = (2) | (1<<3) | (2<<16) | (1<<19) = 0x00080008
    Write HPDMA Register    0x58    0x00080008
    # Enable with TCIE: CC @ 0x54 = EN(1) | TCIE(256) = 0x101
    Write HPDMA Register    0x54    0x101
    # Verify destination data after transfer
    ${out0}=    Execute Command    sysbus ReadDoubleWord ${dst}
    ${out0}=    Strip String    ${out0}
    ${out0}=    Convert To Integer    ${out0}
    # Read second word at dst+4
    ${dst4}=    Evaluate    ${dst} + 4
    ${out4}=    Execute Command    sysbus ReadDoubleWord ${dst4}
    ${out4}=    Strip String    ${out4}
    ${out4}=    Convert To Integer    ${out4}
    Should Be Equal As Integers    ${out0}    0xCAFEBABE
    Should Be Equal As Integers    ${out4}    0xDEADBEEF

HPDMA CH0 TransferComplete IRQ
    [Tags]    L2-state
    Create STM32N6 Machine
    ${src}=    Evaluate    0x34003000
    ${dst}=    Evaluate    0x34004000
    Execute Command    sysbus WriteDoubleWord ${src} 0x12345678
    # Set CSAR @ 0x60
    Write HPDMA Register    0x60    ${src}
    # Set CDAR @ 0x64
    Write HPDMA Register    0x64    ${dst}
    # Set BNDT @ 0x5C = 4 bytes
    Write HPDMA Register    0x5C    0x4
    # Set CTR2 @ 0x58: word, SINC, DINC
    Write HPDMA Register    0x58    0x00080008
    # Enable with TCIE
    Write HPDMA Register    0x54    0x101
    # Check IRQ line
    ${irq}=    Execute Command    sysbus.hpdma1 IRQ IsSet
    Should Contain    ${irq}    True

HPDMA CH0 BNDT=0 No Transfer
    [Tags]    L2-state
    Create STM32N6 Machine
    # Set CSAR and CDAR but BNDT=0 should skip transfer
    Write HPDMA Register    0x60    0x34005000
    Write HPDMA Register    0x64    0x34006000
    Write HPDMA Register    0x5C    0x0
    Write HPDMA Register    0x54    0x101
    # CC should not have EN set (no transfer)
    ${cc}=    Execute Command    sysbus ReadDoubleWord 0x48020054
    ${cc}=    Strip String    ${cc}
    ${cc}=    Convert To Integer    ${cc}
    ${cc_and_1}=    Evaluate    ${cc} & 1
    Should Be Equal As Integers    ${cc_and_1}    0

Boot Regression
    [Tags]    boot-regression
    Start STM32N6
    Wait For NSH
