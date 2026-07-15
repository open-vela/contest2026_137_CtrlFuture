//
// Copyright (c) 2026 CtrlFuture
//
// SPDX-License-Identifier: Apache-2.0
//
// STM32N6 SDMMC model for Renode.
// L2/L3 command/response path for NuttX stm32n6_sdmmc probe sequence:
//   POWER ON → CLKCR → CMD0 → CMD8 → (CMD55+ACMD41)* → CMD2 → CMD3 →
//   CMD7 → CMD16. Canned card responses; no SDCard image / FIFO / IDMA.
//
// CMSIS SDMMC_TypeDef layout (H7-class, not F4):
//   POWER   @ 0x00  PWRCTRL[1:0]
//   CLKCR   @ 0x04  CLKDIV / WIDBUS (no CLKEN on N6; retained as RW)
//   ARG     @ 0x08
//   CMD     @ 0x0C  CMDINDEX[5:0], WAITRESP[9:8], CPSMEN[12]
//   RESPCMD @ 0x10
//   RESP1-4 @ 0x14..0x20
//   DTIMER  @ 0x24
//   DLEN    @ 0x28
//   DCTRL   @ 0x2C
//   STA     @ 0x34  CMDREND[6], CMDSENT[7], ...
//   ICR     @ 0x38  W1C of STA sticky flags
//   MASK    @ 0x3C
//
// Driver pre-wait compatibility (stm32n6_sdmmc.c sdmmc_send_cmd):
//   The driver polls STA for CMDSENT|CMDREND *before* issuing a command
//   (path-ready wait). Real HW does not require this. To let the probe
//   path run without hanging:
//     1. Reset / POWER-ON asserts CMDSENT (idle/ready indication).
//     2. CPSMEN fires: clear prior completion flags, process command,
//        assert CMDSENT (no-resp) or CMDREND (short/long) + fill RESP.
//     3. After ICR clears those bits, re-assert CMDSENT so the next
//        pre-wait succeeds. Documented intentional sim-for-driver quirk.
//

using System;
using System.Collections.Generic;

using Antmicro.Renode.Core;
using Antmicro.Renode.Core.Structure.Registers;
using Antmicro.Renode.Logging;
using Antmicro.Renode.Peripherals.Bus;

namespace Antmicro.Renode.Peripherals.Miscellaneous
{
    public class STM32N6_SDMMC : BasicDoubleWordPeripheral, IKnownSize
    {
        public STM32N6_SDMMC(IMachine machine) : base(machine)
        {
            DefineRegisters();
            // Path-ready for first sdmmc_send_cmd pre-wait.
            cmdSent = true;
            IRQ = new GPIO();
        }

        public long Size => 0x1000;

        public GPIO IRQ { get; }

        public override void Reset()
        {
            base.Reset();
            cardState = CardState.Idle;
            appCmdPending = false;
            rca = DefaultRca;
            respCmd = 0;
            resp1 = 0;
            resp2 = 0;
            resp3 = 0;
            resp4 = 0;
            cmdSent = true;   // pre-wait ready
            cmdREnd = false;
            cCrcFail = false;
            cTimeout = false;
            dataEnd = false;
            IRQ.Unset();
        }

