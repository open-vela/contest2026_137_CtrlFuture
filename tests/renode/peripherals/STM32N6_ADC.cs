//
// Copyright (c) 2026 CtrlFuture
//
// SPDX-License-Identifier: Apache-2.0
//
// STM32N6 ADC (Analog-to-Digital Converter) model for Renode.
// Minimal model: register read/write without actual conversions.
//
// Registers:
//   ISR   @ 0x00: Interrupt and Status Register
//   CR    @ 0x08: Control Register
//   CFGR  @ 0x0C: Configuration Register
//   SMPR1 @ 0x14: Sampling Time Register 1
//   SMPR2 @ 0x18: Sampling Time Register 2
//   TR1   @ 0x20: Watchdog Threshold Register 1
//   TR2   @ 0x24: Watchdog Threshold Register 2
//   SQR1  @ 0x30: Regular Sequence Register 1
//   SQR2  @ 0x34: Regular Sequence Register 2
//   SQR3  @ 0x38: Regular Sequence Register 3
//   SQR4  @ 0x3C: Regular Sequence Register 4
//   DR    @ 0x40: Regular Data Register
//   OFR1  @ 0x60: Offset Register 1
//   OFR2  @ 0x64: Offset Register 2
//   OFR3  @ 0x68: Offset Register 3
//   OFR4  @ 0x6C: Offset Register 4
//   JSQR  @ 0x70: Injected Sequence Register
//

using Antmicro.Renode.Core;
using Antmicro.Renode.Core.Structure.Registers;
using Antmicro.Renode.Logging;
using Antmicro.Renode.Peripherals.Bus;

namespace Antmicro.Renode.Peripherals.Miscellaneous
{
    public class STM32N6_ADC : BasicDoubleWordPeripheral, IKnownSize
    {
        public STM32N6_ADC(IMachine machine) : base(machine)
        {
            DefineRegisters();
        }

        public long Size => 0x400;

        private void DefineRegisters()
        {
            // ISR @ 0x00: Interrupt and Status Register
            Registers.ISR.Define(this)
                .WithValueField(0, 32, name: "ISR");

            // CR @ 0x08: Control Register
            Registers.CR.Define(this)
                .WithValueField(0, 32, name: "CR");

            // CFGR @ 0x0C: Configuration Register
            Registers.CFGR.Define(this)
                .WithValueField(0, 32, name: "CFGR");

            // SMPR1 @ 0x14: Sampling Time Register 1
            Registers.SMPR1.Define(this)
                .WithValueField(0, 32, name: "SMPR1");

            // SMPR2 @ 0x18: Sampling Time Register 2
            Registers.SMPR2.Define(this)
                .WithValueField(0, 32, name: "SMPR2");

            // TR1 @ 0x20: Watchdog Threshold Register 1
            Registers.TR1.Define(this)
                .WithValueField(0, 32, name: "TR1");

            // TR2 @ 0x24: Watchdog Threshold Register 2
            Registers.TR2.Define(this)
                .WithValueField(0, 32, name: "TR2");

            // SQR1 @ 0x30: Regular Sequence Register 1
            Registers.SQR1.Define(this)
                .WithValueField(0, 32, name: "SQR1");

            // SQR2 @ 0x34: Regular Sequence Register 2
            Registers.SQR2.Define(this)
                .WithValueField(0, 32, name: "SQR2");

            // SQR3 @ 0x38: Regular Sequence Register 3
            Registers.SQR3.Define(this)
                .WithValueField(0, 32, name: "SQR3");

            // SQR4 @ 0x3C: Regular Sequence Register 4
            Registers.SQR4.Define(this)
                .WithValueField(0, 32, name: "SQR4");

            // DR @ 0x40: Regular Data Register (read-only)
            Registers.DR.Define(this)
                .WithValueField(0, 32, FieldMode.Read, name: "DR");

            // OFR1 @ 0x60: Offset Register 1
            Registers.OFR1.Define(this)
                .WithValueField(0, 32, name: "OFR1");

            // OFR2 @ 0x64: Offset Register 2
            Registers.OFR2.Define(this)
                .WithValueField(0, 32, name: "OFR2");

            // OFR3 @ 0x68: Offset Register 3
            Registers.OFR3.Define(this)
                .WithValueField(0, 32, name: "OFR3");

            // OFR4 @ 0x6C: Offset Register 4
            Registers.OFR4.Define(this)
                .WithValueField(0, 32, name: "OFR4");

            // JSQR @ 0x70: Injected Sequence Register
            Registers.JSQR.Define(this)
                .WithValueField(0, 32, name: "JSQR");
        }

        private enum Registers : long
        {
            ISR   = 0x00,
            CR    = 0x08,
            CFGR  = 0x0C,
            SMPR1 = 0x14,
            SMPR2 = 0x18,
            TR1   = 0x20,
            TR2   = 0x24,
            SQR1  = 0x30,
            SQR2  = 0x34,
            SQR3  = 0x38,
            SQR4  = 0x3C,
            DR    = 0x40,
            OFR1  = 0x60,
            OFR2  = 0x64,
            OFR3  = 0x68,
            OFR4  = 0x6C,
            JSQR  = 0x70,
        }
    }
}
