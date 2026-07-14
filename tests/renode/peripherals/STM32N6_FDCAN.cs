//
// Copyright (c) 2026 CtrlFuture
//
// SPDX-License-Identifier: Apache-2.0
//
// STM32N6 FDCAN model for Renode.
// L2 model: CCCR.INIT sticks on write so driver wait-for-INIT completes;
// CCE only accepted while INIT=1; leave INIT clears CCE. No CAN bus.
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
                        if (init)
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

            Registers.BTP.Define(this)
                .WithValueField(0, 32, name: "BTP");

            Registers.TSCC.Define(this)
                .WithValueField(0, 32, name: "TSCC");

            Registers.IR.Define(this)
                .WithValueField(0, 32, name: "IR");

            Registers.IE.Define(this)
                .WithValueField(0, 32, name: "IE");

            Registers.ILS.Define(this)
                .WithValueField(0, 32, name: "ILS");

            Registers.RXF0S.Define(this)
                .WithValueField(0, 32, name: "RXF0S");

            Registers.TXBAR.Define(this)
                .WithValueField(0, 32, name: "TXBAR");

            Registers.TXBCR.Define(this)
                .WithValueField(0, 32, name: "TXBCR");

            // TXBTO: buffers complete immediately when requested via TXBAR
            Registers.TXBTO.Define(this)
                .WithValueField(0, 32, FieldMode.Read,
                    valueProviderCallback: _ => 0, name: "TXBTO");
        }

        private uint cccr = 0x1;

        private enum Registers : long
        {
            CREL = 0x00,
            ENDN = 0x04,
            DBTP = 0x0C,
            TEST = 0x10,
            RWD = 0x14,
            CCCR = 0x18,
            BTP = 0x1C,
            TSCC = 0x20,
            IR = 0x24,
            IE = 0x28,
            ILS = 0x34,
            RXF0S = 0x44,
            TXBAR = 0xC8,
            TXBCR = 0xCC,
            TXBTO = 0xD0,
        }
    }
}