        private void DefineRegisters()
        {
            // POWER @ 0x00
            Registers.POWER.Define(this)
                .WithValueField(0, 2, out pwrCtrl, name: "PWRCTRL",
                    writeCallback: (_, val) =>
                    {
                        if(val == PwrCtrlOn)
                        {
                            // Power-on: mark command path ready.
                            cmdSent = true;
                            cardState = CardState.Idle;
                            appCmdPending = false;
                        }
                    })
                .WithFlag(2, name: "VSWITCH")
                .WithFlag(3, name: "VSWITCHEN")
                .WithFlag(4, name: "DIRPOL")
                .WithReservedBits(5, 27);

            // CLKCR @ 0x04 — flat RW so driver CLKEN bit16 is retained
            Registers.CLKCR.Define(this)
                .WithValueField(0, 32, name: "CLKCR");

            // ARG @ 0x08
            Registers.ARG.Define(this)
                .WithValueField(0, 32, out commandArgument, name: "CMDARG");

            // CMD @ 0x0C — N6/H7 bit layout (CMSIS)
            Registers.CMD.Define(this)
                .WithValueField(0, 6, out commandIndex, name: "CMDINDEX")
                .WithFlag(6, name: "CMDTRANS")
                .WithFlag(7, name: "CMDSTOP")
                .WithValueField(8, 2, out waitResp, name: "WAITRESP")
                .WithFlag(10, name: "WAITINT")
                .WithFlag(11, name: "WAITPEND")
                .WithFlag(12, name: "CPSMEN",
                    writeCallback: (_, val) =>
                    {
                        if(val)
                        {
                            ProcessCommand();
                        }
                    })
                .WithFlag(13, name: "DTHOLD")
                .WithFlag(14, name: "BOOTMODE")
                .WithFlag(15, name: "BOOTEN")
                .WithFlag(16, name: "CMDSUSPEND")
                .WithReservedBits(17, 15);

            // RESPCMD @ 0x10
            Registers.RESPCMD.Define(this)
                .WithValueField(0, 6, FieldMode.Read, name: "RESPCMD",
                    valueProviderCallback: _ => respCmd)
                .WithReservedBits(6, 26);

            // RESP1-4 @ 0x14..0x20
            Registers.RESP1.Define(this)
                .WithValueField(0, 32, FieldMode.Read, name: "RESP1",
                    valueProviderCallback: _ => resp1);

            Registers.RESP2.Define(this)
                .WithValueField(0, 32, FieldMode.Read, name: "RESP2",
                    valueProviderCallback: _ => resp2);

            Registers.RESP3.Define(this)
                .WithValueField(0, 32, FieldMode.Read, name: "RESP3",
                    valueProviderCallback: _ => resp3);

            Registers.RESP4.Define(this)
                .WithValueField(0, 32, FieldMode.Read, name: "RESP4",
                    valueProviderCallback: _ => resp4);

            // DTIMER / DLEN / DCTRL / DCOUNT stubs (probe path unused)
            Registers.DTIMER.Define(this)
                .WithValueField(0, 32, name: "DTIMER");

            Registers.DLEN.Define(this)
                .WithValueField(0, 32,
                    valueProviderCallback: _ => dlen,
                    writeCallback: (_, val) => dlen = (uint)val,
                    name: "DLEN");

            Registers.DCTRL.Define(this)
                .WithValueField(0, 32,
                    valueProviderCallback: _ => dctrl,
                    writeCallback: (_, val) =>
                    {
                        var newVal = (uint)val;
                        // DTEN transition 0→1 triggers data path
                        if (((newVal & 0x1) == 1) && ((dctrl & 0x1) == 0))
                        {
                            StartDataPath();
                        }

                        dctrl = newVal;
                    },
                    name: "DCTRL");

            Registers.DCOUNT.Define(this)
                .WithValueField(0, 32, FieldMode.Read, name: "DCOUNT",
                    valueProviderCallback: _ => (ulong)dcountRemaining);

            // FIFO @ 0x80: data read/write
            Registers.FIFO.Define(this)
                .WithValueField(0, 32,
                    valueProviderCallback: _ =>
                    {
                        if (dataDir == DataDir.Read && dcountRemaining > 0)
                        {
                            var b = (uint)(cannedPattern & 0xFF);
                            cannedPattern++;
                            dcountRemaining -= 4;
                            if (dcountRemaining <= 0)
                            {
                                dcountRemaining = 0;
                                dataEnd = true;
                                dataDir = DataDir.Idle;
                                AssertIrq();
                            }

                            return b;
                        }

                        return 0;
                    },
                    writeCallback: (_, val) =>
                    {
                        if (dataDir == DataDir.Write && dcountRemaining > 0)
                        {
                            cannedPattern = (uint)val;
                            dcountRemaining -= 4;
                            if (dcountRemaining <= 0)
                            {
                                dcountRemaining = 0;
                                dataEnd = true;
                                dataDir = DataDir.Idle;
                                AssertIrq();
                            }
                        }
                    },
                    name: "FIFO");

            // STA @ 0x34 — sticky completion flags
            Registers.STA.Define(this)
                .WithFlag(0, FieldMode.Read, name: "CCRCFAIL",
                    valueProviderCallback: _ => cCrcFail)
                .WithFlag(1, FieldMode.Read, name: "DCRCFAIL")
                .WithFlag(2, FieldMode.Read, name: "CTIMEOUT",
                    valueProviderCallback: _ => cTimeout)
                .WithFlag(3, FieldMode.Read, name: "DTIMEOUT")
                .WithFlag(4, FieldMode.Read, name: "TXUNDERR")
                .WithFlag(5, FieldMode.Read, name: "RXOVERR")
                .WithFlag(6, FieldMode.Read, name: "CMDREND",
                    valueProviderCallback: _ => cmdREnd)
                .WithFlag(7, FieldMode.Read, name: "CMDSENT",
                    valueProviderCallback: _ => cmdSent)
                .WithFlag(8, FieldMode.Read, name: "DATAEND",
                    valueProviderCallback: _ => dataEnd)
                .WithFlag(9, FieldMode.Read, name: "DHOLD")
                .WithFlag(10, FieldMode.Read, name: "DBCKEND")
                .WithFlag(11, FieldMode.Read, name: "CMDACT",
                    valueProviderCallback: _ => false)
                .WithFlag(12, FieldMode.Read, name: "TXACT")
                .WithFlag(13, FieldMode.Read, name: "RXACT")
                .WithFlag(14, FieldMode.Read, name: "TXFIFOHE")
                .WithFlag(15, FieldMode.Read, name: "RXFIFOHF")
                .WithFlag(16, FieldMode.Read, name: "TXFIFOF")
                .WithFlag(17, FieldMode.Read, name: "RXFIFOF")
                .WithFlag(18, FieldMode.Read, name: "TXFIFOE")
                .WithFlag(19, FieldMode.Read, name: "RXFIFOE")
                .WithFlag(20, FieldMode.Read, name: "BUSYD0")
                .WithFlag(21, FieldMode.Read, name: "BUSYD0END")
                .WithReservedBits(22, 10);

            // ICR @ 0x38 — write-1-to-clear sticky STA bits
            Registers.ICR.Define(this)
                .WithFlag(0, FieldMode.Write, name: "CCRCFAILC",
                    writeCallback: (_, v) => { if(v) cCrcFail = false; })
                .WithFlag(1, FieldMode.Write, name: "DCRCFAILC")
                .WithFlag(2, FieldMode.Write, name: "CTIMEOUTC",
                    writeCallback: (_, v) => { if(v) cTimeout = false; })
                .WithFlag(3, FieldMode.Write, name: "DTIMEOUTC")
                .WithFlag(4, FieldMode.Write, name: "TXUNDERRC")
                .WithFlag(5, FieldMode.Write, name: "RXOVERRC")
                .WithFlag(6, FieldMode.Write, name: "CMDRENDC",
                    writeCallback: (_, v) =>
                    {
                        if(v)
                        {
                            cmdREnd = false;
                            // Driver pre-wait quirk: re-assert path ready.
                            cmdSent = true;
                        }
                    })
                .WithFlag(7, FieldMode.Write, name: "CMDSENTC",
                    writeCallback: (_, v) =>
                    {
                        if(v)
                        {
                            cmdSent = false;
                            // Re-assert so next pre-wait (CMDSENT|CMDREND)
                            // does not hang after ICR of a no-resp command.
                            cmdSent = true;
                        }
                    })
                .WithFlag(8, FieldMode.Write, name: "DATAENDC",
                    writeCallback: (_, v) => { if(v) dataEnd = false; })
                .WithFlag(9, FieldMode.Write, name: "DHOLDC")
                .WithFlag(10, FieldMode.Write, name: "DBCKENDC")
                .WithFlag(11, FieldMode.Write, name: "DABORTC")
                .WithReservedBits(12, 9)
                .WithFlag(21, FieldMode.Write, name: "BUSYD0ENDC")
                .WithReservedBits(22, 10);

            // MASK @ 0x3C
            Registers.MASK.Define(this)
                .WithValueField(0, 32, name: "MASK");
        }

