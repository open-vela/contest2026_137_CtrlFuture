//
// Copyright (c) 2026 CtrlFuture
//
// SPDX-License-Identifier: Apache-2.0
//
// STM32N6 FDCAN (CAN Bus) model for Renode.
// Minimal model: register read/write without actual CAN transfers.
//
// Registers:
//   CREL   @ 0x00: Core release (read-only)
//   ENDN   @ 0x04: Endian (read-only)
//   DBTP   @ 0x0C: Data bit timing (RW)
//   TEST   @ 0x10: Test (RW)
//   RWD    @ 0x14: RAM watchdog (RW)
//   CCCR   @ 0x18: CC control (RW)
//   BTP    @ 0x1C: Bit timing (RW)
//   TSCC   @ 0x20: Timestamp counter config (RW)
//   IR     @ 0x24: Interrupt (RW)
//   IE     @ 0x28: Interrupt enable (RW)
//   ILS    @ 0x34: Interrupt line select (RW)
//   RXF0S  @ 0x44: RX FIFO 0 status (RW)
//   TXBAR  @ 0xC8: TX buffer add request (RW)
//   TXBCR  @ 0xCC: TX buffer cancel request (RW)
//   TXBTO  @ 0xD0: TX buffer to occurrence (RW)
//

using Antmicro.Renode.Core;
using Antmicro.Renode.Core.Structure.Registers;
using Antmicro.Renode.Logging;
using Antmicro.Renode.Peripherals.Bus;

namespace Antmicro.Renode.Peripherals.Miscellaneous
{
    public class STM32N6_FDCAN : BasicDoubleWordPeripheral, IKnownSize
    {
        public STM32N6_FDCAN(IMachine machine) : base(machine)
        {
            DefineRegisters();
        }

        public long Size => 0x400;

        private void DefineRegisters()
        {
            // CREL @ 0x00: Core release (read-only)
            Registers.CREL.Define(this)
                .WithValueField(0, 32, FieldMode.Read, name: "CREL");

            // ENDN @ 0x04: Endian (read-only)
            Registers.ENDN.Define(this)
                .WithValueField(0, 32, FieldMode.Read, name: "ENDN");

            // DBTP @ 0x0C: Data bit timing (RW)
            Registers.DBTP.Define(this)
                .WithValueField(0, 32, name: "DBTP");

            // TEST @ 0x10: Test (RW)
            Registers.TEST.Define(this)
                .WithValueField(0, 32, name: "TEST");

            // RWD @ 0x14: RAM watchdog (RW)
            Registers.RWD.Define(this)
                .WithValueField(0, 32, name: "RWD");

            // CCCR @ 0x18: CC control (RW)
            Registers.CCCR.Define(this)
                .WithValueField(0, 32, name: "CCCR");

            // BTP @ 0x1C: Bit timing (RW)
            Registers.BTP.Define(this)
                .WithValueField(0, 32, name: "BTP");

            // TSCC @ 0x20: Timestamp counter config (RW)
            Registers.TSCC.Define(this)
                .WithValueField(0, 32, name: "TSCC");

            // IR @ 0x24: Interrupt (RW)
            Registers.IR.Define(this)
                .WithValueField(0, 32, name: "IR");

            // IE @ 0x28: Interrupt enable (RW)
            Registers.IE.Define(this)
                .WithValueField(0, 32, name: "IE");

            // ILS @ 0x34: Interrupt line select (RW)
            Registers.ILS.Define(this)
                .WithValueField(0, 32, name: "ILS");

            // RXF0S @ 0x44: RX FIFO 0 status (RW)
            Registers.RXF0S.Define(this)
                .WithValueField(0, 32, name: "RXF0S");

            // TXBAR @ 0xC8: TX buffer add request (RW)
            Registers.TXBAR.Define(this)
                .WithValueField(0, 32, name: "TXBAR");

            // TXBCR @ 0xCC: TX buffer cancel request (RW)
            Registers.TXBCR.Define(this)
                .WithValueField(0, 32, name: "TXBCR");

            // TXBTO @ 0xD0: TX buffer to occurrence (RW)
            Registers.TXBTO.Define(this)
                .WithValueField(0, 32, name: "TXBTO");
        }

        private enum Registers : long
        {
            CREL = 0x00,
            ENDN = 0x04,
            DBTP = 0x0C,
            TEST = 0x10,
            RWD = 0x14,
            CCCR = 0x18,
            BTP = 0x1C,
            TSCC = 0x20,
            IR = 0x24,
            IE = 0x28,
            ILS = 0x34,
            RXF0S = 0x44,
            TXBAR = 0xC8,
            TXBCR = 0xCC,
            TXBTO = 0xD0,
        }
    }
}
