//
// Copyright (c) 2026 CtrlFuture
//
// SPDX-License-Identifier: Apache-2.0
//
// STM32N6 TIM model for Renode.
// Minimal model: register read/write without actual timer counting.
//
// Registers:
//   CR1   @ 0x00: Control Register 1
//   CR2   @ 0x04: Control Register 2
//   SMCR  @ 0x08: Slave Mode Control
//   DIER  @ 0x0C: DMA/Interrupt Enable
//   SR    @ 0x10: Status Register
//   EGR   @ 0x14: Event Generation
//   CCMR1 @ 0x18: Capture/Compare Mode 1
//   CCMR2 @ 0x1C: Capture/Compare Mode 2
//   CCER  @ 0x20: Capture/Compare Enable
//   CNT   @ 0x24: Counter
//   PSC   @ 0x28: Prescaler
//   ARR   @ 0x2C: Auto-Reload
//   RCR   @ 0x30: Repetition Counter
//   CCR1  @ 0x34: Capture/Compare 1
//   CCR2  @ 0x38: Capture/Compare 2
//   CCR3  @ 0x3C: Capture/Compare 3
//   CCR4  @ 0x40: Capture/Compare 4
//   BDTR  @ 0x44: Break and Dead-Time
//   DCR   @ 0x48: DMA Control
//   DMAR  @ 0x4C: DMA Address for Full Transfer
//

using Antmicro.Renode.Core;
using Antmicro.Renode.Core.Structure.Registers;
using Antmicro.Renode.Logging;
using Antmicro.Renode.Peripherals.Bus;

namespace Antmicro.Renode.Peripherals.Miscellaneous
{
    public class STM32N6_TIM : BasicDoubleWordPeripheral, IKnownSize
    {
        public STM32N6_TIM(IMachine machine) : base(machine)
        {
            DefineRegisters();
        }

        public long Size => 0x400;

        private void DefineRegisters()
        {
            // CR1 @ 0x00: Control Register 1
            Registers.CR1.Define(this)
                .WithValueField(0, 32, name: "CR1");

            // CR2 @ 0x04: Control Register 2
            Registers.CR2.Define(this)
                .WithValueField(0, 32, name: "CR2");

            // SMCR @ 0x08: Slave Mode Control
            Registers.SMCR.Define(this)
                .WithValueField(0, 32, name: "SMCR");

            // DIER @ 0x0C: DMA/Interrupt Enable
            Registers.DIER.Define(this)
                .WithValueField(0, 32, name: "DIER");

            // SR @ 0x10: Status Register
            Registers.SR.Define(this)
                .WithValueField(0, 32, name: "SR");

            // EGR @ 0x14: Event Generation (write-only)
            Registers.EGR.Define(this)
                .WithValueField(0, 32, FieldMode.Write, name: "EGR");

            // CCMR1 @ 0x18: Capture/Compare Mode 1
            Registers.CCMR1.Define(this)
                .WithValueField(0, 32, name: "CCMR1");

            // CCMR2 @ 0x1C: Capture/Compare Mode 2
            Registers.CCMR2.Define(this)
                .WithValueField(0, 32, name: "CCMR2");

            // CCER @ 0x20: Capture/Compare Enable
            Registers.CCER.Define(this)
                .WithValueField(0, 32, name: "CCER");

            // CNT @ 0x24: Counter
            Registers.CNT.Define(this)
                .WithValueField(0, 32, name: "CNT");

            // PSC @ 0x28: Prescaler
            Registers.PSC.Define(this)
                .WithValueField(0, 32, name: "PSC");

            // ARR @ 0x2C: Auto-Reload
            Registers.ARR.Define(this)
                .WithValueField(0, 32, name: "ARR");

            // RCR @ 0x30: Repetition Counter
            Registers.RCR.Define(this)
                .WithValueField(0, 32, name: "RCR");

            // CCR1 @ 0x34: Capture/Compare 1
            Registers.CCR1.Define(this)
                .WithValueField(0, 32, name: "CCR1");

            // CCR2 @ 0x38: Capture/Compare 2
            Registers.CCR2.Define(this)
                .WithValueField(0, 32, name: "CCR2");

            // CCR3 @ 0x3C: Capture/Compare 3
            Registers.CCR3.Define(this)
                .WithValueField(0, 32, name: "CCR3");

            // CCR4 @ 0x40: Capture/Compare 4
            Registers.CCR4.Define(this)
                .WithValueField(0, 32, name: "CCR4");

            // BDTR @ 0x44: Break and Dead-Time
            Registers.BDTR.Define(this)
                .WithValueField(0, 32, name: "BDTR");

            // DCR @ 0x48: DMA Control
            Registers.DCR.Define(this)
                .WithValueField(0, 32, name: "DCR");

            // DMAR @ 0x4C: DMA Address for Full Transfer
            Registers.DMAR.Define(this)
                .WithValueField(0, 32, name: "DMAR");
        }

        private enum Registers : long
        {
            CR1 = 0x00,
            CR2 = 0x04,
            SMCR = 0x08,
            DIER = 0x0C,
            SR = 0x10,
            EGR = 0x14,
            CCMR1 = 0x18,
            CCMR2 = 0x1C,
            CCER = 0x20,
            CNT = 0x24,
            PSC = 0x28,
            ARR = 0x2C,
            RCR = 0x30,
            CCR1 = 0x34,
            CCR2 = 0x38,
            CCR3 = 0x3C,
            CCR4 = 0x40,
            BDTR = 0x44,
            DCR = 0x48,
            DMAR = 0x4C,
        }
    }
}
