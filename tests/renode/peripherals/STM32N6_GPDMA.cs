//
// Copyright (c) 2026 CtrlFuture
//
// SPDX-License-Identifier: Apache-2.0
//
// STM32N6 GPDMA (General Purpose DMA) model for Renode.
// Register layout matches CMSIS DMA_TypeDef / DMA_Channel_TypeDef
// (same channel map as STM32WBA GPDMA).
//
// Global:
//   SECCFGR  @ 0x00
//   PRIVCFGR @ 0x04
//   RCFGLOCKR@ 0x08
//   MISR     @ 0x0C
//   SMISR    @ 0x10
//
// Channel N base = 0x50 + N*0x80:
//   CLBAR    @ +0x00
//   CCIDCFGR @ +0x04
//   CFCR     @ +0x0C  W1C flags (TCFC bit8)
//   CSR      @ +0x10  status RO (TCF bit8)
//   CCR      @ +0x14  control (EN bit0, TCIE bit8; EN auto-clears)
//   CTR1     @ +0x40  SDW/SINC/DDW/DINC
//   CTR2     @ +0x44  SWREQ bit9
//   CBR1     @ +0x48  BNDT[15:0] byte count
//   CSAR     @ +0x4C
//   CDAR     @ +0x50
//   CTR3     @ +0x54  stub
//   CBR2     @ +0x58  stub
//   CLLR     @ +0x7C  stub (no LLI engine)
//
// L3: memory-to-memory transfer on CCR.EN write.
//

using System;

using Antmicro.Renode.Core;
using Antmicro.Renode.Core.Structure.Registers;
using Antmicro.Renode.Logging;
using Antmicro.Renode.Peripherals.Bus;

namespace Antmicro.Renode.Peripherals.Miscellaneous
{
    public class STM32N6_GPDMA : BasicDoubleWordPeripheral, IKnownSize
    {
        public STM32N6_GPDMA(IMachine machine) : base(machine)
        {
            channels = new ChannelState[ChannelCount];
            for(var i = 0; i < ChannelCount; i++)
            {
                channels[i] = new ChannelState();
            }

            DefineRegisters();
        }

        public override void Reset()
        {
            base.Reset();
            for(var i = 0; i < ChannelCount; i++)
            {
                channels[i].Reset();
            }
        }

        public long Size => 0x1000;

