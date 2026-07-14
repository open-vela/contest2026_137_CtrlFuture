//
// Copyright (c) 2026 CtrlFuture
//
// SPDX-License-Identifier: Apache-2.0
//
// STM32N6 I2C model for Renode.
// L2/L3 master path matching NuttX stm32n6_i2c transfer sequence:
//   PE enable, CR2 START+NBYTES+AUTOEND, TXIS/TXDR or RXNE/RXDR, STOPF.
// Register-level model: no real I2C bus slaves; RX uses loopback/canned data.
// IRQ asserts on TXIE/RXIE/STOPIE/TCIE when matching status flags set.
//
// Registers (CMSIS I2C_TypeDef):
//   CR1      @ 0x00
//   CR2      @ 0x04
//   OAR1     @ 0x08
//   OAR2     @ 0x0C
//   TIMINGR  @ 0x10
//   TIMEOUTR @ 0x14
//   ISR      @ 0x18
//   ICR      @ 0x1C
//   PECR     @ 0x20
//   RXDR     @ 0x24
//   TXDR     @ 0x28
//

using System.Collections.Generic;
using Antmicro.Renode.Core;
using Antmicro.Renode.Core.Structure.Registers;
using Antmicro.Renode.Logging;
using Antmicro.Renode.Peripherals.Bus;

namespace Antmicro.Renode.Peripherals.Miscellaneous
{
    public class STM32N6_I2C : BasicDoubleWordPeripheral, IKnownSize
    {
        public STM32N6_I2C(IMachine machine) : base(machine)
        {
            IRQ = new GPIO();
            receiveFifo = new Queue<byte>();
            loopbackBuffer = new Queue<byte>();
            DefineRegisters();
        }

        public long Size => 0x100;

        public GPIO IRQ { get; }

        public override void Reset()
        {
            base.Reset();
            receiveFifo.Clear();
            loopbackBuffer.Clear();
            remainingBytes = 0;
            masterBusy = false;
            transmitInterruptStatus = false;
            stopDetection = false;
            transferComplete = false;
            transferCompleteReload = false;
            isReadTransfer = false;
            IRQ.Unset();
        }

        private void UpdateInterrupt()
        {
            var evt =
                (txie.Value && transmitInterruptStatus) ||
                (rxie.Value && receiveFifo.Count > 0) ||
                (stopie.Value && stopDetection) ||
                (tcie.Value && (transferComplete || transferCompleteReload));
            IRQ.Set(evt);
        }

        private void PeripheralDisabled()
        {
            receiveFifo.Clear();
            loopbackBuffer.Clear();
            remainingBytes = 0;
            masterBusy = false;
            transmitInterruptStatus = false;
            stopDetection = false;
            transferComplete = false;
            transferCompleteReload = false;
            isReadTransfer = false;
            UpdateInterrupt();
        }

        private void StartTransfer()
        {
            if(!pe.Value)
            {
                this.Log(LogLevel.Warning, "START while PE=0 ignored");
                return;
            }

            transferComplete = false;
            transferCompleteReload = false;
            stopDetection = false;
            remainingBytes = (int)nbytes.Value;
            isReadTransfer = readWrite.Value;
            masterBusy = true;

            if(isReadTransfer)
            {
                // Fill RX FIFO with loopback data, else canned pattern
                receiveFifo.Clear();
                for(var i = 0; i < remainingBytes; i++)
                {
                    byte b;
                    if(loopbackBuffer.Count > 0)
                    {
                        b = loopbackBuffer.Dequeue();
                    }
                    else
                    {
                        b = (byte)(0xA5 + i);
                    }

                    receiveFifo.Enqueue(b);
                }

                transmitInterruptStatus = false;
                // NBYTES=0 read: complete immediately (same as write path)
                if(remainingBytes == 0)
                {
                    SetTransferCompleteFlags();
                }
            }
            else
            {
                // Master write: ready for first TXDR write if NBYTES > 0
                transmitInterruptStatus = remainingBytes > 0;
                if(remainingBytes == 0)
                {
                    SetTransferCompleteFlags();
                }
            }

            UpdateInterrupt();
        }

        private void StopTransfer()
        {
            masterBusy = false;
            transmitInterruptStatus = false;
            stopDetection = true;
            UpdateInterrupt();
        }

        private void SetTransferCompleteFlags()
        {
            if(autoEnd.Value)
            {
                masterBusy = false;
                stopDetection = true;
                transmitInterruptStatus = false;
            }
            else if(reload.Value)
            {
                transferCompleteReload = true;
                transmitInterruptStatus = false;
            }
            else
            {
                transferComplete = true;
                transmitInterruptStatus = false;
            }

            UpdateInterrupt();
        }

