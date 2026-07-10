*** Variables ***
${MODEL_ADDR}    0x50000000

*** Keywords ***
Create Test Machine
    Execute Command                 mach create
    Execute Command                 machine LoadPlatformDescriptionFromString "testdev: Miscellaneous.TestPeripheral @ sysbus ${MODEL_ADDR}"

*** Test Cases ***
Register Reset Value
    Create Test Machine
    ${val}=    Execute Command    sysbus ReadDoubleWord ${MODEL_ADDR}
    Should Be Equal As Integers    ${val}    0xABCD1234

Register Read Write
    Create Test Machine
    Execute Command                 sysbus WriteDoubleWord ${MODEL_ADDR} 0x12345678
    ${val}=    Execute Command    sysbus ReadDoubleWord ${MODEL_ADDR}
    Should Be Equal As Integers    ${val}    0x12345678
