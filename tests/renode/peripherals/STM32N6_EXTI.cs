//
// Copyright (c) 2026 CtrlFuture
//
// SPDX-License-Identifier: Apache-2.0
//
// STM32N6 EXTI (Extended Interrupt and Event) model for Renode.
// L2 model: SWIER sets rising pending (RPR); RPR/FPR write-1-to-clear.
// Residual: full GPIO edge path and NVIC line routing not modeled.
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
// Bank 2/3: same layout at +0x20 / +0x40
// EXTICR[4] @ 0x60-0x6C, LOCKR @ 0x70
// IMR/EMR banks @ 0x80+
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

        public override void Reset()
        {
            base.Reset();
            for (var i = 0; i < BankCount; i++)
            {
                rtsr[i] = 0;
                ftsr[i] = 0;
                rpr[i] = 0;
                fpr[i] = 0;
            }
        }

        private void DefineBank(int bank, Registers rtsrReg, Registers ftsrReg,
            Registers swierReg, Registers rprReg, Registers fprReg,
            Registers seccfgrReg, Registers privcfgrReg)
        {
            var b = bank;

            rtsrReg.Define(this)
                .WithValueField(0, 32,
                    valueProviderCallback: _ => rtsr[b],
                    writeCallback: (_, val) => rtsr[b] = (uint)val,
                    name: $"RTSR{b + 1}");

            ftsrReg.Define(this)
                .WithValueField(0, 32,
                    valueProviderCallback: _ => ftsr[b],
                    writeCallback: (_, val) => ftsr[b] = (uint)val,
                    name: $"FTSR{b + 1}");

            // SWIER: set RPR for rising-selected bits, FPR for falling-selected.
            // Hardware auto-clears SWIER when pending is set; leave read 0.
            swierReg.Define(this)
                .WithValueField(0, 32, FieldMode.Write,
                    writeCallback: (_, val) =>
                    {
                        var bits = (uint)val;
                        rpr[b] |= bits & rtsr[b];
                        fpr[b] |= bits & ftsr[b];
                        // If neither edge selected, still raise RPR (SWI path)
                        var unselected = bits & ~(rtsr[b] | ftsr[b]);
                        rpr[b] |= unselected;
                    },
                    name: $"SWIER{b + 1}");

            // RPR: write-1-to-clear
            rprReg.Define(this)
                .WithValueField(0, 32,
                    valueProviderCallback: _ => rpr[b],
                    writeCallback: (_, val) => rpr[b] &= ~(uint)val,
                    name: $"RPR{b + 1}");

            // FPR: write-1-to-clear
            fprReg.Define(this)
                .WithValueField(0, 32,
                    valueProviderCallback: _ => fpr[b],
                    writeCallback: (_, val) => fpr[b] &= ~(uint)val,
                    name: $"FPR{b + 1}");

            seccfgrReg.Define(this)
                .WithValueField(0, 32, name: $"SECCFGR{b + 1}");

            privcfgrReg.Define(this)
                .WithValueField(0, 32, name: $"PRIVCFGR{b + 1}");
        }

        private void DefineRegisters()
        {
            DefineBank(0,
                Registers.RTSR1, Registers.FTSR1, Registers.SWIER1,
                Registers.RPR1, Registers.FPR1,
                Registers.SECCFGR1, Registers.PRIVCFGR1);
            DefineBank(1,
                Registers.RTSR2, Registers.FTSR2, Registers.SWIER2,
                Registers.RPR2, Registers.FPR2,
                Registers.SECCFGR2, Registers.PRIVCFGR2);
            DefineBank(2,
                Registers.RTSR3, Registers.FTSR3, Registers.SWIER3,
                Registers.RPR3, Registers.FPR3,
                Registers.SECCFGR3, Registers.PRIVCFGR3);

            Registers.EXTICR0.Define(this)
                .WithValueField(0, 32, name: "EXTICR0");
            Registers.EXTICR1.Define(this)
                .WithValueField(0, 32, name: "EXTICR1");
            Registers.EXTICR2.Define(this)
                .WithValueField(0, 32, name: "EXTICR2");
            Registers.EXTICR3.Define(this)
                .WithValueField(0, 32, name: "EXTICR3");

            Registers.LOCKR.Define(this)
                .WithValueField(0, 32, name: "LOCKR");

            Registers.IMR1.Define(this)
                .WithValueField(0, 32, name: "IMR1");
            Registers.EMR1.Define(this)
                .WithValueField(0, 32, name: "EMR1");
            Registers.IMR2.Define(this)
                .WithValueField(0, 32, name: "IMR2");
            Registers.EMR2.Define(this)
                .WithValueField(0, 32, name: "EMR2");
            Registers.IMR3.Define(this)
                .WithValueField(0, 32, name: "IMR3");
            Registers.EMR3.Define(this)
                .WithValueField(0, 32, name: "EMR3");
        }

        private const int BankCount = 3;
        private readonly uint[] rtsr = new uint[BankCount];
        private readonly uint[] ftsr = new uint[BankCount];
        private readonly uint[] rpr = new uint[BankCount];
        private readonly uint[] fpr = new uint[BankCount];

        private enum Registers : long
        {
            RTSR1 = 0x00,
            FTSR1 = 0x04,
            SWIER1 = 0x08,
            RPR1 = 0x0C,
            FPR1 = 0x10,
            SECCFGR1 = 0x14,
            PRIVCFGR1 = 0x18,
            RTSR2 = 0x20,
            FTSR2 = 0x24,
            SWIER2 = 0x28,
            RPR2 = 0x2C,
            FPR2 = 0x30,
            SECCFGR2 = 0x34,
            PRIVCFGR2 = 0x38,
            RTSR3 = 0x40,
            FTSR3 = 0x44,
            SWIER3 = 0x48,
            RPR3 = 0x4C,
            FPR3 = 0x50,
            SECCFGR3 = 0x54,
            PRIVCFGR3 = 0x58,
            EXTICR0 = 0x60,
            EXTICR1 = 0x64,
            EXTICR2 = 0x68,
            EXTICR3 = 0x6C,
            LOCKR = 0x70,
            IMR1 = 0x80,
            EMR1 = 0x84,
            IMR2 = 0x90,
            EMR2 = 0x94,
            IMR3 = 0xA0,
            EMR3 = 0xA4,
        }
    }
}
