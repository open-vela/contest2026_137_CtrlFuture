//
// Copyright (c) 2026 CtrlFuture
//
// SPDX-License-Identifier: Apache-2.0
//
// STM32N6 Random Number Generator (RNG) model for Renode.
// Minimal model: register read/write with simulated random data.
//
// Registers:
//   CR    @ 0x00: Control Register
//   SR    @ 0x04: Status Register
//   DR    @ 0x08: Data Register (read-only, returns random values)
//   HTCR  @ 0x10: Health Test Configuration Register
//

using System;
using Antmicro.Renode.Core;
using Antmicro.Renode.Core.Structure.Registers;
using Antmicro.Renode.Logging;
using Antmicro.Renode.Peripherals.Bus;

namespace Antmicro.Renode.Peripherals.Miscellaneous
{
    public class STM32N6_RNG : BasicDoubleWordPeripheral, IKnownSize
    {
        public STM32N6_RNG(IMachine machine) : base(machine)
        {
            random = new Random();
            DefineRegisters();
        }

        public long Size => 0x100;

        private void DefineRegisters()
        {
            // CR @ 0x00: Control Register
            Registers.CR.Define(this)
                .WithFlag(2, out rngEnabled, name: "RNGEN")
                .WithFlag(3, out interruptEnable, name: "IE")
                .WithFlag(5, name: "CED")
                .WithFlag(30, out condrst, name: "CONDRST")
                .WithFlag(31, FieldMode.Read, name: "CONFIGLOCK");

            // SR @ 0x04: Status Register
            Registers.SR.Define(this, 0x00000000)
                .WithFlag(0, out dataReady, FieldMode.Read,
                    valueProviderCallback: _ => dataReady.Value,
                    name: "DRDY")
                .WithFlag(1, FieldMode.Read, name: "CECS")
                .WithFlag(2, FieldMode.Read, name: "SECS")
                .WithFlag(5, FieldMode.Read | FieldMode.WriteOneToClear,
                    name: "CEIS")
                .WithFlag(6, FieldMode.Read | FieldMode.WriteOneToClear,
                    name: "SEIS");

            // DR @ 0x08: Data Register (read-only)
            Registers.DR.Define(this)
                .WithValueField(0, 32, FieldMode.Read,
                    valueProviderCallback: _ =>
                    {
                        // Clear DRDY after read
                        dataReady.Value = false;
                        return (uint)random.Next();
                    },
                    name: "DR");

            // HTCR @ 0x10: Health Test Configuration Register
            Registers.HTCR.Define(this)
                .WithValueField(0, 32, name: "HTCFG");
        }

        private readonly Random random;
        private IFlagRegisterField rngEnabled;
        private IFlagRegisterField interruptEnable;
        private IFlagRegisterField condrst;
        private IFlagRegisterField dataReady;

        private enum Registers : long
        {
            CR   = 0x00,
            SR   = 0x04,
            DR   = 0x08,
            HTCR = 0x10,
        }
    }
}
