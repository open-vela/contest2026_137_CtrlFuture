//
// Copyright (c) 2026 CtrlFuture
//
// SPDX-License-Identifier: Apache-2.0
//
// STM32N6 Random Number Generator (RNG) model for Renode.
// L2 model: RNGEN sets DRDY; DR read returns random data and
// re-asserts DRDY while enabled; CONDRST soft-reset consistent.
// IRQ asserts when RNGEN && IE && DRDY (wired to NVIC RNG_IRQn=40).
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
            IRQ = new GPIO();
            random = new Random();
            DefineRegisters();
        }

        public long Size => 0x100;

        public GPIO IRQ { get; }

        public override void Reset()
        {
            base.Reset();
            IRQ.Unset();
        }

        private void UpdateInterrupt()
        {
            // IRQ when enabled, IE set, and data ready (DRDY-gated L2)
            IRQ.Set(rngEnabled.Value && interruptEnable.Value &&
                    dataReady.Value);
        }

        private void DefineRegisters()
        {
            // CR @ 0x00: Control Register
            Registers.CR.Define(this)
                .WithFlag(2, out rngEnabled, name: "RNGEN",
                    writeCallback: (_, val) =>
                    {
                        // RNGEN=1 makes data available immediately in sim
                        dataReady.Value = val;
                        UpdateInterrupt();
                    })
                .WithFlag(3, out interruptEnable, name: "IE",
                    changeCallback: (_, __) => UpdateInterrupt())
                .WithFlag(5, name: "CED")
                .WithFlag(30, out condrst, name: "CONDRST",
                    writeCallback: (_, val) =>
                    {
                        // Hardware: CONDRST must be written 1 then 0
                        if (!val)
                        {
                            dataReady.Value = rngEnabled.Value;
                            UpdateInterrupt();
                        }
                    })
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
                        var value = (uint)random.Next();
                        // Clear DRDY after read; re-assert if still enabled
                        dataReady.Value = false;
                        if (rngEnabled.Value)
                        {
                            dataReady.Value = true;
                        }
                        UpdateInterrupt();
                        return value;
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
