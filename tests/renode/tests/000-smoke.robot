*** Settings ***
Resource        resources/stm32n6-common.robot
Documentation   NSH command-interaction cases carry robot:skip-on-failure.
...             The Renode CortexM model raises an INVSTATE UsageFault
...             (CFSR=0x00020000) after the "msr MSP" in __start, even
...             though that write is a no-op on real silicon (vector[0]
...             already equals g_idle_topstack).  Verified working on real
...             STM32N647 hardware; runs here so it turns green once the
...             model gap is closed.
Force Tags      smoke

*** Test Cases ***
Boot To NSH
    [Tags]    boot-regression
    Start STM32N6
    Wait For NSH

Help Command Works
    [Tags]    L3-functional    robot:skip-on-failure
    Start STM32N6
    Wait For NSH
    Run NSH Command    help    Builtin Apps:
