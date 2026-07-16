//
// Copyright (c) 2026 CtrlFuture
//
// SPDX-License-Identifier: Apache-2.0
//
// STM32N6 FDCAN model for Renode.
// L2 model: CCCR.INIT sticks on write so driver wait-for-INIT completes;
// CCE only accepted while INIT=1; leave INIT clears CCE. No CAN bus.
//
// CMSIS FDCAN_GlobalTypeDef (subset used by L2):
//   CREL  @ 0x000  ENDN @ 0x004
//   DBTP  @ 0x00C  TEST @ 0x010  RWD @ 0x014
//   CCCR  @ 0x018  NBTP @ 0x01C  TSCC @ 0x020
//   IR    @ 0x050  IE   @ 0x054  ILS  @ 0x058  ILE @ 0x05C
//   RXF0S @ 0x0A4
//   TXBAR @ 0x0D0  TXBCR @ 0x0D4  TXBTO @ 0x0D8
//
// Note: NuttX stm32n6_fdcan.c previously had a wrong CCCR@0x000
// mapping; that was fixed to match the CMSIS offsets below (see
// commit "arch: fix FDCAN register offsets"). This model has
// always followed CMSIS.
//

using Antmicro.Renode.Core;
using Antmicro.Renode.Core.Structure.Registers;
using Antmicro.Renode.Logging;
using Antmicro.Renode.Peripherals.Bus;

namespace Antmicro.Renode.Peripherals.Miscellaneous
{
    public class STM32N6_FDCAN : BasicDoubleWordPeripheral, IKnownSize
    {
        public STM32N6_FDCAN(IMachine machine) : base(machine)
        {
            DefineRegisters();
        }

        public long Size => 0x400;

        public override void Reset()
        {
            base.Reset();
            // Hardware powers up in INIT mode
            cccr = 0x1;
            txbto = 0;
        }

        private void DefineRegisters()
        {
            Registers.CREL.Define(this)
                .WithValueField(0, 32, FieldMode.Read, name: "CREL");

            Registers.ENDN.Define(this)
                .WithValueField(0, 32, FieldMode.Read,
                    valueProviderCallback: _ => 0x87654321, name: "ENDN");

            Registers.DBTP.Define(this)
                .WithValueField(0, 32, name: "DBTP");

            Registers.TEST.Define(this)
                .WithValueField(0, 32, name: "TEST");

            Registers.RWD.Define(this)
                .WithValueField(0, 32, name: "RWD");

            // CCCR @ 0x18: INIT (bit0), CCE (bit1)
            Registers.CCCR.Define(this)
                .WithValueField(0, 32,
                    valueProviderCallback: _ => cccr,
                    writeCallback: (_, val) =>
                    {
                        var v = (uint)val;
                        var init = (v & 0x1) != 0;
                        if(init)
                        {
                            // Enter INIT; accept CCE
                            cccr = v & 0xFFFFu;
                        }
                        else
                        {
                            // Leave INIT: CCE forced clear
                            cccr = v & ~0x3u;
                        }
                    },
                    name: "CCCR");

            Registers.NBTP.Define(this)
                .WithValueField(0, 32, name: "NBTP");

            Registers.TSCC.Define(this)
                .WithValueField(0, 32, name: "TSCC");

            Registers.IR.Define(this)
                .WithValueField(0, 32, name: "IR");

            Registers.IE.Define(this)
                .WithValueField(0, 32, name: "IE");

            Registers.ILS.Define(this)
                .WithValueField(0, 32, name: "ILS");

            Registers.ILE.Define(this)
                .WithValueField(0, 32, name: "ILE");

            Registers.RXF0S.Define(this)
                .WithValueField(0, 32, name: "RXF0S");

            // TXBAR: request bits; mark TXBTO complete immediately (no bus)
            Registers.TXBAR.Define(this)
                .WithValueField(0, 32,
                    writeCallback: (_, val) =>
                    {
                        txbto |= (uint)val;
                    },
                    name: "TXBAR");

            Registers.TXBCR.Define(this)
                .WithValueField(0, 32, name: "TXBCR");

            Registers.TXBTO.Define(this)
                .WithValueField(0, 32, FieldMode.Read,
                    valueProviderCallback: _ => txbto, name: "TXBTO");
        }

        private uint cccr = 0x1;
        private uint txbto;

        private enum Registers : long
        {
            CREL = 0x00,
            ENDN = 0x04,
            DBTP = 0x0C,
            TEST = 0x10,
            RWD = 0x14,
            CCCR = 0x18,
            NBTP = 0x1C,
            TSCC = 0x20,
            IR = 0x50,
            IE = 0x54,
            ILS = 0x58,
            ILE = 0x5C,
            RXF0S = 0xA4,
            TXBAR = 0xD0,
            TXBCR = 0xD4,
            TXBTO = 0xD8,
        }
    }
}
