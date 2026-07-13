//
// Copyright (c) 2026 CtrlFuture
//
// SPDX-License-Identifier: Apache-2.0
//
// STM32N6 HPDMA model for Renode.
// Minimal model: register read/write without actual DMA transfers.
// Structure similar to GPDMA but with 2D transfer support.
//
// Registers:
//   SECCFGR @ 0x00: Security Configuration
//   PRIVCFGR @ 0x04: Privilege Configuration
//   MISR    @ 0x08: Masked Interrupt Status
//   CLBAR   @ 0x50: Channel Linked-list Base Address
//   CC      @ 0x54: Channel Control
//   CTR2    @ 0x58: Channel Transfer Register 2
//   CBR1    @ 0x5C: Channel Block Register 1
//   CSAR    @ 0x60: Channel Source Address
//   CDAR    @ 0x64: Channel Destination Address
//   CTR3    @ 0x68: Channel Transfer Register 3 (2D)
//   CBR2    @ 0x6C: Channel Block Register 2 (2D)
//   CLLR    @ 0x70: Channel Linked-list Register
//

using Antmicro.Renode.Core;
using Antmicro.Renode.Core.Structure.Registers;
using Antmicro.Renode.Logging;
using Antmicro.Renode.Peripherals.Bus;

namespace Antmicro.Renode.Peripherals.Miscellaneous
{
    public class STM32N6_HPDMA : BasicDoubleWordPeripheral, IKnownSize
    {
        public STM32N6_HPDMA(IMachine machine) : base(machine)
        {
            DefineRegisters();
        }

        public long Size => 0x400;

        private void DefineRegisters()
        {
            // SECCFGR @ 0x00: Security Configuration
            Registers.SECCFGR.Define(this)
                .WithValueField(0, 32, name: "SECCFGR");

            // PRIVCFGR @ 0x04: Privilege Configuration
            Registers.PRIVCFGR.Define(this)
                .WithValueField(0, 32, name: "PRIVCFGR");

            // MISR @ 0x08: Masked Interrupt Status
            Registers.MISR.Define(this)
                .WithValueField(0, 32, FieldMode.Read, name: "MISR");

            // Channel 0 registers @ 0x50
            // CLBAR @ 0x50: Channel Linked-list Base Address
            Registers.CLBAR.Define(this)
                .WithValueField(0, 32, name: "CLBAR");

            // CC @ 0x54: Channel Control
            Registers.CC.Define(this)
                .WithValueField(0, 32, name: "CC");

            // CTR2 @ 0x58: Channel Transfer Register 2
            Registers.CTR2.Define(this)
                .WithValueField(0, 32, name: "CTR2");

            // CBR1 @ 0x5C: Channel Block Register 1
            Registers.CBR1.Define(this)
                .WithValueField(0, 32, name: "CBR1");

            // CSAR @ 0x60: Channel Source Address
            Registers.CSAR.Define(this)
                .WithValueField(0, 32, name: "CSAR");

            // CDAR @ 0x64: Channel Destination Address
            Registers.CDAR.Define(this)
                .WithValueField(0, 32, name: "CDAR");

            // CTR3 @ 0x68: Channel Transfer Register 3 (2D)
            Registers.CTR3.Define(this)
                .WithValueField(0, 32, name: "CTR3");

            // CBR2 @ 0x6C: Channel Block Register 2 (2D)
            Registers.CBR2.Define(this)
                .WithValueField(0, 32, name: "CBR2");

            // CLLR @ 0x70: Channel Linked-list Register
            Registers.CLLR.Define(this)
                .WithValueField(0, 32, name: "CLLR");
        }

        private enum Registers : long
        {
            SECCFGR = 0x00,
            PRIVCFGR = 0x04,
            MISR = 0x08,
            // Channel 0
            CLBAR = 0x50,
            CC = 0x54,
            CTR2 = 0x58,
            CBR1 = 0x5C,
            CSAR = 0x60,
            CDAR = 0x64,
            CTR3 = 0x68,
            CBR2 = 0x6C,
            CLLR = 0x70,
        }
    }
}
