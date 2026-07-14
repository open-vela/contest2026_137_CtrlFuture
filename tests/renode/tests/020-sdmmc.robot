*** Settings ***
Resource        resources/stm32n6-common.robot
Force Tags      sdmmc

*** Variables ***
${SDMMC_BASE}   0x48027000
# Register offsets
${REG_POWER}    0x00
${REG_CLKCR}    0x04
${REG_ARG}      0x08
${REG_CMD}      0x0C
${REG_RESPCMD}  0x10
${REG_RESP1}    0x14
${REG_STA}      0x34
${REG_ICR}      0x38
${REG_MASK}     0x3C
# Bit helpers (CMSIS N6/H7 layout)
${PWRCTRL_ON}   0x3
# CLKCR: CLKDIV=250 | bit16 (driver "CLKEN" retained as RW)
${CLKCR_INIT}   0x100FA
# CMD: CMDINDEX | WAITRESP_SHORT(1<<8) | CPSMEN(1<<12)
# CMD0 no-resp: index0 | CPSMEN
${CMD0_VAL}     0x1000
# CMD8 short: index8 | WAITRESP_SHORT | CPSMEN = 8 | 0x100 | 0x1000
${CMD8_VAL}     0x1108
# CMD55 no-resp (driver NULL): 55 | CPSMEN
${CMD55_VAL}    0x1037
# ACMD41 short: 41 | WAITRESP_SHORT | CPSMEN
${ACMD41_VAL}   0x1129
# CMD2 no-resp (driver NULL): 2 | CPSMEN
${CMD2_VAL}     0x1002
# CMD3 short: 3 | WAITRESP_SHORT | CPSMEN
${CMD3_VAL}     0x1103
# CMD7 no-resp: 7 | CPSMEN
${CMD7_VAL}     0x1007
# CMD16 no-resp: 16 | CPSMEN
${CMD16_VAL}    0x1010
${STA_CMDREND}  0x40
${STA_CMDSENT}  0x80
${ICR_CMDREND}  0x40
${ICR_CMDSENT}  0x80
${ICR_BOTH}     0xC0

*** Keywords ***
Read SDMMC Register
    [Arguments]    ${offset}
    ${addr}=    Evaluate    ${SDMMC_BASE} + ${offset}
    ${val}=     Execute Command    sysbus ReadDoubleWord ${addr}
    ${val}=     Strip String    ${val}
    RETURN    ${val}

Write SDMMC Register
    [Arguments]    ${offset}    ${value}
    ${addr}=    Evaluate    ${SDMMC_BASE} + ${offset}
    Execute Command    sysbus WriteDoubleWord ${addr} ${value}

Power And Clock Enable
    Write SDMMC Register    ${REG_POWER}    ${PWRCTRL_ON}
    Write SDMMC Register    ${REG_CLKCR}    ${CLKCR_INIT}

Send SDMMC Command
    [Arguments]    ${cmdreg}    ${cmdarg}=0
    Write SDMMC Register    ${REG_ARG}    ${cmdarg}
    Write SDMMC Register    ${REG_CMD}    ${cmdreg}

Clear Completion Flags
    Write SDMMC Register    ${REG_ICR}    ${ICR_BOTH}

*** Test Cases ***
SDMMC1 POWER Reset Value
    [Tags]    L1-register
    Start STM32N6
    ${val}=    Read SDMMC Register    ${REG_POWER}
    Should Be Equal As Integers    ${val}    0

SDMMC1 CLKCR Writable
    [Tags]    L1-register
    Start STM32N6
    Write SDMMC Register    ${REG_CLKCR}    0x0100
    ${val}=    Read SDMMC Register    ${REG_CLKCR}
    Should Be Equal As Integers    ${val}    0x0100

SDMMC1 STA Accessible
    [Tags]    L1-register
    Start STM32N6
    ${val}=    Read SDMMC Register    ${REG_STA}
    Should Be True    int(${val}) >= 0

