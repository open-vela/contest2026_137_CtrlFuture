//
// Copyright (c) 2026 CtrlFuture
//
// SPDX-License-Identifier: Apache-2.0
//
// STM32N6 DMA2D (Chrom-ART Graphics Accelerator) model for Renode.
// Minimal model: register read/write without actual graphics operations.
//
// Registers:
//   CR      @ 0x00: Control Register
//   ISR     @ 0x04: Interrupt Status Register
//   IFCR    @ 0x08: Interrupt Flag Clear Register
//   FGMAR   @ 0x0C: Foreground Memory Address Register
//   FGOR    @ 0x10: Foreground Offset Register
//   BGMAR   @ 0x14: Background Memory Address Register
//   BGOR    @ 0x18: Background Offset Register
//   FGPFCCR @ 0x1C: Foreground PFC Control Register
//   FGCOLR  @ 0x20: Foreground Color Register
//   BGPFCCR @ 0x24: Background PFC Control Register
//   BGCOLR  @ 0x28: Background Color Register
//   FGCMAR  @ 0x2C: Foreground CLUT Memory Address Register
//   BGCMAR  @ 0x30: Background CLUT Memory Address Register
//   OPFCCR  @ 0x34: Output PFC Control Register
//   OCOLR   @ 0x38: Output Color Register
//   OMAR    @ 0x3C: Output Memory Address Register
//   OOR     @ 0x40: Output Offset Register
//   NLR     @ 0x44: Number of Line Register
//   LWR     @ 0x48: Line Watermark Register
//   AMTCR   @ 0x4C: AHB Master Timer Configuration Register
//

using Antmicro.Renode.Core;
using Antmicro.Renode.Core.Structure.Registers;
using Antmicro.Renode.Logging;
using Antmicro.Renode.Peripherals.Bus;

namespace Antmicro.Renode.Peripherals.Miscellaneous
{
    public class STM32N6_DMA2D : BasicDoubleWordPeripheral, IKnownSize
    {
        public STM32N6_DMA2D(IMachine machine) : base(machine)
        {
            DefineRegisters();
        }

        public long Size => 0x400;

        private void DefineRegisters()
        {
            // CR @ 0x00: Control Register
            Registers.CR.Define(this)
                .WithValueField(0, 32, name: "CR");

            // ISR @ 0x04: Interrupt Status Register (read-only)
            Registers.ISR.Define(this)
                .WithValueField(0, 32, FieldMode.Read, name: "ISR");

            // IFCR @ 0x08: Interrupt Flag Clear Register (write-only)
            Registers.IFCR.Define(this)
                .WithValueField(0, 32, FieldMode.Write, name: "IFCR");

            // FGMAR @ 0x0C: Foreground Memory Address Register
            Registers.FGMAR.Define(this)
                .WithValueField(0, 32, name: "FGMAR");

            // FGOR @ 0x10: Foreground Offset Register
            Registers.FGOR.Define(this)
                .WithValueField(0, 32, name: "FGOR");

            // BGMAR @ 0x14: Background Memory Address Register
            Registers.BGMAR.Define(this)
                .WithValueField(0, 32, name: "BGMAR");

            // BGOR @ 0x18: Background Offset Register
            Registers.BGOR.Define(this)
                .WithValueField(0, 32, name: "BGOR");

            // FGPFCCR @ 0x1C: Foreground PFC Control Register
            Registers.FGPFCCR.Define(this)
                .WithValueField(0, 32, name: "FGPFCCR");

            // FGCOLR @ 0x20: Foreground Color Register
            Registers.FGCOLR.Define(this)
                .WithValueField(0, 32, name: "FGCOLR");

            // BGPFCCR @ 0x24: Background PFC Control Register
            Registers.BGPFCCR.Define(this)
                .WithValueField(0, 32, name: "BGPFCCR");

            // BGCOLR @ 0x28: Background Color Register
            Registers.BGCOLR.Define(this)
                .WithValueField(0, 32, name: "BGCOLR");

            // FGCMAR @ 0x2C: Foreground CLUT Memory Address Register
            Registers.FGCMAR.Define(this)
                .WithValueField(0, 32, name: "FGCMAR");

            // BGCMAR @ 0x30: Background CLUT Memory Address Register
            Registers.BGCMAR.Define(this)
                .WithValueField(0, 32, name: "BGCMAR");

            // OPFCCR @ 0x34: Output PFC Control Register
            Registers.OPFCCR.Define(this)
                .WithValueField(0, 32, name: "OPFCCR");

            // OCOLR @ 0x38: Output Color Register
            Registers.OCOLR.Define(this)
                .WithValueField(0, 32, name: "OCOLR");

            // OMAR @ 0x3C: Output Memory Address Register
            Registers.OMAR.Define(this)
                .WithValueField(0, 32, name: "OMAR");

            // OOR @ 0x40: Output Offset Register
            Registers.OOR.Define(this)
                .WithValueField(0, 32, name: "OOR");

            // NLR @ 0x44: Number of Line Register
            Registers.NLR.Define(this)
                .WithValueField(0, 32, name: "NLR");

            // LWR @ 0x48: Line Watermark Register
            Registers.LWR.Define(this)
                .WithValueField(0, 32, name: "LWR");

            // AMTCR @ 0x4C: AHB Master Timer Configuration Register
            Registers.AMTCR.Define(this)
                .WithValueField(0, 32, name: "AMTCR");
        }

        private enum Registers : long
        {
            CR      = 0x00,
            ISR     = 0x04,
            IFCR    = 0x08,
            FGMAR   = 0x0C,
            FGOR    = 0x10,
            BGMAR   = 0x14,
            BGOR    = 0x18,
            FGPFCCR = 0x1C,
            FGCOLR  = 0x20,
            BGPFCCR = 0x24,
            BGCOLR  = 0x28,
            FGCMAR  = 0x2C,
            BGCMAR  = 0x30,
            OPFCCR  = 0x34,
            OCOLR   = 0x38,
            OMAR    = 0x3C,
            OOR     = 0x40,
            NLR     = 0x44,
            LWR     = 0x48,
            AMTCR   = 0x4C,
        }
    }
}
