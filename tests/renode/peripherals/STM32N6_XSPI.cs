//
// Copyright (c) 2026 CtrlFuture
//
// SPDX-License-Identifier: Apache-2.0
//
// STM32N6 XSPI (Extended SPI) model for Renode.
// L2 model: CR.EN sticks; BUSY always clear; writing IR (or CCR when
// instruction mode requires) completes immediately with TCF; FCR clears
// flags. Enough for driver xspi_wait_tc / xspi_wait_idle polls.
//
// Registers:
//   CR   @ 0x000: Control register
//   DCR1 @ 0x008: Device configuration 1
//   DCR2 @ 0x00C: Device configuration 2
//   DCR3 @ 0x010: Device configuration 3
//   DCR4 @ 0x014: Device configuration 4
//   SR   @ 0x020: Status register
//   FCR  @ 0x024: Flag clear register
//   DLR  @ 0x040: Data length register
//   CCR  @ 0x100: Communication configuration
//   TCR  @ 0x108: Timing configuration
//   IR   @ 0x110: Instruction register
//   ABR  @ 0x120: Alternate bytes register
//   LPTR @ 0x130: Low-power timeout register
//

using Antmicro.Renode.Core;
using Antmicro.Renode.Core.Structure.Registers;
using Antmicro.Renode.Logging;
using Antmicro.Renode.Peripherals.Bus;

namespace Antmicro.Renode.Peripherals.Miscellaneous
{
    public class STM32N6_XSPI : BasicDoubleWordPeripheral, IKnownSize
    {
        public STM32N6_XSPI(IMachine machine) : base(machine)
        {
            DefineRegisters();
        }

        public long Size => 0x1000;

        public override void Reset()
        {
            base.Reset();
            crReg = 0;
            dcr1Reg = 0;
            tcf = false;
            tef = false;
            ftf = false;
        }

        private void CompleteTransfer()
        {
            // Instant complete in simulation when peripheral is enabled
            if ((crReg & 0x1) != 0)
            {
                tcf = true;
                ftf = true;
            }
        }

        private void DefineRegisters()
        {
            // CR @ 0x000: Control register
            Registers.CR.Define(this)
                .WithValueField(0, 32,
                    valueProviderCallback: _ => crReg,
                    writeCallback: (_, val) => crReg = (uint)val,
                    name: "CR");

            Registers.DCR1.Define(this)
                .WithValueField(0, 32,
                    valueProviderCallback: _ => dcr1Reg,
                    writeCallback: (_, val) => dcr1Reg = (uint)val,
                    name: "DCR1");

            Registers.DCR2.Define(this)
                .WithValueField(0, 32, name: "DCR2");

            Registers.DCR3.Define(this)
                .WithValueField(0, 32, name: "DCR3");

            Registers.DCR4.Define(this)
                .WithValueField(0, 32, name: "DCR4");

            // SR @ 0x020: Status register
            // TEF (bit 0), TCF (bit 1), FTF (bit 2), SMF (bit 3),
            // TOF (bit 4), BUSY (bit 5), FLEVEL [14:8]
            Registers.SR.Define(this)
                .WithFlag(0, FieldMode.Read,
                    valueProviderCallback: _ => tef, name: "TEF")
                .WithFlag(1, FieldMode.Read,
                    valueProviderCallback: _ => tcf, name: "TCF")
                .WithFlag(2, FieldMode.Read,
                    valueProviderCallback: _ => ftf, name: "FTF")
                .WithFlag(3, FieldMode.Read, name: "SMF")
                .WithFlag(4, FieldMode.Read, name: "TOF")
                .WithFlag(5, FieldMode.Read,
                    valueProviderCallback: _ => false, name: "BUSY")
                .WithReservedBits(6, 2)
                .WithValueField(8, 7, FieldMode.Read, name: "FLEVEL")
                .WithReservedBits(15, 17);

            // FCR @ 0x024: Flag clear register (write-1-to-clear)
            // CTEF (bit 0), CTCF (bit 1), CSMF (bit 3), CTOF (bit 4)
            Registers.FCR.Define(this)
                .WithValueField(0, 32, FieldMode.Write, name: "FCR",
                    writeCallback: (_, val) =>
                    {
                        if ((val & 0x1) != 0)
                        {
                            tef = false;
                        }
                        if ((val & 0x2) != 0)
                        {
                            tcf = false;
                        }
                        // FTF is not cleared by FCR on HW (level flag)
                    });

            Registers.DLR.Define(this)
                .WithValueField(0, 32, name: "DLR");

            // CCR write may start a transfer depending on FMODE; L2: complete
            Registers.CCR.Define(this)
                .WithValueField(0, 32, name: "CCR",
                    writeCallback: (_, __) => CompleteTransfer());

            Registers.TCR.Define(this)
                .WithValueField(0, 32, name: "TCR");

            // IR write commonly triggers the command phase
            Registers.IR.Define(this)
                .WithValueField(0, 32, name: "IR",
                    writeCallback: (_, __) => CompleteTransfer());

            Registers.ABR.Define(this)
                .WithValueField(0, 32, name: "ABR");

            Registers.LPTR.Define(this)
                .WithValueField(0, 32, name: "LPTR");
        }

        private uint crReg;
        private uint dcr1Reg;
        private bool tcf;
        private bool tef;
        private bool ftf;

        private enum Registers : long
        {
            CR = 0x000,
            DCR1 = 0x008,
            DCR2 = 0x00C,
            DCR3 = 0x010,
            DCR4 = 0x014,
            SR = 0x020,
            FCR = 0x024,
            DLR = 0x040,
            CCR = 0x100,
            TCR = 0x108,
            IR = 0x110,
            ABR = 0x120,
            LPTR = 0x130,
        }
    }
}
