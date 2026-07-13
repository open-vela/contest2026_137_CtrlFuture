//
// Copyright (c) 2026 CtrlFuture
//
// SPDX-License-Identifier: Apache-2.0
//
// STM32N6 CSI-2 MIPI Interface model for Renode.
// Minimal model: register read/write without actual camera input.
//
// Registers:
//   CR  @ 0x00: Configuration Register
//   SR  @ 0x04: Status Register
//   IER @ 0x08: Interrupt Enable Register
//   IFR @ 0x0C: Interrupt Flag Register
//

using Antmicro.Renode.Core;
using Antmicro.Renode.Core.Structure.Registers;
using Antmicro.Renode.Logging;
using Antmicro.Renode.Peripherals.Bus;

namespace Antmicro.Renode.Peripherals.Miscellaneous
{
    public class STM32N6_CSI : BasicDoubleWordPeripheral, IKnownSize
    {
        public STM32N6_CSI(IMachine machine) : base(machine)
        {
            DefineRegisters();
        }

        public long Size => 0x400;

        private void DefineRegisters()
        {
            // CR @ 0x00: Configuration Register
            Registers.CR.Define(this)
                .WithValueField(0, 32, name: "CR");

            // SR @ 0x04: Status Register (read-only)
            Registers.SR.Define(this)
                .WithValueField(0, 32, FieldMode.Read, name: "SR");

            // IER @ 0x08: Interrupt Enable Register
            Registers.IER.Define(this)
                .WithValueField(0, 32, name: "IER");

            // IFR @ 0x0C: Interrupt Flag Register (write-to-clear)
            Registers.IFR.Define(this)
                .WithValueField(0, 32, name: "IFR");
        }

        private enum Registers : long
        {
            CR  = 0x00,
            SR  = 0x04,
            IER = 0x08,
            IFR = 0x0C,
        }
    }
}
