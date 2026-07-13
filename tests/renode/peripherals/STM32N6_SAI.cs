//
// Copyright (c) 2026 CtrlFuture
//
// SPDX-License-Identifier: Apache-2.0
//
// STM32N6 SAI (Serial Audio Interface) model for Renode.
// Minimal model: register read/write without actual audio transfers.
//
// Registers:
//   CR1   @ 0x04: Configuration 1 (RW)
//   CR2   @ 0x08: Configuration 2 (RW)
//   FRCR  @ 0x0C: Frame configuration (RW)
//   SLOTR @ 0x10: Slot configuration (RW)
//   SR    @ 0x14: Status (read-only)
//   CLRFR @ 0x18: Clear flag (write-only)
//   DR    @ 0x20: Data (RW)
//

using Antmicro.Renode.Core;
using Antmicro.Renode.Core.Structure.Registers;
using Antmicro.Renode.Logging;
using Antmicro.Renode.Peripherals.Bus;

namespace Antmicro.Renode.Peripherals.Miscellaneous
{
    public class STM32N6_SAI : BasicDoubleWordPeripheral, IKnownSize
    {
        public STM32N6_SAI(IMachine machine) : base(machine)
        {
            DefineRegisters();
        }

        public long Size => 0x400;

        private void DefineRegisters()
        {
            // CR1 @ 0x04: Configuration 1 (RW)
            Registers.CR1.Define(this)
                .WithValueField(0, 32, name: "CR1");

            // CR2 @ 0x08: Configuration 2 (RW)
            Registers.CR2.Define(this)
                .WithValueField(0, 32, name: "CR2");

            // FRCR @ 0x0C: Frame configuration (RW)
            Registers.FRCR.Define(this)
                .WithValueField(0, 32, name: "FRCR");

            // SLOTR @ 0x10: Slot configuration (RW)
            Registers.SLOTR.Define(this)
                .WithValueField(0, 32, name: "SLOTR");

            // SR @ 0x14: Status register (read-only)
            // All bits read as zero at reset
            Registers.SR.Define(this)
                .WithValueField(0, 32, FieldMode.Read, name: "SR");

            // CLRFR @ 0x18: Clear flag (write-only)
            Registers.CLRFR.Define(this)
                .WithValueField(0, 32, FieldMode.Write, name: "CLRFR");

            // DR @ 0x20: Data register (RW)
            Registers.DR.Define(this)
                .WithValueField(0, 32, name: "DR");
        }

        private enum Registers : long
        {
            CR1 = 0x04,
            CR2 = 0x08,
            FRCR = 0x0C,
            SLOTR = 0x10,
            SR = 0x14,
            CLRFR = 0x18,
            DR = 0x20,
        }
    }
}