        private void ProcessCommand()
        {
            // Clear prior completion before new command finishes.
            cmdSent = false;
            cmdREnd = false;
            cCrcFail = false;
            cTimeout = false;

            var index = (uint)commandIndex.Value;
            var arg = (uint)commandArgument.Value;
            var responseType = (uint)waitResp.Value;
            var isApp = appCmdPending;
            appCmdPending = false;

            this.Log(LogLevel.Noisy,
                "SDMMC CMD{0} arg=0x{1:X8} WAITRESP={2} app={3} state={4}",
                index, arg, responseType, isApp, cardState);

            uint response = 0;
            bool haveResponse = responseType != WaitRespNone;

            if(isApp)
            {
                response = HandleAppCommand(index, arg);
            }
            else
            {
                response = HandleCommand(index, arg, ref haveResponse);
            }

            respCmd = index & 0x3F;
            if(haveResponse && responseType != WaitRespNone)
            {
                resp1 = response;
                // Long response placeholders for CID/CSD (driver often
                // ignores them when resp==NULL, but fill for completeness).
                if(responseType == WaitRespLong)
                {
                    // RESP1 already set; leave RESP2-4 canned zeros/CID.
                }

                cmdREnd = true;
            }
            else
            {
                cmdSent = true;
            }

            AssertIrq();
        }

