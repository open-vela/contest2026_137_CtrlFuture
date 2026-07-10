//
// Copyright (c) 2026 CtrlFuture
//
// SPDX-License-Identifier: Apache-2.0
//
// STM32N6 GPDMA (General Purpose DMA) model for Renode.
// Register layout matches CMSIS DMA_TypeDef / DMA_Channel_TypeDef.
//
// Global registers at base:
//   SECCFGR  @ 0x00: Secure configuration
//   PRIVCFGR @ 0x04: Privilege configuration
//   MISR     @ 0x0C: Non-secure masked interrupt status
//
// Per-channel registers (channel N base = 0x50 + N*0x80):
//   CLBAR    @ +0x00: Linked-list base address
//   CCIDCFGR @ +0x04: CID configuration
//   CFCR     @ +0x0C: Flag clear (write-1-to-clear)
//   CSR      @ +0x10: Channel status (read)
//   CCR      @ +0x14: Channel control
//

using Antmicro.Renode.Core;
using Antmicro.Renode.Core.Structure.Registers;
using Antmicro.Renode.Logging;
using Antmicro.Renode.Peripherals.Bus;

namespace Antmicro.Renode.Peripherals.Miscellaneous
{
    public class STM32N6_GPDMA : BasicDoubleWordPeripheral, IKnownSize
    {
        public STM32N6_GPDMA(IMachine machine) : base(machine)
        {
            DefineRegisters();
        }

        public long Size => 0x1000;

        private void DefineRegisters()
        {
            // SECCFGR @ 0x00: Secure configuration
            Registers.SECCFGR.Define(this)
                .WithValueField(0, 32, name: "SECCFGR");

            // PRIVCFGR @ 0x04: Privilege configuration
            Registers.PRIVCFGR.Define(this)
                .WithValueField(0, 32, name: "PRIVCFGR");

            // MISR @ 0x0C: Non-secure masked interrupt status
            Registers.MISR.Define(this)
                .WithValueField(0, 32, FieldMode.Read, name: "MISR");

            // Per-channel registers (16 channels)
            // Channel N base = 0x50 + N*0x80
            for (int ch = 0; ch < 16; ch++)
            {
                var chBase = 0x50 + 0x80 * ch;

                // CLBAR @ +0x00: Linked-list base address
                ((Registers)(chBase + 0x00)).Define(this)
                    .WithValueField(0, 32, name: $"CH{ch}_CLBAR");

                // CCIDCFGR @ +0x04: CID configuration
                ((Registers)(chBase + 0x04)).Define(this)
                    .WithValueField(0, 32, name: $"CH{ch}_CCIDCFGR");

                // CFCR @ +0x0C: Flag clear (write-1-to-clear)
                ((Registers)(chBase + 0x0C)).Define(this)
                    .WithValueField(0, 32, FieldMode.Write,
                        name: $"CH{ch}_CFCR");

                // CSR @ +0x10: Channel status
                ((Registers)(chBase + 0x10)).Define(this)
                    .WithValueField(0, 32, FieldMode.Read,
                        name: $"CH{ch}_CSR");

                // CCR @ +0x14: Channel control
                ((Registers)(chBase + 0x14)).Define(this)
                    .WithValueField(0, 32, name: $"CH{ch}_CCR");
            }
        }

        private enum Registers : long
        {
            SECCFGR = 0x00,
            PRIVCFGR = 0x04,
            MISR = 0x0C,
        }
    }
}
