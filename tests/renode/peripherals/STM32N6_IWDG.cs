//
// Copyright (c) 2026 CtrlFuture
//
// SPDX-License-Identifier: Apache-2.0
//
// STM32N6 IWDG (Independent Watchdog) model for Renode.
// Register layout matches CMSIS IWDG_TypeDef (stm32n647xx.h).
//
// Registers:
//   KR   @ 0x00: Key register (write-only)
//   PR   @ 0x04: Prescaler (RW, requires unlock)
//   RLR  @ 0x08: Reload value (RW, requires unlock)
//   SR   @ 0x0C: Status flags (read-only)
//   WINR @ 0x10: Window register (RW, requires unlock)
//   EWCR @ 0x14: Early wakeup control
//   ICR  @ 0x18: Interrupt clear (write-only)
//
// SR bits: PVU(0), RVU(1), WVU(2), EWU(3), ONF(8), EWIF(15)
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
            // 0x5555 = unlock PR/RLR/WINR
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

            // PR @ 0x04: Prescaler [2:0]
            Registers.PR.Define(this)
                .WithValueField(0, 3, name: "PR",
                    valueProviderCallback: _ => prescaler,
                    writeCallback: (_, val) =>
                    {
                        if (unlocked) prescaler = (uint)val;
                    });

            // RLR @ 0x08: Reload value [11:0]
            Registers.RLR.Define(this)
                .WithValueField(0, 12, name: "RLR",
                    valueProviderCallback: _ => reload,
                    writeCallback: (_, val) =>
                    {
                        if (unlocked) reload = (uint)val;
                    });

            // SR @ 0x0C: Status register (read-only)
            // PVU (bit 0): prescaler value update
            // RVU (bit 1): reload value update
            // WVU (bit 2): window value update
            // EWU (bit 3): early wakeup register update
            // ONF (bit 8): watchdog enabled flag
            // EWIF (bit 15): early wakeup interrupt flag
            Registers.SR.Define(this)
                .WithFlag(0, FieldMode.Read,
                    valueProviderCallback: _ => false, name: "PVU")
                .WithFlag(1, FieldMode.Read,
                    valueProviderCallback: _ => false, name: "RVU")
                .WithFlag(2, FieldMode.Read,
                    valueProviderCallback: _ => false, name: "WVU")
                .WithFlag(3, FieldMode.Read,
                    valueProviderCallback: _ => false, name: "EWU")
                .WithReservedBits(4, 4)
                .WithFlag(8, FieldMode.Read,
                    valueProviderCallback: _ => enabled, name: "ONF")
                .WithReservedBits(9, 6)
                .WithFlag(15, FieldMode.Read,
                    valueProviderCallback: _ => false, name: "EWIF")
                .WithReservedBits(16, 16);

            // WINR @ 0x10: Window register [11:0]
            Registers.WINR.Define(this)
                .WithValueField(0, 12, name: "WINR",
                    valueProviderCallback: _ => window,
                    writeCallback: (_, val) =>
                    {
                        if (unlocked) window = (uint)val;
                    });

            // EWCR @ 0x14: Early wakeup control
            // EWIT [11:0]: early wakeup comparison value
            // EWIE (bit 15): early wakeup interrupt enable
            Registers.EWCR.Define(this)
                .WithValueField(0, 12, name: "EWIT",
                    valueProviderCallback: _ => ewit,
                    writeCallback: (_, val) => ewit = (uint)val)
                .WithReservedBits(12, 3)
                .WithFlag(15, name: "EWIE",
                    valueProviderCallback: _ => ewie,
                    writeCallback: (_, val) => ewie = val);

            // ICR @ 0x18: Interrupt clear (write-only)
            // EWIC (bit 15): early wakeup interrupt clear
            Registers.ICR.Define(this)
                .WithValueField(0, 32, FieldMode.Write, name: "ICR",
                    writeCallback: (_, val) =>
                    {
                        if ((val & (1 << 15)) != 0)
                        {
                            // Clear EWIF - no-op since we never set it
                        }
                    });
        }

        private bool unlocked;
        private bool enabled;
        private uint prescaler;
        private uint reload;
        private uint window;
        private uint ewit;
        private bool ewie;

        private enum Registers : long
        {
            KR = 0x00,
            PR = 0x04,
            RLR = 0x08,
            SR = 0x0C,
            WINR = 0x10,
            EWCR = 0x14,
            ICR = 0x18,
        }
    }
}