        private uint HandleCommand(uint index, uint arg, ref bool haveResponse)
        {
            switch(index)
            {
            case 0: // GO_IDLE_STATE
                cardState = CardState.Idle;
                rca = DefaultRca;
                haveResponse = false;
                return 0;

            case 2: // ALL_SEND_CID
                // Canned CID across RESP1-4; short path uses RESP1 only.
                resp1 = 0x03534453; // "SDS" mid-CID style
                resp2 = 0x44333200;
                resp3 = 0x00000001;
                resp4 = 0x00000000;
                if(cardState == CardState.Ready)
                {
                    cardState = CardState.Ident;
                }
                return resp1;

            case 3: // SEND_RELATIVE_ADDR
                // R6: RCA in [31:16]
                if(cardState == CardState.Ident ||
                   cardState == CardState.Ready ||
                   cardState == CardState.Idle)
                {
                    cardState = CardState.Standby;
                }
                return (rca << 16);

            case 7: // SELECT/DESELECT_CARD
                {
                    var selRca = (arg >> 16) & 0xFFFF;
                    if(selRca == rca)
                    {
                        cardState = CardState.Transfer;
                    }
                    else if(selRca == 0)
                    {
                        cardState = CardState.Standby;
                    }
                    // R1 card status
                    return R1ReadyForData;
                }

            case 8: // SEND_IF_COND — R7 echoes check pattern
                return arg & 0xFFF;

            case 9: // SEND_CSD
                resp1 = 0x400E0032;
                resp2 = 0x5B590000;
                resp3 = 0x00007F80;
                resp4 = 0x0A400000;
                return resp1;

            case 16: // SET_BLOCKLEN
                return R1ReadyForData;

            case 55: // APP_CMD
                appCmdPending = true;
                // R1 with APP_CMD bit (bit5) set when short resp requested
                return R1ReadyForData | R1AppCmd;

            default:
                this.Log(LogLevel.Warning,
                    "Unhandled SDMMC CMD{0}, returning R1 ready", index);
                return R1ReadyForData;
            }
        }

        private uint HandleAppCommand(uint index, uint arg)
        {
            switch(index)
            {
            case 41: // SD_SEND_OP_COND (ACMD41) — R3 OCR
                // Instant ready: bit31 busy=0 means ready in OCR? SD spec:
                // OCR bit31 = busy (0=busy, 1=ready). Driver checks bit31 set.
                // Driver: if (resp & 0x80000000) ready. Provide CCS + ready.
                if(cardState == CardState.Idle)
                {
                    cardState = CardState.Ready;
                }
                return OcrReadyCcs | (arg & 0x00FFFFFF);

            case 6: // SET_BUS_WIDTH
                return R1ReadyForData;

            case 51: // SEND_SCR
                return R1ReadyForData;

            default:
                this.Log(LogLevel.Warning,
                    "Unhandled SDMMC ACMD{0}, returning R1 ready", index);
                return R1ReadyForData;
            }
        }

        private void AssertIrq()
        {
            bool pending = cmdSent || cmdREnd || dataEnd;
            IRQ.Set(pending);
        }

        private enum CardState
        {
            Idle,
            Ready,
            Ident,
            Standby,
            Transfer,
        }

        private enum Registers : long
        {
            POWER = 0x00,
            CLKCR = 0x04,
            ARG = 0x08,
            CMD = 0x0C,
            RESPCMD = 0x10,
            RESP1 = 0x14,
            RESP2 = 0x18,
            RESP3 = 0x1C,
            RESP4 = 0x20,
            DTIMER = 0x24,
            DLEN = 0x28,
            DCTRL = 0x2C,
            DCOUNT = 0x30,
            STA = 0x34,
            ICR = 0x38,
            MASK = 0x3C,
            FIFO = 0x80,
        }

        // WAITRESP encoding (CMSIS bits [9:8])
        private const uint WaitRespNone = 0;
        private const uint WaitRespShort = 1;
        private const uint WaitRespShortNoCrc = 2;
        private const uint WaitRespLong = 3;

        private const ulong PwrCtrlOn = 0x3;
        private const uint DefaultRca = 0x0001;
        private const uint R1ReadyForData = 0x00000100;
        private const uint R1AppCmd = 0x00000020;
        // OCR: bit31 ready, bit30 CCS (SDHC/SDXC)
        private const uint OcrReadyCcs = 0xC0FF8000;

        private IValueRegisterField pwrCtrl;
        private IValueRegisterField commandArgument;
        private IValueRegisterField commandIndex;
        private IValueRegisterField waitResp;

        private CardState cardState;
        private bool appCmdPending;
        private uint rca = DefaultRca;
        private uint respCmd;
        private uint resp1;
        private uint resp2;
        private uint resp3;
        private uint resp4;

        private bool cmdSent;
        private bool cmdREnd;
        private bool cCrcFail;
        private bool cTimeout;
        private bool dataEnd;

        // Data path state
        private uint dlen;
        private uint dctrl;
        private int dcountRemaining;
        private uint cannedPattern = 0xA5;
        private DataDir dataDir = DataDir.Idle;

        private enum DataDir
        {
            Idle,
            Read,
            Write,
        }

        private void StartDataPath()
        {
            if (dlen == 0)
            {
                return;
            }

            // DCTRL bit1 = data direction (0=read, 1=write)
            var isWrite = (dctrl & 0x2) != 0;
            dataDir = isWrite ? DataDir.Write : DataDir.Read;
            dcountRemaining = (int)dlen;
            cannedPattern = 0xA5;
        }
    }
}
