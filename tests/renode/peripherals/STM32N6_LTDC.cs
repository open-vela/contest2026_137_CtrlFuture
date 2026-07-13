//
// Copyright (c) 2026 CtrlFuture
//
// SPDX-License-Identifier: Apache-2.0
//
// STM32N6 LTDC model for Renode.
// Minimal model: register read/write without actual display output.
//
// Registers:
//   SSCR   @ 0x08: Synchronization Size Configuration
//   BPCR   @ 0x0C: Back Porch Configuration
//   AWCR   @ 0x10: Active Width Configuration
//   TWCR   @ 0x14: Total Width Configuration
//   GCR    @ 0x18: Global Control
//   SRCR   @ 0x24: Shadow Reload Configuration
//   BCCR   @ 0x2C: Background Color Configuration
//   IER    @ 0x34: Interrupt Enable
//   ISR    @ 0x38: Interrupt Status
//   ICR    @ 0x3C: Interrupt Clear
//   LIPCR  @ 0x40: Line Interrupt Position Configuration
//   CPSR   @ 0x44: Current Position Status
//   CDSR   @ 0x48: Current Display Status
//   Layer0 @ +0x80: Layer 0 registers
//   Layer1 @ +0x100: Layer 1 registers
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

        private void DefineRegisters()
        {
            // SSCR @ 0x08: Synchronization Size Configuration
            Registers.SSCR.Define(this)
                .WithValueField(0, 32, name: "SSCR");

            // BPCR @ 0x0C: Back Porch Configuration
            Registers.BPCR.Define(this)
                .WithValueField(0, 32, name: "BPCR");

            // AWCR @ 0x10: Active Width Configuration
            Registers.AWCR.Define(this)
                .WithValueField(0, 32, name: "AWCR");

            // TWCR @ 0x14: Total Width Configuration
            Registers.TWCR.Define(this)
                .WithValueField(0, 32, name: "TWCR");

            // GCR @ 0x18: Global Control
            Registers.GCR.Define(this)
                .WithValueField(0, 32, name: "GCR");

            // SRCR @ 0x24: Shadow Reload Configuration
            Registers.SRCR.Define(this)
                .WithValueField(0, 32, name: "SRCR");

            // BCCR @ 0x2C: Background Color Configuration
            Registers.BCCR.Define(this)
                .WithValueField(0, 32, name: "BCCR");

            // IER @ 0x34: Interrupt Enable
            Registers.IER.Define(this)
                .WithValueField(0, 32, name: "IER");

            // ISR @ 0x38: Interrupt Status
            Registers.ISR.Define(this)
                .WithValueField(0, 32, name: "ISR");

            // ICR @ 0x3C: Interrupt Clear (write-only)
            Registers.ICR.Define(this)
                .WithValueField(0, 32, FieldMode.Write, name: "ICR");

            // LIPCR @ 0x40: Line Interrupt Position Configuration
            Registers.LIPCR.Define(this)
                .WithValueField(0, 32, name: "LIPCR");

            // CPSR @ 0x44: Current Position Status (read-only)
            Registers.CPSR.Define(this)
                .WithValueField(0, 32, FieldMode.Read, name: "CPSR");

            // CDSR @ 0x48: Current Display Status (read-only)
            Registers.CDSR.Define(this)
                .WithValueField(0, 32, FieldMode.Read, name: "CDSR");

            // Layer 0 registers @ +0x80
            // L0_CR @ 0x80: Layer 0 Control
            Registers.L0_CR.Define(this)
                .WithValueField(0, 32, name: "L0_CR");

            // L0_WHPCR @ 0x84: Layer 0 Window Horizontal Position
            Registers.L0_WHPCR.Define(this)
                .WithValueField(0, 32, name: "L0_WHPCR");

            // L0_WVPCR @ 0x88: Layer 0 Window Vertical Position
            Registers.L0_WVPCR.Define(this)
                .WithValueField(0, 32, name: "L0_WVPCR");

            // L0_CKCR @ 0x8C: Layer 0 Color Keying
            Registers.L0_CKCR.Define(this)
                .WithValueField(0, 32, name: "L0_CKCR");

            // L0_PFCR @ 0x90: Layer 0 Pixel Format
            Registers.L0_PFCR.Define(this)
                .WithValueField(0, 32, name: "L0_PFCR");

            // L0_CACR @ 0x94: Layer 0 Constant Alpha
            Registers.L0_CACR.Define(this)
                .WithValueField(0, 32, name: "L0_CACR");

            // L0_DCCR @ 0x98: Layer 0 Default Color
            Registers.L0_DCCR.Define(this)
                .WithValueField(0, 32, name: "L0_DCCR");

            // L0_BFCR @ 0x9C: Layer 0 Blending Factors
            Registers.L0_BFCR.Define(this)
                .WithValueField(0, 32, name: "L0_BFCR");

            // L0_CFBAR @ 0xAC: Layer 0 Color Frame Buffer Address
            Registers.L0_CFBAR.Define(this)
                .WithValueField(0, 32, name: "L0_CFBAR");

            // L0_CFBLNR @ 0xB0: Layer 0 Frame Buffer Line Number
            Registers.L0_CFBLNR.Define(this)
                .WithValueField(0, 32, name: "L0_CFBLNR");

            // L0_CFBLR @ 0xB4: Layer 0 Frame Buffer Line Length
            Registers.L0_CFBLR.Define(this)
                .WithValueField(0, 32, name: "L0_CFBLR");

            // L0_CLUTWR @ 0xC4: Layer 0 CLUT Write
            Registers.L0_CLUTWR.Define(this)
                .WithValueField(0, 32, name: "L0_CLUTWR");

            // Layer 1 registers @ +0x100
            // L1_CR @ 0x100: Layer 1 Control
            Registers.L1_CR.Define(this)
                .WithValueField(0, 32, name: "L1_CR");

            // L1_WHPCR @ 0x104: Layer 1 Window Horizontal Position
            Registers.L1_WHPCR.Define(this)
                .WithValueField(0, 32, name: "L1_WHPCR");

            // L1_WVPCR @ 0x108: Layer 1 Window Vertical Position
            Registers.L1_WVPCR.Define(this)
                .WithValueField(0, 32, name: "L1_WVPCR");

            // L1_CKCR @ 0x10C: Layer 1 Color Keying
            Registers.L1_CKCR.Define(this)
                .WithValueField(0, 32, name: "L1_CKCR");

            // L1_PFCR @ 0x110: Layer 1 Pixel Format
            Registers.L1_PFCR.Define(this)
                .WithValueField(0, 32, name: "L1_PFCR");

            // L1_CACR @ 0x114: Layer 1 Constant Alpha
            Registers.L1_CACR.Define(this)
                .WithValueField(0, 32, name: "L1_CACR");

            // L1_DCCR @ 0x118: Layer 1 Default Color
            Registers.L1_DCCR.Define(this)
                .WithValueField(0, 32, name: "L1_DCCR");

            // L1_BFCR @ 0x11C: Layer 1 Blending Factors
            Registers.L1_BFCR.Define(this)
                .WithValueField(0, 32, name: "L1_BFCR");

            // L1_CFBAR @ 0x12C: Layer 1 Color Frame Buffer Address
            Registers.L1_CFBAR.Define(this)
                .WithValueField(0, 32, name: "L1_CFBAR");

            // L1_CFBLNR @ 0x130: Layer 1 Frame Buffer Line Number
            Registers.L1_CFBLNR.Define(this)
                .WithValueField(0, 32, name: "L1_CFBLNR");

            // L1_CFBLR @ 0x134: Layer 1 Frame Buffer Line Length
            Registers.L1_CFBLR.Define(this)
                .WithValueField(0, 32, name: "L1_CFBLR");

            // L1_CLUTWR @ 0x144: Layer 1 CLUT Write
            Registers.L1_CLUTWR.Define(this)
                .WithValueField(0, 32, name: "L1_CLUTWR");
        }

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
            // Layer 0
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
            // Layer 1
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
