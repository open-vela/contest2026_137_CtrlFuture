//
// Copyright (c) 2026 CtrlFuture
//
// SPDX-License-Identifier: Apache-2.0
//
// STM32N6 HPDMA (High-Performance DMA) model for Renode.
// L3 memory-to-memory transfer + IRQ GPIO.
// Channel layout: global regs @ 0x00, channel 0 @ 0x50..0x7C.
//

using System;

using Antmicro.Renode.Core;
using Antmicro.Renode.Core.Structure.Registers;
using Antmicro.Renode.Logging;
using Antmicro.Renode.Peripherals.Bus;

namespace Antmicro.Renode.Peripherals.Miscellaneous
{
    public class STM32N6_HPDMA : BasicDoubleWordPeripheral, IKnownSize
    {
        public STM32N6_HPDMA(IMachine machine) : base(machine)
        {
            ch = new ChannelState();
            IRQ = new GPIO();
            DefineRegisters();
        }

        public long Size => 0x400;

        public GPIO IRQ { get; }

        public override void Reset()
        {
            base.Reset();
            ch.Reset();
            IRQ.Unset();
        }

        private void DefineRegisters()
        {
            Registers.SECCFGR.Define(this)
                .WithValueField(0, 32, name: "SECCFGR");

            Registers.PRIVCFGR.Define(this)
                .WithValueField(0, 32, name: "PRIVCFGR");

            Registers.MISR.Define(this)
                .WithValueField(0, 32, FieldMode.Read,
                    valueProviderCallback: _ => BuildMisr(),
                    name: "MISR");

            // Channel 0: CLBAR
            Registers.CLBAR.Define(this)
                .WithValueField(0, 32,
                    valueProviderCallback: _ => ch.Clbar,
                    writeCallback: (_, val) => ch.Clbar = (uint)val,
                    name: "CLBAR");

            // Channel 0: CC (control) — bit 0 = EN, bit 8 = TCIE
            Registers.CC.Define(this)
                .WithFlag(0,
                    writeCallback: (_, val) =>
                    {
                        if (val) DoTransfer();
                    },
                    valueProviderCallback: _ => ch.Enabled,
                    name: "CC_EN")
                .WithFlag(8,
                    valueProviderCallback: _ => ch.Tcie,
                    writeCallback: (_, val) =>
                    {
                        ch.Tcie = val;
                        UpdateInterrupt();
                    },
                    name: "CC_TCIE");

            // CTR2: SDW[1:0]/DDW[17:16] + SINC[3]/DINC[19]
            Registers.CTR2.Define(this)
                .WithValueField(0, 2,
                    valueProviderCallback: _ => ch.SdwLog2,
                    writeCallback: (_, val) => ch.SdwLog2 = (uint)val,
                    name: "CTR2_SDW")
                .WithReservedBits(2, 1)
                .WithFlag(3,
                    valueProviderCallback: _ => ch.Sinc,
                    writeCallback: (_, val) => ch.Sinc = val,
                    name: "CTR2_SINC")
                .WithReservedBits(4, 12)
                .WithValueField(16, 2,
                    valueProviderCallback: _ => ch.DdwLog2,
                    writeCallback: (_, val) => ch.DdwLog2 = (uint)val,
                    name: "CTR2_DDW")
                .WithReservedBits(18, 1)
                .WithFlag(19,
                    valueProviderCallback: _ => ch.Dinc,
                    writeCallback: (_, val) => ch.Dinc = val,
                    name: "CTR2_DINC")
                .WithReservedBits(20, 12);

            // CBR1: BNDT[15:0]
            Registers.CBR1.Define(this)
                .WithValueField(0, 16,
                    valueProviderCallback: _ => ch.Bndt,
                    writeCallback: (_, val) => ch.Bndt = (uint)val,
                    name: "CBR1_BNDT");

            // CSAR
            Registers.CSAR.Define(this)
                .WithValueField(0, 32,
                    valueProviderCallback: _ => ch.Csar,
                    writeCallback: (_, val) => ch.Csar = (uint)val,
                    name: "CSAR");

            // CDAR
            Registers.CDAR.Define(this)
                .WithValueField(0, 32,
                    valueProviderCallback: _ => ch.Cdar,
                    writeCallback: (_, val) => ch.Cdar = (uint)val,
                    name: "CDAR");

            // CTR3 / CBR2 / CLLR stubs
            Registers.CTR3.Define(this)
                .WithValueField(0, 32,
                    valueProviderCallback: _ => ch.Ctr3,
                    writeCallback: (_, val) => ch.Ctr3 = (uint)val,
                    name: "CTR3");

            Registers.CBR2.Define(this)
                .WithValueField(0, 32,
                    valueProviderCallback: _ => ch.Cbr2,
                    writeCallback: (_, val) => ch.Cbr2 = (uint)val,
                    name: "CBR2");

            Registers.CLLR.Define(this)
                .WithValueField(0, 32,
                    valueProviderCallback: _ => ch.Cllr,
                    writeCallback: (_, val) => ch.Cllr = (uint)val,
                    name: "CLLR");
        }

        private uint BuildMisr()
        {
            if (ch.TransferComplete && ch.Tcie)
                return 0x1;
            return 0;
        }

        private void DoTransfer()
        {
            var bndt = ch.Bndt & 0xFFFF;
            if (bndt == 0)
            {
                this.Log(LogLevel.Warning,
                    "HPDMA: EN with BNDT=0, no transfer");
                ch.Enabled = false;
                return;
            }

            var sdw = Math.Min(ch.SdwLog2, 2u);
            var ddw = Math.Min(ch.DdwLog2, 2u);
            var srcWidth = 1u << (int)sdw;
            var dstWidth = 1u << (int)ddw;
            var unit = Math.Max(srcWidth, dstWidth);

            ch.Enabled = true;
            this.Log(LogLevel.Debug,
                "HPDMA: mem2mem {0} bytes CSAR=0x{1:X8} CDAR=0x{2:X8}",
                bndt, ch.Csar, ch.Cdar);

            var src = ch.Csar;
            var dst = ch.Cdar;
            var remaining = bndt;

            while (remaining > 0)
            {
                var chunk = Math.Min(unit, remaining);
                for (uint i = 0; i < chunk; i++)
                {
                    var b = sysbus.ReadByte(src + i);
                    sysbus.WriteByte(dst + i, b);
                }

                if (ch.Sinc) src += srcWidth;
                if (ch.Dinc) dst += dstWidth;

                if (remaining >= unit)
                    remaining -= unit;
                else
                    remaining = 0;
            }

            ch.Csar = src;
            ch.Cdar = dst;
            ch.Bndt = 0;
            ch.Enabled = false;
            ch.TransferComplete = true;

            this.Log(LogLevel.Debug, "HPDMA: transfer complete");
            UpdateInterrupt();
        }

        private void UpdateInterrupt()
        {
            bool pending = ch.TransferComplete && ch.Tcie;
            IRQ.Set(pending);
        }

        private ChannelState ch;

        private sealed class ChannelState
        {
            public void Reset()
            {
                Clbar = 0;
                Enabled = false;
                Tcie = false;
                TransferComplete = false;
                SdwLog2 = 0;
                DdwLog2 = 0;
                Sinc = false;
                Dinc = false;
                Bndt = 0;
                Csar = 0;
                Cdar = 0;
                Ctr3 = 0;
                Cbr2 = 0;
                Cllr = 0;
            }

            public uint Clbar;
            public bool Enabled;
            public bool Tcie;
            public bool TransferComplete;
            public uint SdwLog2;
            public uint DdwLog2;
            public bool Sinc;
            public bool Dinc;
            public uint Bndt;
            public uint Csar;
            public uint Cdar;
            public uint Ctr3;
            public uint Cbr2;
            public uint Cllr;
        }

        private enum Registers : long
        {
            SECCFGR = 0x00,
            PRIVCFGR = 0x04,
            MISR = 0x08,
            CLBAR = 0x50,
            CC = 0x54,
            CTR2 = 0x58,
            CBR1 = 0x5C,
            CSAR = 0x60,
            CDAR = 0x64,
            CTR3 = 0x68,
            CBR2 = 0x6C,
            CLLR = 0x70,
        }
    }
}
