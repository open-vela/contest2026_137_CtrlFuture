//
// Copyright (c) 2026 CtrlFuture
//
// SPDX-License-Identifier: Apache-2.0
//
// STM32N6 DCMIPP model for Renode.
// L2 model: CMSIS-aligned key offsets — IPGR1@0x00, PRCR@0x104
// (ENABLE bit14 sticks), P0FCTCR@0x500 (CPTREQ sticks), P0SR@0x5F8.
// Residual: full pipe map / CSI / DMA capture path not modeled.
// Size expanded so P0 registers are reachable.
//

using Antmicro.Renode.Core;
using Antmicro.Renode.Core.Structure.Registers;
using Antmicro.Renode.Logging;
using Antmicro.Renode.Peripherals.Bus;

namespace Antmicro.Renode.Peripherals.Miscellaneous
{
    public class STM32N6_DCMIPP : BasicDoubleWordPeripheral, IKnownSize
    {
        public STM32N6_DCMIPP(IMachine machine) : base(machine)
        {
            DefineRegisters();
        }

        // Cover Pipe0 status @ 0x5F8
        public long Size => 0x800;

        public override void Reset()
        {
            base.Reset();
            ipgr1 = 0;
            prcr = 0;
            p0fctcr = 0;
            p0sr = 0;
        }

        private void DefineRegisters()
        {
            // IPGR1 @ 0x00 (was misnamed CR in L1 shell)
            Registers.IPGR1.Define(this)
                .WithValueField(0, 32,
                    valueProviderCallback: _ => ipgr1,
                    writeCallback: (_, val) => ipgr1 = (uint)val,
                    name: "IPGR1");

            // Keep SR shell at 0x04 for legacy L1 accessibility tests
            Registers.SR_LEGACY.Define(this)
                .WithValueField(0, 32, name: "SR_LEGACY");

            Registers.IER_LEGACY.Define(this)
                .WithValueField(0, 32, name: "IER_LEGACY");

            Registers.SRCR_LEGACY.Define(this)
                .WithValueField(0, 32, name: "SRCR_LEGACY");

            // PRCR @ 0x104: parallel interface control; ENABLE bit14
            Registers.PRCR.Define(this)
                .WithValueField(0, 32,
                    valueProviderCallback: _ => prcr,
                    writeCallback: (_, val) => prcr = (uint)val,
                    name: "PRCR");

            // CMCR @ 0x204 (common config shell)
            Registers.CMCR.Define(this)
                .WithValueField(0, 32, name: "CMCR");

            // P0FSCR @ 0x404
            Registers.P0FSCR.Define(this)
                .WithValueField(0, 32, name: "P0FSCR");

            // P0FCTCR @ 0x500: CPTREQ bit3 sticks
            Registers.P0FCTCR.Define(this)
                .WithValueField(0, 32,
                    valueProviderCallback: _ => p0fctcr,
                    writeCallback: (_, val) => p0fctcr = (uint)val,
                    name: "P0FCTCR");

            // P0SR @ 0x5F8: status (read-only zeros in L2)
            Registers.P0SR.Define(this)
                .WithValueField(0, 32, FieldMode.Read,
                    valueProviderCallback: _ => p0sr,
                    name: "P0SR");
        }

        private uint ipgr1;
        private uint prcr;
        private uint p0fctcr;
        private uint p0sr;

        private enum Registers : long
        {
            IPGR1 = 0x00,
            SR_LEGACY = 0x04,
            IER_LEGACY = 0x10,
            SRCR_LEGACY = 0x14,
            PRCR = 0x104,
            CMCR = 0x204,
            P0FSCR = 0x404,
            P0FCTCR = 0x500,
            P0SR = 0x5F8,
        }
    }
}