        private void DefineRegisters()
        {
            Registers.SECCFGR.Define(this)
                .WithValueField(0, 32, name: "SECCFGR");

            Registers.PRIVCFGR.Define(this)
                .WithValueField(0, 32, name: "PRIVCFGR");

            Registers.RCFGLOCKR.Define(this)
                .WithValueField(0, 32, name: "RCFGLOCKR");

            Registers.MISR.Define(this)
                .WithValueField(0, 32, FieldMode.Read,
                    valueProviderCallback: _ => BuildMaskedInterruptStatus(),
                    name: "MISR");

            Registers.SMISR.Define(this)
                .WithValueField(0, 32, FieldMode.Read, name: "SMISR");

            for(var ch = 0; ch < ChannelCount; ch++)
            {
                var channel = ch;
                var chBase = ChannelBaseOffset + ChannelStride * channel;

                // CLBAR @ +0x00
                ((Registers)(chBase + 0x00)).Define(this)
                    .WithValueField(0, 32,
                        valueProviderCallback: _ => channels[channel].Clbar,
                        writeCallback: (_, val) => channels[channel].Clbar = (uint)val,
                        name: $"CH{channel}_CLBAR");

                // CCIDCFGR @ +0x04
                ((Registers)(chBase + 0x04)).Define(this)
                    .WithValueField(0, 32,
                        valueProviderCallback: _ => channels[channel].Ccidcfgr,
                        writeCallback: (_, val) => channels[channel].Ccidcfgr = (uint)val,
                        name: $"CH{channel}_CCIDCFGR");

                // CFCR @ +0x0C: write-1-to-clear flags
                ((Registers)(chBase + 0x0C)).Define(this)
                    .WithReservedBits(0, 8)
                    .WithFlag(8, FieldMode.Write,
                        writeCallback: (_, val) =>
                        {
                            if(val)
                            {
                                channels[channel].TransferComplete = false;
                            }
                        },
                        name: $"CH{channel}_TCFC")
                    .WithFlag(9, FieldMode.Write,
                        writeCallback: (_, val) =>
                        {
                            if(val)
                            {
                                channels[channel].HalfTransfer = false;
                            }
                        },
                        name: $"CH{channel}_HTFC")
                    .WithReservedBits(10, 22);

                // CSR @ +0x10: channel status (RO)
                ((Registers)(chBase + 0x10)).Define(this)
                    .WithFlag(0, FieldMode.Read,
                        valueProviderCallback: _ => !channels[channel].Enabled,
                        name: $"CH{channel}_IDLEF")
                    .WithReservedBits(1, 7)
                    .WithFlag(8, FieldMode.Read,
                        valueProviderCallback: _ => channels[channel].TransferComplete,
                        name: $"CH{channel}_TCF")
                    .WithFlag(9, FieldMode.Read,
                        valueProviderCallback: _ => channels[channel].HalfTransfer,
                        name: $"CH{channel}_HTF")
                    .WithReservedBits(10, 22);

                // CCR @ +0x14: channel control
                ((Registers)(chBase + 0x14)).Define(this)
                    .WithFlag(0,
                        writeCallback: (_, val) =>
                        {
                            if(val)
                            {
                                DoTransfer(channel);
                            }
                            else
                            {
                                channels[channel].Enabled = false;
                            }
                        },
                        // Hardware auto-clears EN after block complete.
                        valueProviderCallback: _ => channels[channel].Enabled,
                        name: $"CH{channel}_EN")
                    .WithFlag(1, FieldMode.Write,
                        writeCallback: (_, val) =>
                        {
                            if(val)
                            {
                                channels[channel].Enabled = false;
                            }
                        },
                        name: $"CH{channel}_RESET")
                    .WithFlag(2,
                        valueProviderCallback: _ => channels[channel].Suspend,
                        writeCallback: (_, val) => channels[channel].Suspend = val,
                        name: $"CH{channel}_SUSP")
                    .WithReservedBits(3, 5)
                    .WithFlag(8,
                        valueProviderCallback: _ => channels[channel].Tcie,
                        writeCallback: (_, val) => channels[channel].Tcie = val,
                        name: $"CH{channel}_TCIE")
                    .WithReservedBits(9, 23);

                // CTR1 @ +0x40
                ((Registers)(chBase + 0x40)).Define(this)
                    .WithValueField(0, 2,
                        valueProviderCallback: _ => channels[channel].SdwLog2,
                        writeCallback: (_, val) => channels[channel].SdwLog2 = (uint)val,
                        name: $"CH{channel}_SDW_LOG2")
                    .WithReservedBits(2, 1)
                    .WithFlag(3,
                        valueProviderCallback: _ => channels[channel].Sinc,
                        writeCallback: (_, val) => channels[channel].Sinc = val,
                        name: $"CH{channel}_SINC")
                    .WithReservedBits(4, 12)
                    .WithValueField(16, 2,
                        valueProviderCallback: _ => channels[channel].DdwLog2,
                        writeCallback: (_, val) => channels[channel].DdwLog2 = (uint)val,
                        name: $"CH{channel}_DDW_LOG2")
                    .WithReservedBits(18, 1)
                    .WithFlag(19,
                        valueProviderCallback: _ => channels[channel].Dinc,
                        writeCallback: (_, val) => channels[channel].Dinc = val,
                        name: $"CH{channel}_DINC")
                    .WithReservedBits(20, 12);

                // CTR2 @ +0x44
                ((Registers)(chBase + 0x44)).Define(this)
                    .WithValueField(0, 9,
                        valueProviderCallback: _ => channels[channel].Ctr2Low,
                        writeCallback: (_, val) => channels[channel].Ctr2Low = (uint)val,
                        name: $"CH{channel}_CTR2_LOW")
                    .WithFlag(9,
                        valueProviderCallback: _ => channels[channel].Swreq,
                        writeCallback: (_, val) => channels[channel].Swreq = val,
                        name: $"CH{channel}_SWREQ")
                    .WithValueField(10, 22,
                        valueProviderCallback: _ => channels[channel].Ctr2High,
                        writeCallback: (_, val) => channels[channel].Ctr2High = (uint)val,
                        name: $"CH{channel}_CTR2_HIGH");

                // CBR1 @ +0x48: BNDT[15:0]
                ((Registers)(chBase + 0x48)).Define(this)
                    .WithValueField(0, 16,
                        valueProviderCallback: _ => channels[channel].Bndt,
                        writeCallback: (_, val) => channels[channel].Bndt = (uint)val,
                        name: $"CH{channel}_BNDT")
                    .WithReservedBits(16, 16);

                // CSAR @ +0x4C
                ((Registers)(chBase + 0x4C)).Define(this)
                    .WithValueField(0, 32,
                        valueProviderCallback: _ => channels[channel].Csar,
                        writeCallback: (_, val) => channels[channel].Csar = (uint)val,
                        name: $"CH{channel}_CSAR");

                // CDAR @ +0x50
                ((Registers)(chBase + 0x50)).Define(this)
                    .WithValueField(0, 32,
                        valueProviderCallback: _ => channels[channel].Cdar,
                        writeCallback: (_, val) => channels[channel].Cdar = (uint)val,
                        name: $"CH{channel}_CDAR");

                // CTR3 @ +0x54 stub
                ((Registers)(chBase + 0x54)).Define(this)
                    .WithValueField(0, 32,
                        valueProviderCallback: _ => channels[channel].Ctr3,
                        writeCallback: (_, val) => channels[channel].Ctr3 = (uint)val,
                        name: $"CH{channel}_CTR3");

                // CBR2 @ +0x58 stub
                ((Registers)(chBase + 0x58)).Define(this)
                    .WithValueField(0, 32,
                        valueProviderCallback: _ => channels[channel].Cbr2,
                        writeCallback: (_, val) => channels[channel].Cbr2 = (uint)val,
                        name: $"CH{channel}_CBR2");

                // CLLR @ +0x7C stub (no linked-list engine)
                ((Registers)(chBase + 0x7C)).Define(this)
                    .WithValueField(0, 32,
                        valueProviderCallback: _ => channels[channel].Cllr,
                        writeCallback: (_, val) => channels[channel].Cllr = (uint)val,
                        name: $"CH{channel}_CLLR");
            }
        }