        private void HandleTransmitWrite(uint value)
        {
            if(!pe.Value || !masterBusy || isReadTransfer)
            {
                this.Log(LogLevel.Warning,
                    "TXDR write ignored (PE={0} busy={1} read={2})",
                    pe.Value, masterBusy, isReadTransfer);
                return;
            }

            if(remainingBytes <= 0)
            {
                this.Log(LogLevel.Warning, "TXDR write with no remaining bytes");
                return;
            }

            // Writing TXDR clears TXIS; reassert if more bytes remain
            transmitInterruptStatus = false;
            loopbackBuffer.Enqueue((byte)value);
            remainingBytes--;

            if(remainingBytes == 0)
            {
                SetTransferCompleteFlags();
            }
            else
            {
                // Still need more TX bytes (level-triggered TXIS)
                transmitInterruptStatus = true;
                UpdateInterrupt();
            }
        }

        private uint HandleReceiveRead()
        {
            if(receiveFifo.Count == 0)
            {
                this.Log(LogLevel.Warning, "RXDR read while FIFO empty");
                return 0;
            }

            var value = receiveFifo.Dequeue();
            if(remainingBytes > 0)
            {
                remainingBytes--;
            }

            if(remainingBytes == 0 && receiveFifo.Count == 0)
            {
                SetTransferCompleteFlags();
            }
            else
            {
                UpdateInterrupt();
            }

            return value;
        }

