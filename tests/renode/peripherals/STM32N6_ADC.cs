//
// Copyright (c) 2026 CtrlFuture
//
// SPDX-License-Identifier: Apache-2.0
//
// STM32N6 ADC (Analog-to-Digital Converter) model for Renode.
//
// L3 model: CR.ADEN sets ISR.ADRDY; CR.ADSTART "completes" a
// regular conversion synchronously (same simulation-time-collapse
// convention as STM32N6_GPDMA.cs/STM32N6_HPDMA.cs) by copying the
// externally-injected simulated sample (SIMDR, see below) into DR,
// setting ISR.EOC, and raising IRQ if IER.EOCIE is set.
//
// Registers (CMSIS ADC_TypeDef offsets, verified against
// stm32n647xx.h; STM32N657 CMSIS is byte-for-byte identical):
//   ISR    @ 0x00: Interrupt and Status Register
//   IER    @ 0x04: Interrupt Enable Register
//   CR     @ 0x08: Control Register
//   CFGR1  @ 0x0C: Configuration Register 1
//   CFGR2  @ 0x10: Configuration Register 2
//   SMPR1  @ 0x14: Sampling Time Register 1
//   SMPR2  @ 0x18: Sampling Time Register 2
//   PCSEL  @ 0x1C: Channel Preselection Register
//   SQR1   @ 0x30: Regular Sequence Register 1
//   SQR2   @ 0x34: Regular Sequence Register 2
//   SQR3   @ 0x38: Regular Sequence Register 3
//   SQR4   @ 0x3C: Regular Sequence Register 4
//   DR     @ 0x40: Regular Data Register
//   JSQR   @ 0x4C: Injected Sequence Register
//   OFR1-4 @ 0x60-0x6C: Offset Registers 1-4
//   AWD1LTR @ 0xA8: Analog Watchdog 1 Low Threshold Register
//   AWD1HTR @ 0xAC: Analog Watchdog 1 High Threshold Register
//
// Note: 0x20/0x24 (used by a previous revision of this model as
// "TR1"/"TR2") are CMSIS RESERVED1 space, not real watchdog
// threshold registers; the real thresholds are AWD1LTR/AWD1HTR at
// 0xA8/0xAC.  JSQR was also previously placed at 0x70 (that offset
// is CMSIS RESERVED3); corrected to 0x4C.
//
// SIMDR @ 0x200: team-defined, NOT a CMSIS register. The real
// CMSIS ADC_TypeDef ends at OR @ 0xD0; every offset from 0xD4
// through Size-1 (0x3FF) is genuinely unmapped on real hardware,
// so 0x200 cannot alias any current or plausible future real ADC
// register. This is the external "simulated analog input" knob a
// test can write before triggering CR.ADSTART, standing in for a
// physical voltage on the ADC input pin (which Renode has no way
// to model without a dedicated analog-signal peripheral). Absent
// a SIMDR write, DR reads back 0, matching the previous L2-state
// behavior.
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
                .WithFlag(0, out adready, name: "ADRDY")
                .WithFlag(1, name: "EOSMP")
                .WithFlag(2, out endOfConversion, name: "EOC")
                .WithValueField(3, 29, name: "ISR_REST");

            // IER @ 0x04: Interrupt Enable Register
            Registers.IER.Define(this)
                .WithFlag(0, name: "ADRDYIE")
                .WithFlag(1, name: "EOSMPIE")
                .WithFlag(2, out endOfConversionInterruptEnable, name: "EOCIE")
                .WithValueField(3, 29, name: "IER_REST");

            // CR @ 0x08: Control Register
            Registers.CR.Define(this)
                .WithFlag(0, writeCallback: (_, val) =>
                    {
                        if (val)
                        {
                            adready.Value = true;
                        }
                    }, name: "ADEN")
                .WithFlag(1, name: "ADDIS")
                .WithFlag(2, writeCallback: (_, val) =>
                    {
                        if (val && adready.Value)
                        {
                            StartConversion();
                        }
                    }, name: "ADSTART")
                .WithValueField(3, 29, name: "CR_REST");

            // CFGR1 @ 0x0C: Configuration Register 1
            Registers.CFGR1.Define(this)
                .WithValueField(0, 32, name: "CFGR1");

            // CFGR2 @ 0x10: Configuration Register 2
            Registers.CFGR2.Define(this)
                .WithValueField(0, 32, name: "CFGR2");

            // SMPR1 @ 0x14: Sampling Time Register 1
            Registers.SMPR1.Define(this)
                .WithValueField(0, 32, name: "SMPR1");

            // SMPR2 @ 0x18: Sampling Time Register 2
            Registers.SMPR2.Define(this)
                .WithValueField(0, 32, name: "SMPR2");

            // PCSEL @ 0x1C: Channel Preselection Register
            Registers.PCSEL.Define(this)
                .WithValueField(0, 32, name: "PCSEL");

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

            // DR @ 0x40: Regular Data Register (read-only). Holds
            // the last converted sample: 0 until the first
            // ADSTART-triggered conversion completes.
            Registers.DR.Define(this)
                .WithValueField(0, 32, FieldMode.Read,
                    valueProviderCallback: _ => lastConversionResult,
                    name: "DR");

            // JSQR @ 0x4C: Injected Sequence Register
            Registers.JSQR.Define(this)
                .WithValueField(0, 32, name: "JSQR");

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

            // AWD1LTR @ 0xA8: Analog Watchdog 1 Low Threshold Register
            Registers.AWD1LTR.Define(this)
                .WithValueField(0, 32, name: "AWD1LTR");

            // AWD1HTR @ 0xAC: Analog Watchdog 1 High Threshold Register
            Registers.AWD1HTR.Define(this)
                .WithValueField(0, 32, name: "AWD1HTR");

            // SIMDR @ 0x200: not a CMSIS register, see header note.
            Registers.SIMDR.Define(this)
                .WithValueField(0, 32, writeCallback: (_, val) =>
                    {
                        simulatedSample = (uint)val;
                    },
                    valueProviderCallback: _ => simulatedSample,
                    name: "SIMDR");
        }

        private void StartConversion()
        {
            lastConversionResult = simulatedSample;
            endOfConversion.Value = true;
            if (endOfConversionInterruptEnable.Value)
            {
                IRQ.Set(true);
            }
        }

        public GPIO IRQ { get; } = new GPIO();

        private IFlagRegisterField adready;
        private IFlagRegisterField endOfConversion;
        private IFlagRegisterField endOfConversionInterruptEnable;
        private uint simulatedSample;
        private uint lastConversionResult;

        private enum Registers : long
        {
            ISR     = 0x00,
            IER     = 0x04,
            CR      = 0x08,
            CFGR1   = 0x0C,
            CFGR2   = 0x10,
            SMPR1   = 0x14,
            SMPR2   = 0x18,
            PCSEL   = 0x1C,
            SQR1    = 0x30,
            SQR2    = 0x34,
            SQR3    = 0x38,
            SQR4    = 0x3C,
            DR      = 0x40,
            JSQR    = 0x4C,
            OFR1    = 0x60,
            OFR2    = 0x64,
            OFR3    = 0x68,
            OFR4    = 0x6C,
            AWD1LTR = 0xA8,
            AWD1HTR = 0xAC,
            SIMDR   = 0x200,
        }
    }
}
