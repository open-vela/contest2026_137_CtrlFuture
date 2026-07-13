//
// Copyright (c) 2026 CtrlFuture
//
// SPDX-License-Identifier: Apache-2.0
//
// STM32N6 Neural-ART NPU model for Renode.
// Minimal model: register read/write without actual neural processing.
//
// Registers:
//   CR  @ 0x00: Control Register
//   SR  @ 0x04: Status Register
//   IER @ 0x08: Interrupt Enable Register
//   VER @ 0x0C: Version Register
//   CFG @ 0x10: Configuration Register
//

using Antmicro.Renode.Core;
using Antmicro.Renode.Core.Structure.Registers;
using Antmicro.Renode.Logging;
using Antmicro.Renode.Peripherals.Bus;

namespace Antmicro.Renode.Peripherals.Miscellaneous
{
    public class STM32N6_NPU : BasicDoubleWordPeripheral, IKnownSize
    {
        public STM32N6_NPU(IMachine machine) : base(machine)
        {
            DefineRegisters();
        }

        public long Size => 0x1000;

        private void DefineRegisters()
        {
            // CR @ 0x00: Control Register
            Registers.CR.Define(this)
                .WithValueField(0, 32, name: "CR");

            // SR @ 0x04: Status Register (read-only)
            Registers.SR.Define(this)
                .WithValueField(0, 32, FieldMode.Read, name: "SR");

            // IER @ 0x08: Interrupt Enable Register
            Registers.IER.Define(this)
                .WithValueField(0, 32, name: "IER");

            // VER @ 0x0C: Version Register (read-only)
            Registers.VER.Define(this)
                .WithValueField(0, 32, FieldMode.Read, name: "VER");

            // CFG @ 0x10: Configuration Register
            Registers.CFG.Define(this)
                .WithValueField(0, 32, name: "CFG");
        }

        private enum Registers : long
        {
            CR  = 0x00,
            SR  = 0x04,
            IER = 0x08,
            VER = 0x0C,
            CFG = 0x10,
        }
    }
}
