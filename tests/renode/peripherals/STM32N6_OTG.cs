//
// Copyright (c) 2026 CtrlFuture
//
// SPDX-License-Identifier: Apache-2.0
//
// STM32N6 OTG (USB OTG HS) model for Renode.
// L2 model: GRSTCTL.CSRST (bit0) self-clears; AHBIDL (bit31) always 1
// so core soft-reset wait loops complete. No USB transfers.
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

        public override void Reset()
        {
            base.Reset();
            grstctl = 0x80000000; // AHBIDL set at idle
            gahbcfg = 0;
            gusbcfg = 0;
            dcfg = 0;
            cid = 0;
        }

        private void DefineRegisters()
        {
            Registers.GOTGCTL.Define(this)
                .WithValueField(0, 32, FieldMode.Read, name: "GOTGCTL");

            Registers.GOTGINT.Define(this)
                .WithValueField(0, 32, name: "GOTGINT");

            Registers.GAHBCFG.Define(this)
                .WithValueField(0, 32,
                    valueProviderCallback: _ => gahbcfg,
                    writeCallback: (_, val) => gahbcfg = (uint)val,
                    name: "GAHBCFG");

            Registers.GUSBCFG.Define(this)
                .WithValueField(0, 32,
                    valueProviderCallback: _ => gusbcfg,
                    writeCallback: (_, val) => gusbcfg = (uint)val,
                    name: "GUSBCFG");

            // GRSTCTL @ 0x010: CSRST (bit0) self-clears; AHBIDL (bit31)=1
            Registers.GRSTCTL.Define(this)
                .WithValueField(0, 32,
                    valueProviderCallback: _ => grstctl | 0x80000000u,
                    writeCallback: (_, val) =>
                    {
                        // Soft-reset and TX/RX FIFO flushes complete instantly
                        var v = (uint)val;
                        v &= ~0x1u;          // CSRST self-clear
                        v &= ~(0x1u << 4);   // RXFFLSH self-clear
                        v &= ~(0x1u << 5);   // TXFFLSH self-clear
                        grstctl = v | 0x80000000u; // AHBIDL
                    },
                    name: "GRSTCTL");

            Registers.GINTSTS.Define(this)
                .WithValueField(0, 32, FieldMode.Read, name: "GINTSTS");

            Registers.GINTMSK.Define(this)
                .WithValueField(0, 32, name: "GINTMSK");

            Registers.GRXSTSR.Define(this)
                .WithValueField(0, 32, FieldMode.Read, name: "GRXSTSR");

            Registers.GRXFSIZ.Define(this)
                .WithValueField(0, 32, name: "GRXFSIZ");

            Registers.GNPTXFSIZ.Define(this)
                .WithValueField(0, 32, name: "GNPTXFSIZ");

            Registers.GCCFG.Define(this)
                .WithValueField(0, 32, name: "GCCFG");

            Registers.CID.Define(this)
                .WithValueField(0, 32,
                    valueProviderCallback: _ => cid,
                    writeCallback: (_, val) => cid = (uint)val,
                    name: "CID");

            Registers.DCFG.Define(this)
                .WithValueField(0, 32,
                    valueProviderCallback: _ => dcfg,
                    writeCallback: (_, val) => dcfg = (uint)val,
                    name: "DCFG");

            Registers.DCTL.Define(this)
                .WithValueField(0, 32, FieldMode.Write, name: "DCTL");

            Registers.DSTS.Define(this)
                .WithValueField(0, 32, FieldMode.Read, name: "DSTS");

            Registers.DIEPMSK.Define(this)
                .WithValueField(0, 32, name: "DIEPMSK");

            Registers.DOEPMSK.Define(this)
                .WithValueField(0, 32, name: "DOEPMSK");

            Registers.DAINT.Define(this)
                .WithValueField(0, 32, name: "DAINT");
        }

        private uint grstctl = 0x80000000;
        private uint gahbcfg;
        private uint gusbcfg;
        private uint dcfg;
        private uint cid;

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
