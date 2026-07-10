//
// Copyright (c) 2026 CtrlFuture
//
// SPDX-License-Identifier: Apache-2.0
//
// STM32N6 IWDG (Independent Watchdog) model for Renode.
//
// Registers:
//   KR  @ 0x00: Key register (write-only)
//   PR  @ 0x04: Prescaler (RW, requires unlock)
//   RLR @ 0x08: Reload value (RW, requires unlock)
//   SR  @ 0x0C: Status flags (read-only)
//

using Antmicro.Renode.Core;
using Antmicro.Renode.Core.Structure.Registers;
using Antmicro.Renode.Logging;
using Antmicro.Renode.Peripherals.Bus;

namespace Antmicro.Renode.Peripherals.Miscellaneous
{
    public class STM32N6_IWDG : BasicDoubleWordPeripheral, IKnownSize
    {
        public STM32N6_IWDG(IMachine machine) : base(machine)
        {
            DefineRegisters();
        }

        public long Size => 0x100;

        private void DefineRegisters()
        {
            // KR @ 0x00: Key register (write-only)
            // 0xCCCC = start watchdog
            // 0x5555 = unlock PR/RLR
            // 0xAAAA = reload
            Registers.KR.Define(this)
                .WithValueField(0, 16, FieldMode.Write, name: "KR",
                    writeCallback: (_, val) =>
                    {
                        if (val == 0x5555)
                        {
                            unlocked = true;
                        }
                        else if (val == 0xCCCC)
                        {
                            enabled = true;
                        }
                        else if (val == 0xAAAA)
                        {
                            // Reload - no-op in simulation
                        }
                    });

            // PR @ 0x04: Prescaler
            Registers.PR.Define(this)
                .WithValueField(0, 3, name: "PR",
                    valueProviderCallback: _ => prescaler,
                    writeCallback: (_, val) =>
                    {
                        if (unlocked) prescaler = (uint)val;
                    });

            // RLR @ 0x08: Reload value
            Registers.RLR.Define(this)
                .WithValueField(0, 12, name: "RLR",
                    valueProviderCallback: _ => reload,
                    writeCallback: (_, val) =>
                    {
                        if (unlocked) reload = (uint)val;
                    });

            // SR @ 0x0C: Status register (read-only)
            // Bit 0: PVU (prescaler value update)
            // Bit 1: RVU (reload value update)
            // Bit 2: WVU (window value update)
            Registers.SR.Define(this)
                .WithFlag(0, FieldMode.Read,
                    valueProviderCallback: _ => false, name: "PVU")
                .WithFlag(1, FieldMode.Read,
                    valueProviderCallback: _ => false, name: "RVU")
                .WithFlag(2, FieldMode.Read,
                    valueProviderCallback: _ => false, name: "WVU")
                .WithReservedBits(3, 29);
        }

        private bool unlocked;
        private bool enabled;
        private uint prescaler;
        private uint reload;

        private enum Registers : long
        {
            KR = 0x00,
            PR = 0x04,
            RLR = 0x08,
            SR = 0x0C,
        }
    }
}
