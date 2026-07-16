*** Settings ***
Resource        resources/stm32n6-common.robot
Force Tags      smoke

*** Test Cases ***
Boot To NSH
    [Tags]    boot-regression
    Start STM32N6
    Wait For NSH

Help Command Works
    [Tags]    L3-functional
    Start STM32N6
    Wait For NSH
    Run NSH Command    help    Builtin Apps:
