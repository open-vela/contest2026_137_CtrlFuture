//
// Copyright (c) 2026 CtrlFuture
//
// SPDX-License-Identifier: Apache-2.0
//
// STM32N6 I2C model for Renode.
// Minimal model: register read/write without actual I2C transfers.
//
// Registers:
//   CR1      @ 0x00: Control 1
//   CR2      @ 0x04: Control 2
//   OAR1     @ 0x08: Own address 1
//   OAR2     @ 0x0C: Own address 2
//   TIMINGR  @ 0x10: Timing register
//   TIMEOUTR @ 0x14: Timeout register
//   ISR      @ 0x18: Interrupt and status
//   ICR      @ 0x1C: Interrupt clear
//   PECR     @ 0x20: PEC register
//   RXDR     @ 0x24: Receive data
//   TXDR     @ 0x28: Transmit data
//

using Antmicro.Renode.Core;
using Antmicro.Renode.Core.Structure.Registers;
using Antmicro.Renode.Logging;
using Antmicro.Renode.Peripherals.Bus;

namespace Antmicro.Renode.Peripherals.Miscellaneous
{
    public class STM32N6_I2C : BasicDoubleWordPeripheral, IKnownSize
    {
        public STM32N6_I2C(IMachine machine) : base(machine)
        {
            DefineRegisters();
        }

        public long Size => 0x100;

        private void DefineRegisters()
        {
            // CR1 @ 0x00
            Registers.CR1.Define(this)
                .WithValueField(0, 32, name: "CR1");

            // CR2 @ 0x04
            Registers.CR2.Define(this)
                .WithValueField(0, 32, name: "CR2");

            // OAR1 @ 0x08
            Registers.OAR1.Define(this)
                .WithValueField(0, 32, name: "OAR1");

            // OAR2 @ 0x0C
            Registers.OAR2.Define(this)
                .WithValueField(0, 32, name: "OAR2");

            // TIMINGR @ 0x10
            Registers.TIMINGR.Define(this)
                .WithValueField(0, 32, name: "TIMINGR");

            // TIMEOUTR @ 0x14
            Registers.TIMEOUTR.Define(this)
                .WithValueField(0, 32, name: "TIMEOUTR");

            // ISR @ 0x18
            // TXE (bit 0) = 1 at reset (TX empty)
            Registers.ISR.Define(this, 0x01)
                .WithFlag(0, name: "TXE")
                .WithFlag(1, name: "TXIS")
                .WithFlag(2, name: "RXNE")
                .WithFlag(3, name: "ADDR")
                .WithFlag(4, name: "NACKF")
                .WithFlag(5, name: "STOPF")
                .WithFlag(6, name: "TC")
                .WithFlag(7, name: "TCR")
                .WithFlag(8, name: "BERR")
                .WithFlag(9, name: "ARLO")
                .WithFlag(10, name: "OVR")
                .WithFlag(11, name: "PECERR")
                .WithFlag(12, name: "TIMEOUT")
                .WithFlag(13, name: "ALERT")
                .WithFlag(14, FieldMode.Read,
                    valueProviderCallback: _ => false, name: "BUSY")
                .WithFlag(15, name: "DIR")
                .WithReservedBits(16, 16);

            // ICR @ 0x1C (write-only, clear flags)
            Registers.ICR.Define(this)
                .WithValueField(0, 32, FieldMode.Write, name: "ICR");

            // PECR @ 0x20
            Registers.PECR.Define(this)
                .WithValueField(0, 32, name: "PECR");

            // RXDR @ 0x24
            Registers.RXDR.Define(this)
                .WithValueField(0, 32, name: "RXDR");

            // TXDR @ 0x28
            Registers.TXDR.Define(this)
                .WithValueField(0, 32, name: "TXDR");
        }

        private enum Registers : long
        {
            CR1 = 0x00,
            CR2 = 0x04,
            OAR1 = 0x08,
            OAR2 = 0x0C,
            TIMINGR = 0x10,
            TIMEOUTR = 0x14,
            ISR = 0x18,
            ICR = 0x1C,
            PECR = 0x20,
            RXDR = 0x24,
            TXDR = 0x28,
        }
    }
}