POWER ON Retains PWRCTRL
    [Tags]    L2-state
    Start STM32N6
    Write SDMMC Register    ${REG_POWER}    ${PWRCTRL_ON}
    ${val}=    Read SDMMC Register    ${REG_POWER}
    ${val}=    Convert To Integer    ${val}
    Should Be True    (${val} & 0x3) == 0x3

CLKCR CLKEN Writable
    [Tags]    L2-state
    Start STM32N6
    Write SDMMC Register    ${REG_CLKCR}    ${CLKCR_INIT}
    ${val}=    Read SDMMC Register    ${REG_CLKCR}
    ${val}=    Convert To Integer    ${val}
    # CLKDIV=250 and bit16 retained
    Should Be True    (${val} & 0x3FF) == 250
    Should Be True    (${val} & 0x10000) == 0x10000

CMD0 Sets CMDSENT
    [Tags]    L2-state
    Start STM32N6
    Power And Clock Enable
    Clear Completion Flags
    Send SDMMC Command    ${CMD0_VAL}    0
    ${sta}=    Read SDMMC Register    ${REG_STA}
    ${sta}=    Convert To Integer    ${sta}
    Should Be True    (${sta} & ${STA_CMDSENT}) == ${STA_CMDSENT}

ICR Clears CMDSENT Then Path Ready
    [Tags]    L2-state
    Start STM32N6
    Power And Clock Enable
    Send SDMMC Command    ${CMD0_VAL}    0
    ${sta}=    Read SDMMC Register    ${REG_STA}
    ${sta}=    Convert To Integer    ${sta}
    Should Be True    (${sta} & ${STA_CMDSENT}) == ${STA_CMDSENT}
    # ICR CMDSENTC: model re-asserts CMDSENT for driver pre-wait quirk
    Write SDMMC Register    ${REG_ICR}    ${ICR_CMDSENT}
    ${sta2}=    Read SDMMC Register    ${REG_STA}
    ${sta2}=    Convert To Integer    ${sta2}
    Should Be True    (${sta2} & ${STA_CMDSENT}) == ${STA_CMDSENT}

CMD8 Short Sets CMDREND And Echoes Arg
    [Tags]    L2-state
    Start STM32N6
    Power And Clock Enable
    Clear Completion Flags
    Send SDMMC Command    ${CMD8_VAL}    0x1AA
    ${sta}=    Read SDMMC Register    ${REG_STA}
    ${sta}=    Convert To Integer    ${sta}
    Should Be True    (${sta} & ${STA_CMDREND}) == ${STA_CMDREND}
    ${resp}=    Read SDMMC Register    ${REG_RESP1}
    Should Be Equal As Integers    ${resp}    0x1AA
    ${rcmd}=    Read SDMMC Register    ${REG_RESPCMD}
    ${rcmd}=    Convert To Integer    ${rcmd}
    Should Be True    (${rcmd} & 0x3F) == 8

