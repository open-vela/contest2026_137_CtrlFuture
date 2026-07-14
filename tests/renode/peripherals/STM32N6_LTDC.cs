//
// Copyright (c) 2026 CtrlFuture
//
// SPDX-License-Identifier: Apache-2.0
//
// STM32N6 LTDC model for Renode.
// L2 model: GCR.LTDCEN sticks; SRCR IMR/VBR self-clear after reload;
// layer CR enable sticks. No display output.
//

using Antmicro.Renode.Core;
using Antmicro.Renode.Core.Structure.Registers;
using Antmicro.Renode.Logging;
using Antmicro.Renode.Peripherals.Bus;

namespace Antmicro.Renode.Peripherals.Miscellaneous
{
    public class STM32N6_LTDC : BasicDoubleWordPeripheral, IKnownSize
    {
        public STM32N6_LTDC(IMachine machine) : base(machine)
        {
            DefineRegisters();
        }

        public long Size => 0x400;

        public override void Reset()
        {
            base.Reset();
            gcr = 0;
            l0cr = 0;
            l1cr = 0;
        }

        private void DefineRegisters()
        {
            Registers.SSCR.Define(this)
                .WithValueField(0, 32, name: "SSCR");

            Registers.BPCR.Define(this)
                .WithValueField(0, 32, name: "BPCR");

            Registers.AWCR.Define(this)
                .WithValueField(0, 32, name: "AWCR");

            Registers.TWCR.Define(this)
                .WithValueField(0, 32, name: "TWCR");

            // GCR @ 0x18: LTDCEN bit0 sticks
            Registers.GCR.Define(this)
                .WithValueField(0, 32,
                    valueProviderCallback: _ => gcr,
                    writeCallback: (_, val) => gcr = (uint)val,
                    name: "GCR");

            // SRCR @ 0x24: IMR (bit0) / VBR (bit1) self-clear after reload
            Registers.SRCR.Define(this)
                .WithValueField(0, 32,
                    valueProviderCallback: _ => 0,
                    writeCallback: (_, __) =>
                    {
                        // Instant shadow reload in simulation — always reads 0
                    },
                    name: "SRCR");

            Registers.BCCR.Define(this)
                .WithValueField(0, 32, name: "BCCR");

            Registers.IER.Define(this)
                .WithValueField(0, 32, name: "IER");

            Registers.ISR.Define(this)
                .WithValueField(0, 32, FieldMode.Read, name: "ISR");

            Registers.ICR.Define(this)
                .WithValueField(0, 32, FieldMode.Write, name: "ICR");

            Registers.LIPCR.Define(this)
                .WithValueField(0, 32, name: "LIPCR");

            Registers.CPSR.Define(this)
                .WithValueField(0, 32, FieldMode.Read, name: "CPSR");

            Registers.CDSR.Define(this)
                .WithValueField(0, 32, FieldMode.Read, name: "CDSR");

            Registers.L0_CR.Define(this)
                .WithValueField(0, 32,
                    valueProviderCallback: _ => l0cr,
                    writeCallback: (_, val) => l0cr = (uint)val,
                    name: "L0_CR");

            Registers.L0_WHPCR.Define(this)
                .WithValueField(0, 32, name: "L0_WHPCR");
            Registers.L0_WVPCR.Define(this)
                .WithValueField(0, 32, name: "L0_WVPCR");
            Registers.L0_CKCR.Define(this)
                .WithValueField(0, 32, name: "L0_CKCR");
            Registers.L0_PFCR.Define(this)
                .WithValueField(0, 32, name: "L0_PFCR");
            Registers.L0_CACR.Define(this)
                .WithValueField(0, 32, name: "L0_CACR");
            Registers.L0_DCCR.Define(this)
                .WithValueField(0, 32, name: "L0_DCCR");
            Registers.L0_BFCR.Define(this)
                .WithValueField(0, 32, name: "L0_BFCR");
            Registers.L0_CFBAR.Define(this)
                .WithValueField(0, 32, name: "L0_CFBAR");
            Registers.L0_CFBLNR.Define(this)
                .WithValueField(0, 32, name: "L0_CFBLNR");
            Registers.L0_CFBLR.Define(this)
                .WithValueField(0, 32, name: "L0_CFBLR");
            Registers.L0_CLUTWR.Define(this)
                .WithValueField(0, 32, name: "L0_CLUTWR");

            Registers.L1_CR.Define(this)
                .WithValueField(0, 32,
                    valueProviderCallback: _ => l1cr,
                    writeCallback: (_, val) => l1cr = (uint)val,
                    name: "L1_CR");

            Registers.L1_WHPCR.Define(this)
                .WithValueField(0, 32, name: "L1_WHPCR");
            Registers.L1_WVPCR.Define(this)
                .WithValueField(0, 32, name: "L1_WVPCR");
            Registers.L1_CKCR.Define(this)
                .WithValueField(0, 32, name: "L1_CKCR");
            Registers.L1_PFCR.Define(this)
                .WithValueField(0, 32, name: "L1_PFCR");
            Registers.L1_CACR.Define(this)
                .WithValueField(0, 32, name: "L1_CACR");
            Registers.L1_DCCR.Define(this)
                .WithValueField(0, 32, name: "L1_DCCR");
            Registers.L1_BFCR.Define(this)
                .WithValueField(0, 32, name: "L1_BFCR");
            Registers.L1_CFBAR.Define(this)
                .WithValueField(0, 32, name: "L1_CFBAR");
            Registers.L1_CFBLNR.Define(this)
                .WithValueField(0, 32, name: "L1_CFBLNR");
            Registers.L1_CFBLR.Define(this)
                .WithValueField(0, 32, name: "L1_CFBLR");
            Registers.L1_CLUTWR.Define(this)
                .WithValueField(0, 32, name: "L1_CLUTWR");
        }

        private uint gcr;
        private uint l0cr;
        private uint l1cr;

        private enum Registers : long
        {
            SSCR = 0x08,
            BPCR = 0x0C,
            AWCR = 0x10,
            TWCR = 0x14,
            GCR = 0x18,
            SRCR = 0x24,
            BCCR = 0x2C,
            IER = 0x34,
            ISR = 0x38,
            ICR = 0x3C,
            LIPCR = 0x40,
            CPSR = 0x44,
            CDSR = 0x48,
            L0_CR = 0x80,
            L0_WHPCR = 0x84,
            L0_WVPCR = 0x88,
            L0_CKCR = 0x8C,
            L0_PFCR = 0x90,
            L0_CACR = 0x94,
            L0_DCCR = 0x98,
            L0_BFCR = 0x9C,
            L0_CFBAR = 0xAC,
            L0_CFBLNR = 0xB0,
            L0_CFBLR = 0xB4,
            L0_CLUTWR = 0xC4,
            L1_CR = 0x100,
            L1_WHPCR = 0x104,
            L1_WVPCR = 0x108,
            L1_CKCR = 0x10C,
            L1_PFCR = 0x110,
            L1_CACR = 0x114,
            L1_DCCR = 0x118,
            L1_BFCR = 0x11C,
            L1_CFBAR = 0x12C,
            L1_CFBLNR = 0x130,
            L1_CFBLR = 0x134,
            L1_CLUTWR = 0x144,
        }
    }
}
