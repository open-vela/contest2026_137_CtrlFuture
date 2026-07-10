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
