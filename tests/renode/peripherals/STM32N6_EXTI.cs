//
// Copyright (c) 2026 CtrlFuture
//
// SPDX-License-Identifier: Apache-2.0
//
// STM32N6 EXTI (Extended Interrupt and Event) model for Renode.
// Register layout matches CMSIS EXTI_TypeDef (stm32n647xx.h).
//
// Bank 1 (lines 0-21):
//   RTSR1    @ 0x00: Rising trigger selection
//   FTSR1    @ 0x04: Falling trigger selection
//   SWIER1   @ 0x08: Software interrupt event
//   RPR1     @ 0x0C: Rising pending (write-1-to-clear)
//   FPR1     @ 0x10: Falling pending (write-1-to-clear)
//   SECCFGR1 @ 0x14: Security configuration
//   PRIVCFGR1 @ 0x18: Privilege configuration
//
// Bank 2 (lines 32+):
//   RTSR2    @ 0x20: Rising trigger selection
//   FTSR2    @ 0x24: Falling trigger selection
//   SWIER2   @ 0x28: Software interrupt event
//   RPR2     @ 0x2C: Rising pending
//   FPR2     @ 0x30: Falling pending
//   SECCFGR2 @ 0x34: Security configuration
//   PRIVCFGR2 @ 0x38: Privilege configuration
//
// Bank 3 (lines 64+):
//   RTSR3    @ 0x40: Rising trigger selection
//   FTSR3    @ 0x44: Falling trigger selection
//   SWIER3   @ 0x48: Software interrupt event
//   RPR3     @ 0x4C: Rising pending
//   FPR3     @ 0x50: Falling pending
//   SECCFGR3 @ 0x54: Security configuration
//   PRIVCFGR3 @ 0x58: Privilege configuration
//
// EXTICR[4] @ 0x60-0x6C: External interrupt configuration
// LOCKR     @ 0x70: Lock register
//
// IMR1 @ 0x80: Interrupt mask bank 1
// EMR1 @ 0x84: Event mask bank 1
// IMR2 @ 0x90: Interrupt mask bank 2
// EMR2 @ 0x94: Event mask bank 2
// IMR3 @ 0xA0: Interrupt mask bank 3
// EMR3 @ 0xA4: Event mask bank 3
//

using Antmicro.Renode.Core;
using Antmicro.Renode.Core.Structure.Registers;
using Antmicro.Renode.Logging;
using Antmicro.Renode.Peripherals.Bus;

namespace Antmicro.Renode.Peripherals.Miscellaneous
{
    public class STM32N6_EXTI : BasicDoubleWordPeripheral, IKnownSize
    {
        public STM32N6_EXTI(IMachine machine) : base(machine)
        {
            DefineRegisters();
        }

        public long Size => 0x200;

