//
// Copyright (c) 2026 CtrlFuture
//
// SPDX-License-Identifier: Apache-2.0
//
// STM32N6 Low-Power Timer (LPTIM) model for Renode.
// Minimal model: register read/write without actual timer counting.
//
// Registers:
//   ISR  @ 0x00: Interrupt and Status Register
//   ICR  @ 0x04: Interrupt Clear Register
//   IER  @ 0x08: Interrupt Enable Register
//   CFGR @ 0x0C: Configuration Register
//   CR   @ 0x10: Control Register
//   CMP  @ 0x14: Compare Register
//   ARR  @ 0x18: Autoreload Register
//   CNT  @ 0x1C: Counter Register
//

using Antmicro.Renode.Core;
using Antmicro.Renode.Core.Structure.Registers;
using Antmicro.Renode.Logging;
using Antmicro.Renode.Peripherals.Bus;

namespace Antmicro.Renode.Peripherals.Miscellaneous
{
    public class STM32N6_LPTIM : BasicDoubleWordPeripheral, IKnownSize
    {
        public STM32N6_LPTIM(IMachine machine) : base(machine)
        {
            DefineRegisters();
        }

        public long Size => 0x400;

        private void DefineRegisters()
        {
            // ISR @ 0x00: Interrupt and Status Register
            Registers.ISR.Define(this)
                .WithValueField(0, 32, name: "ISR");

            // ICR @ 0x04: Interrupt Clear Register (write-only)
            Registers.ICR.Define(this)
                .WithValueField(0, 32, FieldMode.Write, name: "ICR");

            // IER @ 0x08: Interrupt Enable Register
            Registers.IER.Define(this)
                .WithValueField(0, 32, name: "IER");

            // CFGR @ 0x0C: Configuration Register
            Registers.CFGR.Define(this)
                .WithValueField(0, 32, name: "CFGR");

            // CR @ 0x10: Control Register
            Registers.CR.Define(this)
                .WithValueField(0, 32, name: "CR");

            // CMP @ 0x14: Compare Register
            Registers.CMP.Define(this)
                .WithValueField(0, 32, name: "CMP");

            // ARR @ 0x18: Autoreload Register
            Registers.ARR.Define(this)
                .WithValueField(0, 32, name: "ARR");

            // CNT @ 0x1C: Counter Register
            Registers.CNT.Define(this)
                .WithValueField(0, 32, name: "CNT");
        }

        private enum Registers : long
        {
            ISR  = 0x00,
            ICR  = 0x04,
            IER  = 0x08,
            CFGR = 0x0C,
            CR   = 0x10,
            CMP  = 0x14,
            ARR  = 0x18,
            CNT  = 0x1C,
        }
    }
}
