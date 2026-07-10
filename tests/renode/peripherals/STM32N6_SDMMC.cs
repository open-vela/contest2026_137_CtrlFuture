//
// Copyright (c) 2026 CtrlFuture
//
// SPDX-License-Identifier: Apache-2.0
//
// STM32N6 SDMMC model for Renode.
// Minimal model: register read/write without actual SD card transfers.
//
// Registers:
//   POWER @ 0x00: Power control
//   CLKCR @ 0x04: Clock control
//   ARG   @ 0x08: Command argument
//   CMD   @ 0x0C: Command register
//   RESPCMD @ 0x10: Command response
//   RESP1 @ 0x14: Response 1
//   DTIMER @ 0x24: Data timer
//   DLEN  @ 0x28: Data length
//   DCTRL @ 0x2C: Data control
//   STA   @ 0x34: Status
//   ICR   @ 0x38: Interrupt clear
//   MASK  @ 0x3C: Mask
//

using Antmicro.Renode.Core;
using Antmicro.Renode.Core.Structure.Registers;
using Antmicro.Renode.Logging;
using Antmicro.Renode.Peripherals.Bus;

namespace Antmicro.Renode.Peripherals.Miscellaneous
{
    public class STM32N6_SDMMC : BasicDoubleWordPeripheral, IKnownSize
    {
        public STM32N6_SDMMC(IMachine machine) : base(machine)
        {
            DefineRegisters();
        }

        public long Size => 0x1000;

        private void DefineRegisters()
        {
            // POWER @ 0x00
            Registers.POWER.Define(this)
                .WithValueField(0, 32, name: "POWER");

            // CLKCR @ 0x04
            Registers.CLKCR.Define(this)
                .WithValueField(0, 32, name: "CLKCR");

            // ARG @ 0x08
            Registers.ARG.Define(this)
                .WithValueField(0, 32, name: "ARG");

            // CMD @ 0x0C
            Registers.CMD.Define(this)
                .WithValueField(0, 32, name: "CMD");

            // RESPCMD @ 0x10
            Registers.RESPCMD.Define(this)
                .WithValueField(0, 32, name: "RESPCMD");

            // RESP1 @ 0x14
            Registers.RESP1.Define(this)
                .WithValueField(0, 32, name: "RESP1");

            // DTIMER @ 0x24
            Registers.DTIMER.Define(this)
                .WithValueField(0, 32, name: "DTIMER");

            // DLEN @ 0x28
            Registers.DLEN.Define(this)
                .WithValueField(0, 32, name: "DLEN");

            // DCTRL @ 0x2C
            Registers.DCTRL.Define(this)
                .WithValueField(0, 32, name: "DCTRL");

            // STA @ 0x34: Status register
            // CCRCFAIL (bit 0), DCRCFAIL (bit 1), CTIMEOUT (bit 2),
            // DTIMEOUT (bit 3), TXUNDERR (bit 4), RXOVERR (bit 5),
            // CMDREND (bit 6), CMDSENT (bit 7), DATAEND (bit 8),
            // DBCKEND (bit 10), CMDACT (bit 11), TXACT (bit 12),
            // RXACT (bit 13), TXFIFOHE (bit 14), RXFIFOHF (bit 15),
            // TXFIFOF (bit 16), RXFIFOF (bit 17), TXFIFOE (bit 18),
            // RXFIFOE (bit 19), BUSYD0 (bit 20), BUSYD0END (bit 21)
            Registers.STA.Define(this)
                .WithValueField(0, 32, name: "STA");

            // ICR @ 0x38: Interrupt clear (write-only)
            Registers.ICR.Define(this)
                .WithValueField(0, 32, FieldMode.Write, name: "ICR");

            // MASK @ 0x3C
            Registers.MASK.Define(this)
                .WithValueField(0, 32, name: "MASK");
        }

        private enum Registers : long
        {
            POWER = 0x00,
            CLKCR = 0x04,
            ARG = 0x08,
            CMD = 0x0C,
            RESPCMD = 0x10,
            RESP1 = 0x14,
            DTIMER = 0x24,
            DLEN = 0x28,
            DCTRL = 0x2C,
            STA = 0x34,
            ICR = 0x38,
            MASK = 0x3C,
        }
    }
}