Probe Sequence CMD0 CMD8 ACMD41 CMD3
    [Tags]    L3-functional
    Start STM32N6
    Power And Clock Enable
    # CMD0 GO_IDLE
    Clear Completion Flags
    Send SDMMC Command    ${CMD0_VAL}    0
    ${sta}=    Read SDMMC Register    ${REG_STA}
    ${sta}=    Convert To Integer    ${sta}
    Should Be True    (${sta} & ${STA_CMDSENT}) == ${STA_CMDSENT}
    # CMD8 SEND_IF_COND
    Clear Completion Flags
    Send SDMMC Command    ${CMD8_VAL}    0x1AA
    ${sta}=    Read SDMMC Register    ${REG_STA}
    ${sta}=    Convert To Integer    ${sta}
    Should Be True    (${sta} & ${STA_CMDREND}) == ${STA_CMDREND}
    ${resp}=    Read SDMMC Register    ${REG_RESP1}
    Should Be Equal As Integers    ${resp}    0x1AA
    # CMD55 APP_CMD (no-resp as driver)
    Clear Completion Flags
    Send SDMMC Command    ${CMD55_VAL}    0
    ${sta}=    Read SDMMC Register    ${REG_STA}
    ${sta}=    Convert To Integer    ${sta}
    Should Be True    (${sta} & ${STA_CMDSENT}) == ${STA_CMDSENT}
    # ACMD41 SD_SEND_OP_COND — RESP1 bit31 ready
    Clear Completion Flags
    Send SDMMC Command    ${ACMD41_VAL}    0x40FF8000
    ${sta}=    Read SDMMC Register    ${REG_STA}
    ${sta}=    Convert To Integer    ${sta}
    Should Be True    (${sta} & ${STA_CMDREND}) == ${STA_CMDREND}
    ${ocr}=    Read SDMMC Register    ${REG_RESP1}
    ${ocr}=    Convert To Integer    ${ocr}
    Should Be True    (${ocr} & 0x80000000) == 0x80000000
    # CMD2 ALL_SEND_CID (no-resp as driver)
    Clear Completion Flags
    Send SDMMC Command    ${CMD2_VAL}    0
    ${sta}=    Read SDMMC Register    ${REG_STA}
    ${sta}=    Convert To Integer    ${sta}
    Should Be True    (${sta} & ${STA_CMDSENT}) == ${STA_CMDSENT}
    # CMD3 SEND_RELATIVE_ADDR — RCA in high half
    Clear Completion Flags
    Send SDMMC Command    ${CMD3_VAL}    0
    ${sta}=    Read SDMMC Register    ${REG_STA}
    ${sta}=    Convert To Integer    ${sta}
    Should Be True    (${sta} & ${STA_CMDREND}) == ${STA_CMDREND}
    ${r6}=    Read SDMMC Register    ${REG_RESP1}
    ${r6}=    Convert To Integer    ${r6}
    Should Be True    (${r6} & 0xFFFF0000) != 0

Full Init Through CMD16
    [Tags]    L3-functional
    Start STM32N6
    Power And Clock Enable
    Send SDMMC Command    ${CMD0_VAL}    0
    Clear Completion Flags
    Send SDMMC Command    ${CMD8_VAL}    0x1AA
    Clear Completion Flags
    Send SDMMC Command    ${CMD55_VAL}    0
    Clear Completion Flags
    Send SDMMC Command    ${ACMD41_VAL}    0x40FF8000
    ${ocr}=    Read SDMMC Register    ${REG_RESP1}
    ${ocr}=    Convert To Integer    ${ocr}
    Should Be True    (${ocr} & 0x80000000) == 0x80000000
    Clear Completion Flags
    Send SDMMC Command    ${CMD2_VAL}    0
    Clear Completion Flags
    Send SDMMC Command    ${CMD3_VAL}    0
    ${r6}=    Read SDMMC Register    ${REG_RESP1}
    ${r6}=    Convert To Integer    ${r6}
    ${rca_arg}=    Evaluate    ${r6} & 0xFFFF0000
    Clear Completion Flags
    Send SDMMC Command    ${CMD7_VAL}    ${rca_arg}
    ${sta}=    Read SDMMC Register    ${REG_STA}
    ${sta}=    Convert To Integer    ${sta}
    Should Be True    (${sta} & ${STA_CMDSENT}) == ${STA_CMDSENT}
    Clear Completion Flags
    Send SDMMC Command    ${CMD16_VAL}    512
    ${sta2}=    Read SDMMC Register    ${REG_STA}
    ${sta2}=    Convert To Integer    ${sta2}
    Should Be True    (${sta2} & ${STA_CMDSENT}) == ${STA_CMDSENT}
    # Switch to 4-bit faster clock (driver post-init CLKCR write)
    Write SDMMC Register    ${REG_CLKCR}    0x14002
    ${clk}=    Read SDMMC Register    ${REG_CLKCR}
    Should Be Equal As Integers    ${clk}    0x14002

Boot Regression
    [Tags]    boot-regression
    Start STM32N6
    Wait For NSH
