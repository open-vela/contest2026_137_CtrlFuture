//
// Copyright (c) 2026 CtrlFuture
//
// SPDX-License-Identifier: Apache-2.0
//
// STM32N6 Cryptographic Processor (SAES) model for Renode.
// Minimal model: register read/write without actual crypto operations.
//
// Registers:
//   CR    @ 0x00: Control Register
//   SR    @ 0x04: Status Register
//   DIN   @ 0x08: Data Input Register
//   DOUT  @ 0x0C: Data Output Register
//   KEYR0 @ 0x10: Key Register 0
//   KEYR1 @ 0x14: Key Register 1
//   KEYR2 @ 0x18: Key Register 2
//   KEYR3 @ 0x1C: Key Register 3
//   IVR0  @ 0x20: Initialization Vector Register 0
//   IVR1  @ 0x24: Initialization Vector Register 1
//   IVR2  @ 0x28: Initialization Vector Register 2
//   IVR3  @ 0x2C: Initialization Vector Register 3
//

using Antmicro.Renode.Core;
using Antmicro.Renode.Core.Structure.Registers;
using Antmicro.Renode.Logging;
using Antmicro.Renode.Peripherals.Bus;

namespace Antmicro.Renode.Peripherals.Miscellaneous
{
    public class STM32N6_CRYP : BasicDoubleWordPeripheral, IKnownSize
    {
        public STM32N6_CRYP(IMachine machine) : base(machine)
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

            // DIN @ 0x08: Data Input Register (write-only)
            Registers.DIN.Define(this)
                .WithValueField(0, 32, FieldMode.Write, name: "DIN");

            // DOUT @ 0x0C: Data Output Register (read-only)
            Registers.DOUT.Define(this)
                .WithValueField(0, 32, FieldMode.Read, name: "DOUT");

            // KEYR0 @ 0x10: Key Register 0 (write-only)
            Registers.KEYR0.Define(this)
                .WithValueField(0, 32, FieldMode.Write, name: "KEYR0");

            // KEYR1 @ 0x14: Key Register 1 (write-only)
            Registers.KEYR1.Define(this)
                .WithValueField(0, 32, FieldMode.Write, name: "KEYR1");

            // KEYR2 @ 0x18: Key Register 2 (write-only)
            Registers.KEYR2.Define(this)
                .WithValueField(0, 32, FieldMode.Write, name: "KEYR2");

            // KEYR3 @ 0x1C: Key Register 3 (write-only)
            Registers.KEYR3.Define(this)
                .WithValueField(0, 32, FieldMode.Write, name: "KEYR3");

            // IVR0 @ 0x20: Initialization Vector Register 0
            Registers.IVR0.Define(this)
                .WithValueField(0, 32, name: "IVR0");

            // IVR1 @ 0x24: Initialization Vector Register 1
            Registers.IVR1.Define(this)
                .WithValueField(0, 32, name: "IVR1");

            // IVR2 @ 0x28: Initialization Vector Register 2
            Registers.IVR2.Define(this)
                .WithValueField(0, 32, name: "IVR2");

            // IVR3 @ 0x2C: Initialization Vector Register 3
            Registers.IVR3.Define(this)
                .WithValueField(0, 32, name: "IVR3");
        }

        private enum Registers : long
        {
            CR    = 0x00,
            SR    = 0x04,
            DIN   = 0x08,
            DOUT  = 0x0C,
            KEYR0 = 0x10,
            KEYR1 = 0x14,
            KEYR2 = 0x18,
            KEYR3 = 0x1C,
            IVR0  = 0x20,
            IVR1  = 0x24,
            IVR2  = 0x28,
            IVR3  = 0x2C,
        }
    }
}
