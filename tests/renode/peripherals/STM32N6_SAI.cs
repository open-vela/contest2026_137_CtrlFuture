//
// Copyright (c) 2026 CtrlFuture
//
// SPDX-License-Identifier: Apache-2.0
//
// STM32N6 SAI (Serial Audio Interface) model for Renode.
// L2 model: CR1.SAIEN sticks; SR.FREQ asserts while enabled so TX path
// does not spin forever. No real audio data path.
//
// Note: N6 SAI block uses A/B sub-blocks; this model maps block A offsets
// (CR1@0x04) used by the NuttX driver register helpers.
//

using Antmicro.Renode.Core;
using Antmicro.Renode.Core.Structure.Registers;
using Antmicro.Renode.Logging;
using Antmicro.Renode.Peripherals.Bus;

namespace Antmicro.Renode.Peripherals.Miscellaneous
{
    public class STM32N6_SAI : BasicDoubleWordPeripheral, IKnownSize
    {
        public STM32N6_SAI(IMachine machine) : base(machine)
        {
            DefineRegisters();
        }

        public long Size => 0x400;

        public override void Reset()
        {
            base.Reset();
            cr1 = 0;
            data = 0;
        }

        private void DefineRegisters()
        {
            // CR1 @ 0x04: SAIEN is bit 16
            Registers.CR1.Define(this)
                .WithValueField(0, 32,
                    valueProviderCallback: _ => cr1,
                    writeCallback: (_, val) => cr1 = (uint)val,
                    name: "CR1");

            Registers.CR2.Define(this)
                .WithValueField(0, 32, name: "CR2");

            Registers.FRCR.Define(this)
                .WithValueField(0, 32, name: "FRCR");

            Registers.SLOTR.Define(this)
                .WithValueField(0, 32, name: "SLOTR");

            // SR @ 0x14: FREQ (bit 3) when SAIEN so FIFO always "ready"
            Registers.SR.Define(this)
                .WithFlag(0, FieldMode.Read, name: "OVRUDR")
                .WithFlag(1, FieldMode.Read, name: "MUTEDET")
                .WithFlag(2, FieldMode.Read, name: "WCKCFG")
                .WithFlag(3, FieldMode.Read,
                    valueProviderCallback: _ => (cr1 & (1u << 16)) != 0,
                    name: "FREQ")
                .WithFlag(4, FieldMode.Read, name: "CNRDY")
                .WithFlag(5, FieldMode.Read, name: "AFSDET")
                .WithFlag(6, FieldMode.Read, name: "LFSDET")
                .WithReservedBits(7, 9)
                .WithValueField(16, 3, FieldMode.Read, name: "FLVL")
                .WithReservedBits(19, 13);

            Registers.CLRFR.Define(this)
                .WithValueField(0, 32, FieldMode.Write, name: "CLRFR");

            Registers.DR.Define(this)
                .WithValueField(0, 32,
                    valueProviderCallback: _ => data,
                    writeCallback: (_, val) => data = (uint)val,
                    name: "DR");
        }

        private uint cr1;
        private uint data;

        private enum Registers : long
        {
            CR1 = 0x04,
            CR2 = 0x08,
            FRCR = 0x0C,
            SLOTR = 0x10,
            SR = 0x14,
            CLRFR = 0x18,
            DR = 0x20,
        }
    }
}
