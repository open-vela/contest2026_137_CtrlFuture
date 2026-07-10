//
// Copyright (c) 2026 CtrlFuture
//
// SPDX-License-Identifier: Apache-2.0
//
// Test model for Renode build validation.
// Single RW register at offset 0x00, reset value 0xABCD1234.
//

using Antmicro.Renode.Core;
using Antmicro.Renode.Core.Structure.Registers;
using Antmicro.Renode.Peripherals.Bus;

namespace Antmicro.Renode.Peripherals.Miscellaneous
{
    public class TestPeripheral : BasicDoubleWordPeripheral, IKnownSize
    {
        public TestPeripheral(IMachine machine) : base(machine)
        {
            DefineRegisters();
        }

        public long Size => 0x100;

        private void DefineRegisters()
        {
            Registers.TestReg.Define(this, 0xABCD1234)
                .WithValueField(0, 32, name: "TEST_REG");
        }

        private enum Registers : long
        {
            TestReg = 0x00,
        }
    }
}
