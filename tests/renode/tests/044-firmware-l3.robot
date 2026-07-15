*** Settings ***
Resource        resources/stm32n6-common.robot
Force Tags      firmware-l3
Documentation   Contest-local firmware L3 tests (Approach B)
...             Phase-2 cmocka drivertest deferred: contest isolation + binary size limit

*** Test Cases ***
Firmware UART Help Is Driver L3
    [Tags]    L3-functional
    Start STM32N6
    Wait For NSH
    Run NSH Command    help    Builtin Apps:

Firmware Hello Builtin
    [Tags]    L3-functional
    Start STM32N6
    Wait For NSH
    Run NSH Command    hello    Hello, World!!

Boot Regression
    [Tags]    boot-regression
    Start STM32N6
    Wait For NSH
