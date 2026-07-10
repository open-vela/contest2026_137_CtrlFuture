//
// Copyright (c) 2026 CtrlFuture
//
// SPDX-License-Identifier: Apache-2.0
//
// STM32N6 RTC model for Renode.
// Register layout matches CMSIS RTC_TypeDef (stm32n647xx.h).
//
// Registers:
//   TR       @ 0x00: Time register (BCD, RW in init mode)
//   DR       @ 0x04: Date register (BCD, RW in init mode)
//   SSR      @ 0x08: Sub-second register (read-only)
//   ICSR     @ 0x0C: Init control/status register
//   PRER     @ 0x10: Prescaler register
//   WUTR     @ 0x14: Wakeup timer register
//   CR       @ 0x18: Control register
//   PRIVCFGR @ 0x1C: Privilege configuration
//   SECCFGR  @ 0x20: Security configuration
//   WPR      @ 0x24: Write protection (write-only)
//   SR       @ 0x50: Status register
//

using Antmicro.Renode.Core;
using Antmicro.Renode.Core.Structure.Registers;
using Antmicro.Renode.Logging;
using Antmicro.Renode.Peripherals.Bus;

namespace Antmicro.Renode.Peripherals.Miscellaneous
{
    public class STM32N6_RTC : BasicDoubleWordPeripheral, IKnownSize
    {
        public STM32N6_RTC(IMachine machine) : base(machine)
        {
            DefineRegisters();
        }

        public long Size => 0x400;

        private void DefineRegisters()
        {
            // TR @ 0x00: Time register (BCD)
            Registers.TR.Define(this)
                .WithValueField(0, 32, name: "TR");

            // DR @ 0x04: Date register (BCD)
            Registers.DR.Define(this)
                .WithValueField(0, 32, name: "DR");

            // SSR @ 0x08: Sub-second register (read-only)
            Registers.SSR.Define(this)
                .WithValueField(0, 32, FieldMode.Read, name: "SSR");

            // ICSR @ 0x0C: Init control/status register
            // WUTWF (bit 2), SHPF (bit 3), INITS (bit 4),
            // RSF (bit 5), INITF (bit 6), INIT (bit 7)
            Registers.ICSR.Define(this)
                .WithReservedBits(0, 2)
                .WithFlag(2, FieldMode.Read,
                    valueProviderCallback: _ => true, name: "WUTWF")
                .WithFlag(3, FieldMode.Read,
                    valueProviderCallback: _ => false, name: "SHPF")
                .WithFlag(4, FieldMode.Read,
                    valueProviderCallback: _ => false, name: "INITS")
                .WithFlag(5, name: "RSF")
                .WithFlag(6, FieldMode.Read,
                    valueProviderCallback: _ => initMode, name: "INITF")
                .WithFlag(7, name: "INIT",
                    writeCallback: (_, val) => initMode = val,
                    valueProviderCallback: _ => initMode)
                .WithReservedBits(8, 24);

            // PRER @ 0x10: Prescaler register
            Registers.PRER.Define(this)
                .WithValueField(0, 32, name: "PRER");

            // WUTR @ 0x14: Wakeup timer register
            Registers.WUTR.Define(this)
                .WithValueField(0, 32, name: "WUTR");

            // CR @ 0x18: Control register
            Registers.CR.Define(this)
                .WithValueField(0, 32, name: "CR");

            // PRIVCFGR @ 0x1C: Privilege configuration
            Registers.PRIVCFGR.Define(this)
                .WithValueField(0, 32, name: "PRIVCFGR");

            // SECCFGR @ 0x20: Security configuration
            Registers.SECCFGR.Define(this)
                .WithValueField(0, 32, name: "SECCFGR");

            // WPR @ 0x24: Write protection (write-only)
            Registers.WPR.Define(this)
                .WithValueField(0, 32, FieldMode.Write, name: "WPR");

            // SR @ 0x50: Status register
            // ALRAF (bit 0), ALRBF (bit 1), WUTF (bit 2),
            // TSF (bit 3), TSOVF (bit 4), ITSF (bit 5), SSRUF (bit 6)
            Registers.SR.Define(this)
                .WithFlag(0, name: "ALRAF")
                .WithFlag(1, name: "ALRBF")
                .WithFlag(2, name: "WUTF")
                .WithFlag(3, name: "TSF")
                .WithFlag(4, name: "TSOVF")
                .WithFlag(5, name: "ITSF")
                .WithFlag(6, name: "SSRUF")
                .WithReservedBits(7, 25);
        }

        private bool initMode;

        private enum Registers : long
        {
            TR = 0x00,
            DR = 0x04,
            SSR = 0x08,
            ICSR = 0x0C,
            PRER = 0x10,
            WUTR = 0x14,
            CR = 0x18,
            PRIVCFGR = 0x1C,
            SECCFGR = 0x20,
            WPR = 0x24,
            SR = 0x50,
        }
    }
}
