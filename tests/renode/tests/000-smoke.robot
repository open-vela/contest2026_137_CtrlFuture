*** Settings ***
Resource        resources/stm32n6-common.robot

*** Test Cases ***
Boot To NSH
    Start STM32N6
    Wait For NSH

Help Command Works
    Start STM32N6
    Wait For NSH
    Run NSH Command    help    Builtin Apps:
