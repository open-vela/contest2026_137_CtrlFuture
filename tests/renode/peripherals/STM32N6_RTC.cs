//
// Copyright (c) 2026 CtrlFuture
//
// SPDX-License-Identifier: Apache-2.0
//
// STM32N6 RTC model for Renode.
// L2 model: WPR unlock (0xCA then 0x53), INIT->INITF, calendar
// init sets INITS+RSF, protected CR/PRER/TR/DR writes when locked.
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

        public override void Reset()
        {
            base.Reset();
            initMode = false;
            calendarInit = false;
            registersSynced = false;
            writeProtected = true;
            unlockStep = 0;
            timeReg = 0;
            dateReg = 0;
            prerReg = 0x007F00FF; // PREDIV_A=127, PREDIV_S=255 typical
            crReg = 0;
            wutrReg = 0xFFFF;
        }

        private void DefineRegisters()
        {
            // TR @ 0x00: Time register (BCD)
            Registers.TR.Define(this)
                .WithValueField(0, 32,
                    valueProviderCallback: _ => timeReg,
                    writeCallback: (_, val) =>
                    {
                        if (!writeProtected && initMode)
                        {
                            timeReg = (uint)val;
                            calendarInit = true;
                        }
                    },
                    name: "TR");

            // DR @ 0x04: Date register (BCD)
            Registers.DR.Define(this)
                .WithValueField(0, 32,
                    valueProviderCallback: _ => dateReg,
                    writeCallback: (_, val) =>
                    {
                        if (!writeProtected && initMode)
                        {
                            dateReg = (uint)val;
                            calendarInit = true;
                        }
                    },
                    name: "DR");

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
                    valueProviderCallback: _ => calendarInit, name: "INITS")
                .WithFlag(5,
                    valueProviderCallback: _ => registersSynced,
                    writeCallback: (_, val) =>
                    {
                        // Hardware: software clears RSF by writing 0
                        if (!val)
                        {
                            registersSynced = false;
                        }
                    },
                    name: "RSF")
                .WithFlag(6, FieldMode.Read,
                    valueProviderCallback: _ => initMode, name: "INITF")
                .WithFlag(7, name: "INIT",
                    writeCallback: (_, val) =>
                    {
                        if (writeProtected)
                        {
                            return;
                        }
                        if (val)
                        {
                            initMode = true;
                            registersSynced = false;
                        }
                        else
                        {
                            // Exit init: INITF clears, RSF sets when calendar ready
                            initMode = false;
                            registersSynced = true;
                        }
                    },
                    valueProviderCallback: _ => initMode)
                .WithReservedBits(8, 24);

            // PRER @ 0x10: Prescaler register
            Registers.PRER.Define(this)
                .WithValueField(0, 32,
                    valueProviderCallback: _ => prerReg,
                    writeCallback: (_, val) =>
                    {
                        if (!writeProtected)
                        {
                            prerReg = (uint)val;
                        }
                    },
                    name: "PRER");

            // WUTR @ 0x14: Wakeup timer register
            Registers.WUTR.Define(this)
                .WithValueField(0, 32,
                    valueProviderCallback: _ => wutrReg,
                    writeCallback: (_, val) =>
                    {
                        if (!writeProtected)
                        {
                            wutrReg = (uint)val;
                        }
                    },
                    name: "WUTR");

            // CR @ 0x18: Control register
            Registers.CR.Define(this)
                .WithValueField(0, 32,
                    valueProviderCallback: _ => crReg,
                    writeCallback: (_, val) =>
                    {
                        if (!writeProtected)
                        {
                            crReg = (uint)val;
                        }
                    },
                    name: "CR");

            // PRIVCFGR @ 0x1C: Privilege configuration
            Registers.PRIVCFGR.Define(this)
                .WithValueField(0, 32, name: "PRIVCFGR");

            // SECCFGR @ 0x20: Security configuration
            Registers.SECCFGR.Define(this)
                .WithValueField(0, 32, name: "SECCFGR");

            // WPR @ 0x24: Write protection (write-only)
            // Unlock sequence: write 0xCA then 0x53. Any other value re-locks.
            Registers.WPR.Define(this)
                .WithValueField(0, 8, FieldMode.Write, name: "WPR",
                    writeCallback: (_, val) =>
                    {
                        if (val == 0xCA && unlockStep == 0)
                        {
                            unlockStep = 1;
                        }
                        else if (val == 0x53 && unlockStep == 1)
                        {
                            writeProtected = false;
                            unlockStep = 0;
                        }
                        else
                        {
                            writeProtected = true;
                            unlockStep = 0;
                        }
                    });

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
        private bool calendarInit;
        private bool registersSynced;
        private bool writeProtected = true;
        private int unlockStep;
        private uint timeReg;
        private uint dateReg;
        private uint prerReg = 0x007F00FF;
        private uint crReg;
        private uint wutrReg = 0xFFFF;

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
