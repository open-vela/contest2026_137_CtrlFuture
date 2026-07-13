//
// Copyright (c) 2026 CtrlFuture
//
// SPDX-License-Identifier: Apache-2.0
//
// STM32N6 OTP (One-Time Programmable Memory) model for Renode.
// Minimal model: register read/write without actual OTP operations.
//
// Registers:
//   CR  @ 0x00: Control Register
//   SR  @ 0x04: Status Register
//   AR  @ 0x08: Address Register
//   DR  @ 0x0C: Data Register
//

using Antmicro.Renode.Core;
using Antmicro.Renode.Core.Structure.Registers;
using Antmicro.Renode.Logging;
using Antmicro.Renode.Peripherals.Bus;

namespace Antmicro.Renode.Peripherals.Miscellaneous
{
    public class STM32N6_OTP : BasicDoubleWordPeripheral, IKnownSize
    {
        public STM32N6_OTP(IMachine machine) : base(machine)
        {
            DefineRegisters();
        }

        public long Size => 0x400;

        private void DefineRegisters()
        {
            // CR @ 0x00: Control Register
            Registers.CR.Define(this)
                .WithValueField(0, 32, name: "CR");

            // SR @ 0x04: Status Register (read-only)
            Registers.SR.Define(this)
                .WithValueField(0, 32, FieldMode.Read, name: "SR");

            // AR @ 0x08: Address Register
            Registers.AR.Define(this)
                .WithValueField(0, 32, name: "AR");

            // DR @ 0x0C: Data Register
            Registers.DR.Define(this)
                .WithValueField(0, 32, name: "DR");
        }

        private enum Registers : long
        {
            CR  = 0x00,
            SR  = 0x04,
            AR  = 0x08,
            DR  = 0x0C,
        }
    }
}
