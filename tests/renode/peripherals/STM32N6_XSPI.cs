//
// Copyright (c) 2026 CtrlFuture
//
// SPDX-License-Identifier: Apache-2.0
//
// STM32N6 XSPI (Extended SPI) model for Renode.
// Register layout matches CMSIS XSPI_TypeDef (stm32n647xx.h).
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

        private void DefineRegisters()
        {
            // CR @ 0x000: Control register
            // EN (bit 0), ABORT (bit 1), DMAEN (bit 2), TCEN (bit 3),
            // DMM (bit 6), FSEL (bit 7), FTHRES [13:8],
            // TEIE (bit 16), TCIE (bit 17), FTIE (bit 18),
            // SMIE (bit 19), TOIE (bit 20), APMS (bit 22), PMM (bit 23),
            // CSSEL (bit 24), FMODE [29:28], MSEL [31:30]
            Registers.CR.Define(this)
                .WithValueField(0, 32, name: "CR");

            // DCR1 @ 0x008: Device configuration 1
            // CKMODE (bit 0), FRCK (bit 1), CSHT [13:8],
            // DEVSIZE [20:16], EXTENDMEM (bit 21), MTYP [26:24]
            Registers.DCR1.Define(this)
                .WithValueField(0, 32, name: "DCR1");

            // DCR2 @ 0x00C: Device configuration 2
            // PRESCALER [7:0], WRAPSIZE [18:16]
            Registers.DCR2.Define(this)
                .WithValueField(0, 32, name: "DCR2");

            // DCR3 @ 0x010: Device configuration 3
            // MAXTRAN [7:0], CSBOUND [20:16]
            Registers.DCR3.Define(this)
                .WithValueField(0, 32, name: "DCR3");

            // DCR4 @ 0x014: Device configuration 4
            // REFRESH [31:0]
            Registers.DCR4.Define(this)
                .WithValueField(0, 32, name: "DCR4");

            // SR @ 0x020: Status register
            // TEF (bit 0), TCF (bit 1), FTF (bit 2), SMF (bit 3),
            // TOF (bit 4), BUSY (bit 5), FLEVEL [14:8]
            Registers.SR.Define(this)
                .WithFlag(0, FieldMode.Read, name: "TEF")
                .WithFlag(1, FieldMode.Read, name: "TCF")
                .WithFlag(2, FieldMode.Read, name: "FTF")
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
                .WithValueField(0, 32, FieldMode.Write, name: "FCR");

            // DLR @ 0x040: Data length register
            Registers.DLR.Define(this)
                .WithValueField(0, 32, name: "DLR");

            // CCR @ 0x100: Communication configuration
            // IMODE [2:0], IDTR (bit 3), ISIZE [5:4],
            // ADMODE [10:8], ADDTR (bit 11), ADSIZE [13:12],
            // ABMODE [18:16], ABDTR (bit 19), ABSIZE [21:20],
            // DMODE [26:24], DDTR (bit 27), DQSE (bit 29)
            Registers.CCR.Define(this)
                .WithValueField(0, 32, name: "CCR");

            // TCR @ 0x108: Timing configuration
            // DCYC [4:0], DHQC (bit 28), SSHIFT (bit 30)
            Registers.TCR.Define(this)
                .WithValueField(0, 32, name: "TCR");

            // IR @ 0x110: Instruction register
            Registers.IR.Define(this)
                .WithValueField(0, 32, name: "IR");

            // ABR @ 0x120: Alternate bytes register
            Registers.ABR.Define(this)
                .WithValueField(0, 32, name: "ABR");

            // LPTR @ 0x130: Low-power timeout register
            // TIMEOUT [15:0]
            Registers.LPTR.Define(this)
                .WithValueField(0, 32, name: "LPTR");
        }

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