        private uint BuildMaskedInterruptStatus()
        {
            uint status = 0;
            for(var i = 0; i < ChannelCount; i++)
            {
                if(channels[i].TransferComplete && channels[i].Tcie)
                {
                    status |= 1u << i;
                }
            }

            return status;
        }

        private void DoTransfer(int channel)
        {
            var ch = channels[channel];
            var bndt = ch.Bndt & 0xFFFF;
            if(bndt == 0)
            {
                this.Log(LogLevel.Warning,
                    "GPDMA ch{0}: EN with BNDT=0, no transfer", channel);
                ch.Enabled = false;
                ch.TransferComplete = true;
                return;
            }

            // Cap widths to byte/halfword/word (LOG2 0..2).
            var sdw = Math.Min(ch.SdwLog2, 2u);
            var ddw = Math.Min(ch.DdwLog2, 2u);
            var srcWidth = 1u << (int)sdw;
            var dstWidth = 1u << (int)ddw;
            var unit = Math.Max(srcWidth, dstWidth);

            if((bndt % unit) != 0)
            {
                this.Log(LogLevel.Warning,
                    "GPDMA ch{0}: BNDT={1} not aligned to unit={2}",
                    channel, bndt, unit);
            }

            ch.Enabled = true;
            this.Log(LogLevel.Debug,
                "GPDMA ch{0}: mem2mem {1} bytes CSAR=0x{2:X8} CDAR=0x{3:X8} SINC={4} DINC={5} unit={6}",
                channel, bndt, ch.Csar, ch.Cdar, ch.Sinc, ch.Dinc, unit);

            var src = ch.Csar;
            var dst = ch.Cdar;
            var remaining = bndt;

            // Byte-accurate copy; advance SA/DA by programmed width when SINC/DINC.
            while(remaining > 0)
            {
                var chunk = Math.Min(unit, remaining);
                for(uint i = 0; i < chunk; i++)
                {
                    var b = sysbus.ReadByte(src + (ch.Sinc ? i : 0));
                    sysbus.WriteByte(dst + (ch.Dinc ? i : 0), b);
                }

                if(ch.Sinc)
                {
                    src += srcWidth;
                }

                if(ch.Dinc)
                {
                    dst += dstWidth;
                }

                if(remaining >= unit)
                {
                    remaining -= unit;
                }
                else
                {
                    remaining = 0;
                }

                if(remaining <= (bndt / 2))
                {
                    ch.HalfTransfer = true;
                }
            }

            ch.Csar = src;
            ch.Cdar = dst;
            ch.Bndt = 0;
            ch.Enabled = false;
            ch.TransferComplete = true;

            this.Log(LogLevel.Debug,
                "GPDMA ch{0}: transfer complete TCF=1 EN=0", channel);
        }

        private readonly ChannelState[] channels;

        private const int ChannelCount = 16;
        private const int ChannelBaseOffset = 0x50;
        private const int ChannelStride = 0x80;

        private sealed class ChannelState
        {
            public void Reset()
            {
                Clbar = 0;
                Ccidcfgr = 0;
                Enabled = false;
                Suspend = false;
                Tcie = false;
                TransferComplete = false;
                HalfTransfer = false;
                SdwLog2 = 0;
                DdwLog2 = 0;
                Sinc = false;
                Dinc = false;
                Swreq = false;
                Ctr2Low = 0;
                Ctr2High = 0;
                Bndt = 0;
                Csar = 0;
                Cdar = 0;
                Ctr3 = 0;
                Cbr2 = 0;
                Cllr = 0;
            }

            public uint Clbar;
            public uint Ccidcfgr;
            public bool Enabled;
            public bool Suspend;
            public bool Tcie;
            public bool TransferComplete;
            public bool HalfTransfer;
            public uint SdwLog2;
            public uint DdwLog2;
            public bool Sinc;
            public bool Dinc;
            public bool Swreq;
            public uint Ctr2Low;
            public uint Ctr2High;
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
            RCFGLOCKR = 0x08,
            MISR = 0x0C,
            SMISR = 0x10,
        }
    }
}
