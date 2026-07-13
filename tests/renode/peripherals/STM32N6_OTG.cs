//
// Copyright (c) 2026 CtrlFuture
//
// SPDX-License-Identifier: Apache-2.0
//
// STM32N6 OTG (USB OTG HS) model for Renode.
// Minimal model: register read/write without actual USB transfers.
//
// Registers:
//   GOTGCTL  @ 0x000: OTG control (read-only)
//   GOTGINT  @ 0x004: OTG interrupt (RW)
//   GAHBCFG  @ 0x008: AHB configuration (RW)
//   GUSBCFG  @ 0x00C: USB configuration (RW)
//   GRSTCTL  @ 0x010: Reset (RW)
//   GINTSTS  @ 0x014: Interrupt status (read-only)
//   GINTMSK  @ 0x018: Interrupt mask (RW)
//   GRXSTSR  @ 0x01C: Receive status debug read (read-only)
//   GRXFSIZ  @ 0x024: Receive FIFO size (RW)
//   GNPTXFSIZ @ 0x028: Non-periodic TX FIFO size (RW)
//   GCCFG    @ 0x038: General core configuration (RW)
//   CID      @ 0x03C: Core ID (RW)
//   DCFG     @ 0x800: Device configuration (RW)
//   DCTL     @ 0x804: Device control (write-only)
//   DSTS     @ 0x808: Device status (read-only)
//   DIEPMSK  @ 0x810: Device IN endpoint mask (RW)
//   DOEPMSK  @ 0x814: Device OUT endpoint mask (RW)
//   DAINT    @ 0x818: Device all endpoints interrupt (RW)
//

using Antmicro.Renode.Core;
using Antmicro.Renode.Core.Structure.Registers;
using Antmicro.Renode.Logging;
using Antmicro.Renode.Peripherals.Bus;

namespace Antmicro.Renode.Peripherals.Miscellaneous
{
    public class STM32N6_OTG : BasicDoubleWordPeripheral, IKnownSize
    {
        public STM32N6_OTG(IMachine machine) : base(machine)
        {
            DefineRegisters();
        }

        public long Size => 0x1000;

        private void DefineRegisters()
        {
            // GOTGCTL @ 0x000: OTG control (read-only)
            Registers.GOTGCTL.Define(this)
                .WithValueField(0, 32, FieldMode.Read, name: "GOTGCTL");

            // GOTGINT @ 0x004: OTG interrupt (RW)
            Registers.GOTGINT.Define(this)
                .WithValueField(0, 32, name: "GOTGINT");

            // GAHBCFG @ 0x008: AHB configuration (RW)
            Registers.GAHBCFG.Define(this)
                .WithValueField(0, 32, name: "GAHBCFG");

            // GUSBCFG @ 0x00C: USB configuration (RW)
            Registers.GUSBCFG.Define(this)
                .WithValueField(0, 32, name: "GUSBCFG");

            // GRSTCTL @ 0x010: Reset (RW)
            Registers.GRSTCTL.Define(this)
                .WithValueField(0, 32, name: "GRSTCTL");

            // GINTSTS @ 0x014: Interrupt status (read-only)
            Registers.GINTSTS.Define(this)
                .WithValueField(0, 32, FieldMode.Read, name: "GINTSTS");

            // GINTMSK @ 0x018: Interrupt mask (RW)
            Registers.GINTMSK.Define(this)
                .WithValueField(0, 32, name: "GINTMSK");

            // GRXSTSR @ 0x01C: Receive status debug read (read-only)
            Registers.GRXSTSR.Define(this)
                .WithValueField(0, 32, FieldMode.Read, name: "GRXSTSR");

            // GRXFSIZ @ 0x024: Receive FIFO size (RW)
            Registers.GRXFSIZ.Define(this)
                .WithValueField(0, 32, name: "GRXFSIZ");

            // GNPTXFSIZ @ 0x028: Non-periodic TX FIFO size (RW)
            Registers.GNPTXFSIZ.Define(this)
                .WithValueField(0, 32, name: "GNPTXFSIZ");

            // GCCFG @ 0x038: General core configuration (RW)
            Registers.GCCFG.Define(this)
                .WithValueField(0, 32, name: "GCCFG");

            // CID @ 0x03C: Core ID (RW)
            Registers.CID.Define(this)
                .WithValueField(0, 32, name: "CID");

            // DCFG @ 0x800: Device configuration (RW)
            Registers.DCFG.Define(this)
                .WithValueField(0, 32, name: "DCFG");

            // DCTL @ 0x804: Device control (write-only)
            Registers.DCTL.Define(this)
                .WithValueField(0, 32, FieldMode.Write, name: "DCTL");

            // DSTS @ 0x808: Device status (read-only)
            Registers.DSTS.Define(this)
                .WithValueField(0, 32, FieldMode.Read, name: "DSTS");

            // DIEPMSK @ 0x810: Device IN endpoint mask (RW)
            Registers.DIEPMSK.Define(this)
                .WithValueField(0, 32, name: "DIEPMSK");

            // DOEPMSK @ 0x814: Device OUT endpoint mask (RW)
            Registers.DOEPMSK.Define(this)
                .WithValueField(0, 32, name: "DOEPMSK");

            // DAINT @ 0x818: Device all endpoints interrupt (RW)
            Registers.DAINT.Define(this)
                .WithValueField(0, 32, name: "DAINT");
        }

        private enum Registers : long
        {
            GOTGCTL = 0x000,
            GOTGINT = 0x004,
            GAHBCFG = 0x008,
            GUSBCFG = 0x00C,
            GRSTCTL = 0x010,
            GINTSTS = 0x014,
            GINTMSK = 0x018,
            GRXSTSR = 0x01C,
            GRXFSIZ = 0x024,
            GNPTXFSIZ = 0x028,
            GCCFG = 0x038,
            CID = 0x03C,
            DCFG = 0x800,
            DCTL = 0x804,
            DSTS = 0x808,
            DIEPMSK = 0x810,
            DOEPMSK = 0x814,
            DAINT = 0x818,
        }
    }
}
