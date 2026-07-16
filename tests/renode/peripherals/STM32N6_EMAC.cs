//
// Copyright (c) 2026 CtrlFuture
//
// SPDX-License-Identifier: Apache-2.0
//
// STM32N6 EMAC (Ethernet GMAC / ETH1) model for Renode.
// L2 model: DMAMR.SWR self-clears; MACMDIOAR.GB self-clears after MDIO
// access; Size covers DMA region @ 0x1000. No real MAC/PHY transfers.
//
// Registers (CMSIS ETH_TypeDef offsets):
//   MACCR     @ 0x000
//   MACFFR    @ 0x004  (MACPacketFilter)
//   MACA0HR   @ 0x300
//   MACA0LR   @ 0x304
//   MACMDIOAR @ 0x200
//   MACMDIODR @ 0x204
//   DMAMR     @ 0x1000
//

using Antmicro.Renode.Core;
using Antmicro.Renode.Core.Structure.Registers;
using Antmicro.Renode.Logging;
using Antmicro.Renode.Peripherals.Bus;

namespace Antmicro.Renode.Peripherals.Miscellaneous
{
    public class STM32N6_EMAC : BasicDoubleWordPeripheral, IKnownSize
    {
        public STM32N6_EMAC(IMachine machine) : base(machine)
        {
            DefineRegisters();
        }

        // Cover DMA mode register at 0x1000
        public long Size => 0x1200;

        public override void Reset()
        {
            base.Reset();
            maccr = 0;
            macmdioar = 0;
            macmdiodr = 0;
            dmamr = 0;
        }

        private void DefineRegisters()
        {
            Registers.MACCR.Define(this)
                .WithValueField(0, 32,
                    valueProviderCallback: _ => maccr,
                    writeCallback: (_, val) => maccr = (uint)val,
                    name: "MACCR");

            Registers.MACFFR.Define(this)
                .WithValueField(0, 32, name: "MACFFR");

            // Legacy MII aliases (older layout) retained for L1 shells
            Registers.MACMIIAR.Define(this)
                .WithValueField(0, 32, name: "MACMIIAR");

            Registers.MACMIIDR.Define(this)
                .WithValueField(0, 32, name: "MACMIIDR");

            Registers.MACFCR.Define(this)
                .WithValueField(0, 32, name: "MACFCR");

            Registers.MACVLANTR.Define(this)
                .WithValueField(0, 32, name: "MACVLANTR");

            Registers.MMCCR.Define(this)
                .WithValueField(0, 32, name: "MMCCR");

            Registers.MMCIR.Define(this)
                .WithValueField(0, 32, name: "MMCIR");

            Registers.MMCRIR.Define(this)
                .WithValueField(0, 32, name: "MMCRIR");

            Registers.MMCTIR.Define(this)
                .WithValueField(0, 32, name: "MMCTIR");

            // MACMDIOAR @ 0x200: GB (bit 0) self-clears after MDIO cycle
            Registers.MACMDIOAR.Define(this)
                .WithValueField(0, 32,
                    valueProviderCallback: _ => macmdioar,
                    writeCallback: (_, val) =>
                    {
                        // Accept write but clear GB immediately (instant MDIO)
                        macmdioar = (uint)val & ~0x1u;
                    },
                    name: "MACMDIOAR");

            Registers.MACMDIODR.Define(this)
                .WithValueField(0, 32,
                    valueProviderCallback: _ => macmdiodr,
                    writeCallback: (_, val) => macmdiodr = (uint)val,
                    name: "MACMDIODR");

            // CMSIS MACA0HR/LR @ 0x300/0x304
            Registers.MACA0HR.Define(this)
                .WithValueField(0, 32, name: "MACA0HR");

            Registers.MACA0LR.Define(this)
                .WithValueField(0, 32, name: "MACA0LR");

            // DMAMR @ 0x1000: SWR (bit 0) self-clears after soft reset
            Registers.DMAMR.Define(this)
                .WithValueField(0, 32,
                    valueProviderCallback: _ => dmamr,
                    writeCallback: (_, val) =>
                    {
                        // Soft reset completes instantly in simulation
                        dmamr = (uint)val & ~0x1u;
                    },
                    name: "DMAMR");
        }

        private uint maccr;
        private uint macmdioar;
        private uint macmdiodr;
        private uint dmamr;

        private enum Registers : long
        {
            MACCR = 0x000,
            MACFFR = 0x004,
            MACMIIAR = 0x010,
            MACMIIDR = 0x014,
            MACFCR = 0x018,
            MACVLANTR = 0x01C,
            MMCCR = 0x100,
            MMCIR = 0x104,
            MMCRIR = 0x108,
            MMCTIR = 0x10C,
            MACMDIOAR = 0x200,
            MACMDIODR = 0x204,
            MACA0HR = 0x300,
            MACA0LR = 0x304,
            DMAMR = 0x1000,
        }
    }
}
