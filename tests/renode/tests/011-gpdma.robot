*** Settings ***
Resource        resources/stm32n6-common.robot
# Tags: L1-register | L2-state | L3-functional | boot-regression
Force Tags      gpdma

*** Variables ***
${GPDMA_BASE}   0x40021000

*** Keywords ***
Read GPDMA Register
    [Arguments]    ${offset}
    ${addr}=    Evaluate    ${GPDMA_BASE} + ${offset}
    ${val}=     Execute Command    sysbus ReadDoubleWord ${addr}
    ${val}=     Strip String    ${val}
    RETURN    ${val}

Write GPDMA Register
    [Arguments]    ${offset}    ${value}
    ${addr}=    Evaluate    ${GPDMA_BASE} + ${offset}
    Execute Command    sysbus WriteDoubleWord ${addr} ${value}

*** Test Cases ***
GPDMA SECCFGR Accessible
    [Tags]    L1-register
    Start STM32N6
    # SECCFGR @ 0x00 (global register)
    ${val}=    Read GPDMA Register    0x00
    Should Be True    int(${val}) >= 0

GPDMA Channel 0 CCR Accessible
    [Tags]    L1-register
    Start STM32N6
    # Channel 0: base=0x50, CCR=base+0x14=0x64
    ${val}=    Read GPDMA Register    0x64
    Should Be True    int(${val}) >= 0

GPDMA Channel 0 CSR Accessible
    [Tags]    L1-register
    Start STM32N6
    # Channel 0: base=0x50, CSR=base+0x10=0x60
    ${val}=    Read GPDMA Register    0x60
    Should Be True    int(${val}) >= 0

GPDMA Channel 0 CLBAR Writable
    [Tags]    L1-register
    Start STM32N6
    # Channel 0: base=0x50, CLBAR=base+0x00=0x50
    Write GPDMA Register    0x50    0x20000000
    ${val}=    Read GPDMA Register    0x50
    Should Be Equal As Integers    ${val}    0x20000000

GPDMA Channel 1 CLBAR Writable
    [Tags]    L1-register
    Start STM32N6
    # Channel 1: base=0x50+0x80=0xD0, CLBAR=0xD0
    Write GPDMA Register    0xD0    0x34000000
    ${val}=    Read GPDMA Register    0xD0
    Should Be Equal As Integers    ${val}    0x34000000

GPDMA TCF Clear Via CFCR
    [Tags]    L2-state
    Start STM32N6
    ${SRC}=    Set Variable    0x34001000
    ${DST}=    Set Variable    0x34002000
    Execute Command    sysbus WriteDoubleWord ${SRC} 0x11223344
    Write GPDMA Register    0x9C    ${SRC}
    Write GPDMA Register    0xA0    ${DST}
    Write GPDMA Register    0x98    0x4
    # CTR1: byte SINC|DINC = bit3|bit19 = 0x00080008
    Write GPDMA Register    0x90    0x00080008
    Write GPDMA Register    0x94    0x200
    Write GPDMA Register    0x64    0x1
    ${csr}=    Read GPDMA Register    0x60
    ${csr}=    Convert To Integer    ${csr}
    Should Be True    (${csr} & 0x100) == 0x100
    # CFCR TCFC bit8 W1C
    Write GPDMA Register    0x5C    0x100
    ${csr2}=    Read GPDMA Register    0x60
    ${csr2}=    Convert To Integer    ${csr2}
    Should Be True    (${csr2} & 0x100) == 0x0

GPDMA Mem2Mem Word Copy
    [Tags]    L3-functional
    Start STM32N6
    # Use SRAM region mapped in platform (0x34000000)
    ${SRC}=    Set Variable    0x34001000
    ${DST}=    Set Variable    0x34002000
    Execute Command    sysbus WriteDoubleWord ${SRC} 0xA5A5A5A5
    ${src4}=    Evaluate    ${SRC} + 4
    Execute Command    sysbus WriteDoubleWord ${src4} 0x5A5A5A5A
    # Program ch0: CSAR, CDAR, CBR1=8 bytes
    Write GPDMA Register    0x9C    ${SRC}
    Write GPDMA Register    0xA0    ${DST}
    Write GPDMA Register    0x98    0x8
    # CTR1: SDW=2, SINC, DDW=2, DINC
    # = 2 | (1<<3) | (2<<16) | (1<<19) = 0x000A000A
    Write GPDMA Register    0x90    0x000A000A
    # CTR2 SWREQ bit9
    Write GPDMA Register    0x94    0x200
    # CCR EN
    Write GPDMA Register    0x64    0x1
    # CSR TCF bit8
    ${csr}=    Read GPDMA Register    0x60
    ${csr}=    Convert To Integer    ${csr}
    Should Be True    (${csr} & 0x100) == 0x100
    # EN auto-cleared after complete
    ${ccr}=    Read GPDMA Register    0x64
    ${ccr}=    Convert To Integer    ${ccr}
    Should Be True    (${ccr} & 0x1) == 0x0
    ${d0}=    Execute Command    sysbus ReadDoubleWord ${DST}
    ${d0}=    Strip String    ${d0}
    ${dst4}=    Evaluate    ${DST} + 4
    ${d1}=    Execute Command    sysbus ReadDoubleWord ${dst4}
    ${d1}=    Strip String    ${d1}
    Should Be Equal As Integers    ${d0}    0xA5A5A5A5
    Should Be Equal As Integers    ${d1}    0x5A5A5A5A
    # BNDT decremented to 0
    ${bndt}=    Read GPDMA Register    0x98
    Should Be Equal As Integers    ${bndt}    0

Boot Regression
    [Tags]    boot-regression
    Start STM32N6
    Wait For NSH