        private void DefineRegisters()
        {
            // === Bank 1 (lines 0-21) ===

            // RTSR1 @ 0x00: Rising trigger selection
            Registers.RTSR1.Define(this)
                .WithValueField(0, 32, name: "RTSR1");

            // FTSR1 @ 0x04: Falling trigger selection
            Registers.FTSR1.Define(this)
                .WithValueField(0, 32, name: "FTSR1");

            // SWIER1 @ 0x08: Software interrupt event
            Registers.SWIER1.Define(this)
                .WithValueField(0, 32, name: "SWIER1");

            // RPR1 @ 0x0C: Rising pending (write-1-to-clear)
            Registers.RPR1.Define(this)
                .WithValueField(0, 32, name: "RPR1");

            // FPR1 @ 0x10: Falling pending (write-1-to-clear)
            Registers.FPR1.Define(this)
                .WithValueField(0, 32, name: "FPR1");

            // SECCFGR1 @ 0x14: Security configuration
            Registers.SECCFGR1.Define(this)
                .WithValueField(0, 32, name: "SECCFGR1");

            // PRIVCFGR1 @ 0x18: Privilege configuration
            Registers.PRIVCFGR1.Define(this)
                .WithValueField(0, 32, name: "PRIVCFGR1");

            // === Bank 2 (lines 32+) ===

            // RTSR2 @ 0x20
            Registers.RTSR2.Define(this)
                .WithValueField(0, 32, name: "RTSR2");

            // FTSR2 @ 0x24
            Registers.FTSR2.Define(this)
                .WithValueField(0, 32, name: "FTSR2");

            // SWIER2 @ 0x28
            Registers.SWIER2.Define(this)
                .WithValueField(0, 32, name: "SWIER2");

            // RPR2 @ 0x2C
            Registers.RPR2.Define(this)
                .WithValueField(0, 32, name: "RPR2");

            // FPR2 @ 0x30
            Registers.FPR2.Define(this)
                .WithValueField(0, 32, name: "FPR2");

            // SECCFGR2 @ 0x34
            Registers.SECCFGR2.Define(this)
                .WithValueField(0, 32, name: "SECCFGR2");

            // PRIVCFGR2 @ 0x38
            Registers.PRIVCFGR2.Define(this)
                .WithValueField(0, 32, name: "PRIVCFGR2");

            // === Bank 3 (lines 64+) ===

            // RTSR3 @ 0x40
            Registers.RTSR3.Define(this)
                .WithValueField(0, 32, name: "RTSR3");

            // FTSR3 @ 0x44
            Registers.FTSR3.Define(this)
                .WithValueField(0, 32, name: "FTSR3");

            // SWIER3 @ 0x48
            Registers.SWIER3.Define(this)
                .WithValueField(0, 32, name: "SWIER3");

            // RPR3 @ 0x4C
            Registers.RPR3.Define(this)
                .WithValueField(0, 32, name: "RPR3");

            // FPR3 @ 0x50
            Registers.FPR3.Define(this)
                .WithValueField(0, 32, name: "FPR3");

            // SECCFGR3 @ 0x54
            Registers.SECCFGR3.Define(this)
                .WithValueField(0, 32, name: "SECCFGR3");

            // PRIVCFGR3 @ 0x58
            Registers.PRIVCFGR3.Define(this)
                .WithValueField(0, 32, name: "PRIVCFGR3");

            // === EXTICR[4] @ 0x60-0x6C ===

            // EXTICR[0] @ 0x60
            Registers.EXTICR0.Define(this)
                .WithValueField(0, 32, name: "EXTICR0");

            // EXTICR[1] @ 0x64
            Registers.EXTICR1.Define(this)
                .WithValueField(0, 32, name: "EXTICR1");

            // EXTICR[2] @ 0x68
            Registers.EXTICR2.Define(this)
                .WithValueField(0, 32, name: "EXTICR2");

            // EXTICR[3] @ 0x6C
            Registers.EXTICR3.Define(this)
                .WithValueField(0, 32, name: "EXTICR3");

            // LOCKR @ 0x70: Lock register
            Registers.LOCKR.Define(this)
                .WithValueField(0, 32, name: "LOCKR");

            // === Mask registers ===

            // IMR1 @ 0x80: Interrupt mask bank 1
            Registers.IMR1.Define(this)
                .WithValueField(0, 32, name: "IMR1");

            // EMR1 @ 0x84: Event mask bank 1
            Registers.EMR1.Define(this)
                .WithValueField(0, 32, name: "EMR1");

            // IMR2 @ 0x90: Interrupt mask bank 2
            Registers.IMR2.Define(this)
                .WithValueField(0, 32, name: "IMR2");

            // EMR2 @ 0x94: Event mask bank 2
            Registers.EMR2.Define(this)
                .WithValueField(0, 32, name: "EMR2");

            // IMR3 @ 0xA0: Interrupt mask bank 3
            Registers.IMR3.Define(this)
                .WithValueField(0, 32, name: "IMR3");

            // EMR3 @ 0xA4: Event mask bank 3
            Registers.EMR3.Define(this)
                .WithValueField(0, 32, name: "EMR3");
        }

        private enum Registers : long
        {
            // Bank 1
            RTSR1 = 0x00,
            FTSR1 = 0x04,
            SWIER1 = 0x08,
            RPR1 = 0x0C,
            FPR1 = 0x10,
            SECCFGR1 = 0x14,
            PRIVCFGR1 = 0x18,
            // Bank 2
            RTSR2 = 0x20,
            FTSR2 = 0x24,
            SWIER2 = 0x28,
            RPR2 = 0x2C,
            FPR2 = 0x30,
            SECCFGR2 = 0x34,
            PRIVCFGR2 = 0x38,
            // Bank 3
            RTSR3 = 0x40,
            FTSR3 = 0x44,
            SWIER3 = 0x48,
            RPR3 = 0x4C,
            FPR3 = 0x50,
            SECCFGR3 = 0x54,
            PRIVCFGR3 = 0x58,
            // EXTICR
            EXTICR0 = 0x60,
            EXTICR1 = 0x64,
            EXTICR2 = 0x68,
            EXTICR3 = 0x6C,
            // Lock
            LOCKR = 0x70,
            // Mask registers
            IMR1 = 0x80,
            EMR1 = 0x84,
            IMR2 = 0x90,
            EMR2 = 0x94,
            IMR3 = 0xA0,
            EMR3 = 0xA4,
        }
    }
}
