//
// Copyright (c) 2026 CtrlFuture
//
// SPDX-License-Identifier: Apache-2.0
//
// STM32N6 LTDC model for Renode.
// L2 model: GCR.LTDCEN sticks; SRCR IMR/VBR self-clear after reload;
// layer CR enable sticks. No display output.
//
// CMSIS LTDC_TypeDef / LTDC_Layer_TypeDef (N6, not F4/H7):
//   SSCR@0x08 BPCR@0x0C AWCR@0x10 TWCR@0x14 GCR@0x18
//   SRCR@0x24 BCCR@0x2C IER@0x34 ISR@0x38 ICR@0x3C
//   LIPCR@0x40 CPSR@0x44 CDSR@0x48
//   Layer1 base = LTDC + 0x100: C0R@+0 C1R@+4 RCR@+8 CR@+0x0C
//     → absolute Layer1 CR @ 0x10C
//   Layer2 base = LTDC + 0x200 → Layer2 CR @ 0x20C
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
            l1cr = 0;
            l2cr = 0;
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

            // Layer1 @ 0x100: C0R/C1R/RCR stubs + CR enable
            Registers.L1_C0R.Define(this)
                .WithValueField(0, 32, name: "L1_C0R");
            Registers.L1_C1R.Define(this)
                .WithValueField(0, 32, name: "L1_C1R");
            Registers.L1_RCR.Define(this)
                .WithValueField(0, 32, name: "L1_RCR");
            Registers.L1_CR.Define(this)
                .WithValueField(0, 32,
                    valueProviderCallback: _ => l1cr,
                    writeCallback: (_, val) => l1cr = (uint)val,
                    name: "L1_CR");
            Registers.L1_WHPCR.Define(this)
                .WithValueField(0, 32, name: "L1_WHPCR");
            Registers.L1_WVPCR.Define(this)
                .WithValueField(0, 32, name: "L1_WVPCR");
            Registers.L1_PFCR.Define(this)
                .WithValueField(0, 32, name: "L1_PFCR");
            Registers.L1_CFBAR.Define(this)
                .WithValueField(0, 32, name: "L1_CFBAR");
            Registers.L1_CFBLR.Define(this)
                .WithValueField(0, 32, name: "L1_CFBLR");
            Registers.L1_CFBLNR.Define(this)
                .WithValueField(0, 32, name: "L1_CFBLNR");

            // Layer2 @ 0x200
            Registers.L2_C0R.Define(this)
                .WithValueField(0, 32, name: "L2_C0R");
            Registers.L2_C1R.Define(this)
                .WithValueField(0, 32, name: "L2_C1R");
            Registers.L2_RCR.Define(this)
                .WithValueField(0, 32, name: "L2_RCR");
            Registers.L2_CR.Define(this)
                .WithValueField(0, 32,
                    valueProviderCallback: _ => l2cr,
                    writeCallback: (_, val) => l2cr = (uint)val,
                    name: "L2_CR");
            Registers.L2_WHPCR.Define(this)
                .WithValueField(0, 32, name: "L2_WHPCR");
            Registers.L2_WVPCR.Define(this)
                .WithValueField(0, 32, name: "L2_WVPCR");
            Registers.L2_PFCR.Define(this)
                .WithValueField(0, 32, name: "L2_PFCR");
            Registers.L2_CFBAR.Define(this)
                .WithValueField(0, 32, name: "L2_CFBAR");
            Registers.L2_CFBLR.Define(this)
                .WithValueField(0, 32, name: "L2_CFBLR");
            Registers.L2_CFBLNR.Define(this)
                .WithValueField(0, 32, name: "L2_CFBLNR");
        }

        private uint gcr;
        private uint l1cr;
        private uint l2cr;

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

            // Layer1 base 0x100
            L1_C0R = 0x100,
            L1_C1R = 0x104,
            L1_RCR = 0x108,
            L1_CR = 0x10C,
            L1_WHPCR = 0x110,
            L1_WVPCR = 0x114,
            L1_PFCR = 0x11C,
            L1_CFBAR = 0x134,
            L1_CFBLR = 0x138,
            L1_CFBLNR = 0x13C,

            // Layer2 base 0x200
            L2_C0R = 0x200,
            L2_C1R = 0x204,
            L2_RCR = 0x208,
            L2_CR = 0x20C,
            L2_WHPCR = 0x210,
            L2_WVPCR = 0x214,
            L2_PFCR = 0x21C,
            L2_CFBAR = 0x234,
            L2_CFBLR = 0x238,
            L2_CFBLNR = 0x23C,
        }
    }
}
