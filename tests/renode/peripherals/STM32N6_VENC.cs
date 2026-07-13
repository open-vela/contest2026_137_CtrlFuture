//
// Copyright (c) 2026 CtrlFuture
//
// SPDX-License-Identifier: Apache-2.0
//
// STM32N6 H.264 Video Encoder (VENC) model for Renode.
// Minimal model: register read/write without actual encoding.
//
// Registers:
//   CR     @ 0x00: Control Register
//   SR     @ 0x04: Status Register
//   IER    @ 0x08: Interrupt Enable Register
//   CFG    @ 0x0C: Configuration Register
//   STRIDE @ 0x10: Stride Register
//

using Antmicro.Renode.Core;
using Antmicro.Renode.Core.Structure.Registers;
using Antmicro.Renode.Logging;
using Antmicro.Renode.Peripherals.Bus;

namespace Antmicro.Renode.Peripherals.Miscellaneous
{
    public class STM32N6_VENC : BasicDoubleWordPeripheral, IKnownSize
    {
        public STM32N6_VENC(IMachine machine) : base(machine)
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

            // IER @ 0x08: Interrupt Enable Register
            Registers.IER.Define(this)
                .WithValueField(0, 32, name: "IER");

            // CFG @ 0x0C: Configuration Register
            Registers.CFG.Define(this)
                .WithValueField(0, 32, name: "CFG");

            // STRIDE @ 0x10: Stride Register
            Registers.STRIDE.Define(this)
                .WithValueField(0, 32, name: "STRIDE");
        }

        private enum Registers : long
        {
            CR     = 0x00,
            SR     = 0x04,
            IER    = 0x08,
            CFG    = 0x0C,
            STRIDE = 0x10,
        }
    }
}