        private void DefineRegisters()
        {
            // CR1 @ 0x00
            Registers.CR1.Define(this)
                .WithFlag(0, out pe, name: "PE",
                    changeCallback: (_, val) =>
                    {
                        if(!val)
                        {
                            PeripheralDisabled();
                        }
                    })
                .WithFlag(1, out txie, name: "TXIE",
                    changeCallback: (_, __) => UpdateInterrupt())
                .WithFlag(2, out rxie, name: "RXIE",
                    changeCallback: (_, __) => UpdateInterrupt())
                .WithFlag(3, name: "ADDRIE")
                .WithFlag(4, name: "NACKIE")
                .WithFlag(5, out stopie, name: "STOPIE",
                    changeCallback: (_, __) => UpdateInterrupt())
                .WithFlag(6, out tcie, name: "TCIE",
                    changeCallback: (_, __) => UpdateInterrupt())
                .WithFlag(7, name: "ERRIE")
                .WithValueField(8, 4, name: "DNF")
                .WithFlag(12, name: "ANFOFF")
                .WithFlag(13, name: "SWRST")
                .WithFlag(14, name: "TXDMAEN")
                .WithFlag(15, name: "RXDMAEN")
                .WithFlag(16, name: "SBC")
                .WithFlag(17, name: "NOSTRETCH")
                .WithFlag(18, name: "WUPEN")
                .WithFlag(19, name: "GCEN")
                .WithFlag(20, name: "SMBHEN")
                .WithFlag(21, name: "SMBDEN")
                .WithFlag(22, name: "ALERTEN")
                .WithFlag(23, name: "PECEN")
                .WithReservedBits(24, 8);

            // CR2 @ 0x04
            Registers.CR2.Define(this)
                .WithValueField(0, 10, name: "SADD")
                .WithFlag(10, out readWrite, name: "RD_WRN")
                .WithFlag(11, name: "ADD10")
                .WithFlag(12, name: "HEAD10R")
                .WithFlag(13, out start, name: "START")
                .WithFlag(14, out stop, name: "STOP")
                .WithFlag(15, name: "NACK")
                .WithValueField(16, 8, out nbytes, name: "NBYTES")
                .WithFlag(24, out reload, name: "RELOAD")
                .WithFlag(25, out autoEnd, name: "AUTOEND")
                .WithFlag(26, name: "PECBYTE")
                .WithReservedBits(27, 5)
                .WithWriteCallback((_, __) =>
                {
                    if(start.Value && stop.Value)
                    {
                        this.Log(LogLevel.Warning,
                            "START and STOP set together, ignoring");
                    }
                    else if(start.Value)
                    {
                        StartTransfer();
                    }
                    else if(stop.Value)
                    {
                        StopTransfer();
                    }

                    // Hardware auto-clears START/STOP after handling
                    start.Value = false;
                    stop.Value = false;
                });

            // OAR1 / OAR2 — stubs for config writes
            Registers.OAR1.Define(this)
                .WithValueField(0, 32, name: "OAR1");

            Registers.OAR2.Define(this)
                .WithValueField(0, 32, name: "OAR2");

            // TIMINGR @ 0x10
            Registers.TIMINGR.Define(this)
                .WithValueField(0, 32, name: "TIMINGR");

            // TIMEOUTR @ 0x14
            Registers.TIMEOUTR.Define(this)
                .WithValueField(0, 32, name: "TIMEOUTR");

            // ISR @ 0x18 — dynamic status (CMSIS: BUSY=15, DIR=16)
            // Reset: TXE=1
            Registers.ISR.Define(this, 0x01)
                .WithFlag(0, FieldMode.Read, name: "TXE",
                    valueProviderCallback: _ => true)
                .WithFlag(1, FieldMode.Read, name: "TXIS",
                    valueProviderCallback: _ => transmitInterruptStatus)
                .WithFlag(2, FieldMode.Read, name: "RXNE",
                    valueProviderCallback: _ => receiveFifo.Count > 0)
                .WithFlag(3, FieldMode.Read, name: "ADDR")
                .WithFlag(4, FieldMode.Read, name: "NACKF")
                .WithFlag(5, FieldMode.Read, name: "STOPF",
                    valueProviderCallback: _ => stopDetection)
                .WithFlag(6, FieldMode.Read, name: "TC",
                    valueProviderCallback: _ => transferComplete)
                .WithFlag(7, FieldMode.Read, name: "TCR",
                    valueProviderCallback: _ => transferCompleteReload)
                .WithFlag(8, FieldMode.Read, name: "BERR")
                .WithFlag(9, FieldMode.Read, name: "ARLO")
                .WithFlag(10, FieldMode.Read, name: "OVR")
                .WithFlag(11, FieldMode.Read, name: "PECERR")
                .WithFlag(12, FieldMode.Read, name: "TIMEOUT")
                .WithFlag(13, FieldMode.Read, name: "ALERT")
                .WithReservedBits(14, 1)
                .WithFlag(15, FieldMode.Read, name: "BUSY",
                    valueProviderCallback: _ => masterBusy)
                .WithFlag(16, FieldMode.Read, name: "DIR",
                    valueProviderCallback: _ => isReadTransfer)
                .WithValueField(17, 7, FieldMode.Read, name: "ADDCODE")
                .WithReservedBits(24, 8);

            // ICR @ 0x1C — write-1-to-clear
            Registers.ICR.Define(this)
                .WithReservedBits(0, 3)
                .WithFlag(3, FieldMode.Write, name: "ADDRCF")
                .WithFlag(4, FieldMode.Write, name: "NACKCF")
                .WithFlag(5, FieldMode.Write, name: "STOPCF",
                    writeCallback: (_, val) =>
                    {
                        if(val)
                        {
                            stopDetection = false;
                            UpdateInterrupt();
                        }
                    })
                .WithReservedBits(6, 2)
                .WithFlag(8, FieldMode.Write, name: "BERRCF")
                .WithFlag(9, FieldMode.Write, name: "ARLOCF")
                .WithFlag(10, FieldMode.Write, name: "OVRCF")
                .WithFlag(11, FieldMode.Write, name: "PECCF")
                .WithFlag(12, FieldMode.Write, name: "TIMOUTCF")
                .WithFlag(13, FieldMode.Write, name: "ALERTCF")
                .WithReservedBits(14, 18);

            // PECR @ 0x20
            Registers.PECR.Define(this)
                .WithValueField(0, 32, FieldMode.Read, name: "PECR");

            // RXDR @ 0x24
            Registers.RXDR.Define(this)
                .WithValueField(0, 8, FieldMode.Read, name: "RXDATA",
                    valueProviderCallback: _ => HandleReceiveRead())
                .WithReservedBits(8, 24);

            // TXDR @ 0x28
            Registers.TXDR.Define(this)
                .WithValueField(0, 8, FieldMode.Write, name: "TXDATA",
                    writeCallback: (_, val) => HandleTransmitWrite((uint)val))
                .WithReservedBits(8, 24);
        }

        private readonly Queue<byte> receiveFifo;
        private readonly Queue<byte> loopbackBuffer;

        private IFlagRegisterField pe;
        private IFlagRegisterField txie;
        private IFlagRegisterField rxie;
        private IFlagRegisterField stopie;
        private IFlagRegisterField tcie;
        private IFlagRegisterField readWrite;
        private IFlagRegisterField start;
        private IFlagRegisterField stop;
        private IValueRegisterField nbytes;
        private IFlagRegisterField reload;
        private IFlagRegisterField autoEnd;

        private int remainingBytes;
        private bool masterBusy;
        private bool transmitInterruptStatus;
        private bool stopDetection;
        private bool transferComplete;
        private bool transferCompleteReload;
        private bool isReadTransfer;

        private enum Registers : long
        {
            CR1      = 0x00,
            CR2      = 0x04,
            OAR1     = 0x08,
            OAR2     = 0x0C,
            TIMINGR  = 0x10,
            TIMEOUTR = 0x14,
            ISR      = 0x18,
            ICR      = 0x1C,
            PECR     = 0x20,
            RXDR     = 0x24,
            TXDR     = 0x28,
        }
    }
}
