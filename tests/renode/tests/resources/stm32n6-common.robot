*** Variables ***
${UART}         sysbus.usart1
${PLATFORM}     ${CURDIR}/../../stm32n647x0.repl
${ELF}          %{NUTTX_ELF=${CURDIR}/../../../../nuttx/nuttx}

*** Keywords ***
Create STM32N6 Machine
    Execute Command                 mach create
    Execute Command                 machine LoadPlatformDescription "${PLATFORM}"
    Execute Command                 sysbus LoadELF "${ELF}"

Create STM32N6 UART Tester
    Create Terminal Tester          ${UART}

Start STM32N6
    Create STM32N6 Machine
    Create STM32N6 UART Tester
    Start Emulation

Wait For NSH
    Wait For Prompt On Uart    nsh>    timeout=30

Run NSH Command
    [Arguments]    ${cmd}    ${expected}    ${timeout}=5
    Write Line To Uart         ${cmd}
    Wait For Line On Uart      ${expected}    timeout=${timeout}

*** Keywords ***
Read NVIC ISPRn Pending
    [Documentation]    Read NVIC ISPR word for IRQn (CMSIS number).
    ...                ISPR0 @ 0xE000E200 covers IRQn 0-31; ISPR1 @ +4, etc.
    [Arguments]    ${irqn}
    ${word}=    Evaluate    int(${irqn}) // 32
    ${bit}=     Evaluate    int(${irqn}) % 32
    ${addr}=    Evaluate    0xE000E200 + (${word} * 4)
    ${val}=     Execute Command    sysbus ReadDoubleWord ${addr}
    ${val}=     Strip String    ${val}
    ${pending}=    Evaluate    (int(${val}) >> ${bit}) & 1
    RETURN    ${pending}

Assert NVIC Pending
    [Arguments]    ${irqn}
    ${p}=    Read NVIC ISPRn Pending    ${irqn}
    Should Be Equal As Integers    ${p}    1

Assert NVIC Not Pending
    [Arguments]    ${irqn}
    ${p}=    Read NVIC ISPRn Pending    ${irqn}
    Should Be Equal As Integers    ${p}    0
