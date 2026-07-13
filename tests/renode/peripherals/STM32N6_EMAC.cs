//
// Copyright (c) 2026 CtrlFuture
//
// SPDX-License-Identifier: Apache-2.0
//
// STM32N6 EMAC (Ethernet GMAC) model for Renode.
// Minimal model: register read/write without actual Ethernet transfers.
//
// Registers:
//   MACCR    @ 0x000: MAC configuration
//   MACFFR   @ 0x004: MAC frame filter
//   MACMIIAR @ 0x010: MII address
//   MACMIIDR @ 0x014: MII data
//   MACFCR   @ 0x018: MAC flow control
//   MACVLANTR @ 0x01C: VLAN tag
//   MACA0HR  @ 0x040: Address 0 high
//   MACA0LR  @ 0x044: Address 0 low
//   MMCCR    @ 0x100: MMC control
//   MMCIR    @ 0x104: MMC interrupt
//   MMCRIR   @ 0x108: MMC receive interrupt
//   MMCTIR   @ 0x10C: MMC transmit interrupt
//   DMABMR   @ 0x1000: DMA bus mode
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

        public long Size => 0x800;

        private void DefineRegisters()
        {
            // MACCR @ 0x000: MAC configuration
            Registers.MACCR.Define(this)
                .WithValueField(0, 32, name: "MACCR");

            // MACFFR @ 0x004: MAC frame filter
            Registers.MACFFR.Define(this)
                .WithValueField(0, 32, name: "MACFFR");

            // MACMIIAR @ 0x010: MII address
            Registers.MACMIIAR.Define(this)
                .WithValueField(0, 32, name: "MACMIIAR");

            // MACMIIDR @ 0x014: MII data
            Registers.MACMIIDR.Define(this)
                .WithValueField(0, 32, name: "MACMIIDR");

            // MACFCR @ 0x018: MAC flow control
            Registers.MACFCR.Define(this)
                .WithValueField(0, 32, name: "MACFCR");

            // MACVLANTR @ 0x01C: VLAN tag
            Registers.MACVLANTR.Define(this)
                .WithValueField(0, 32, name: "MACVLANTR");

            // MACA0HR @ 0x040: Address 0 high
            Registers.MACA0HR.Define(this)
                .WithValueField(0, 32, name: "MACA0HR");

            // MACA0LR @ 0x044: Address 0 low
            Registers.MACA0LR.Define(this)
                .WithValueField(0, 32, name: "MACA0LR");

            // MMCCR @ 0x100: MMC control
            Registers.MMCCR.Define(this)
                .WithValueField(0, 32, name: "MMCCR");

            // MMCIR @ 0x104: MMC interrupt
            Registers.MMCIR.Define(this)
                .WithValueField(0, 32, name: "MMCIR");

            // MMCRIR @ 0x108: MMC receive interrupt
            Registers.MMCRIR.Define(this)
                .WithValueField(0, 32, name: "MMCRIR");

            // MMCTIR @ 0x10C: MMC transmit interrupt
            Registers.MMCTIR.Define(this)
                .WithValueField(0, 32, name: "MMCTIR");

            // DMABMR @ 0x1000: DMA bus mode
            Registers.DMABMR.Define(this)
                .WithValueField(0, 32, name: "DMABMR");
        }

        private enum Registers : long
        {
            MACCR = 0x000,
            MACFFR = 0x004,
            MACMIIAR = 0x010,
            MACMIIDR = 0x014,
            MACFCR = 0x018,
            MACVLANTR = 0x01C,
            MACA0HR = 0x040,
            MACA0LR = 0x044,
            MMCCR = 0x100,
            MMCIR = 0x104,
            MMCRIR = 0x108,
            MMCTIR = 0x10C,
            DMABMR = 0x1000,
        }
    }
}
