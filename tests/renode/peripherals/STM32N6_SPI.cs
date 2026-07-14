//
// Copyright (c) 2026 CtrlFuture
//
// SPDX-License-Identifier: Apache-2.0
//
// STM32N6 SPI model for Renode.
// L2/L3 model matching NuttX stm32n6_spi transfer path:
//   SPE enables TXP; TXDR write loopbacks to RXDR and asserts RXP/TXC/EOT.
//
// Registers (CMSIS SPI_TypeDef):
//   CR1     @ 0x00
//   CR2     @ 0x04
//   CFG1    @ 0x08
//   CFG2    @ 0x0C
//   IER     @ 0x10
//   SR      @ 0x14
//   IFCR    @ 0x18
//   TXDR    @ 0x20
//   RXDR    @ 0x30
//   CRCPOLY @ 0x40
//   TXCRC   @ 0x44
//   RXCRC   @ 0x48
//   UDRDR   @ 0x4C
//   I2SCFGR @ 0x50
//

using System.Collections.Generic;
using Antmicro.Renode.Core;
using Antmicro.Renode.Core.Structure.Registers;
using Antmicro.Renode.Logging;
using Antmicro.Renode.Peripherals.Bus;

namespace Antmicro.Renode.Peripherals.Miscellaneous
{
    public class STM32N6_SPI : BasicDoubleWordPeripheral, IKnownSize
    {
        public STM32N6_SPI(IMachine machine) : base(machine)
        {
            receiveFifo = new Queue<uint>();
            IRQ = new GPIO();
            DefineRegisters();
        }

        public long Size => 0x400;

        public GPIO IRQ { get; }

        public override void Reset()
        {
            base.Reset();
            receiveFifo.Clear();
            txComplete = false;
            endOfTransfer = false;
            ierValue = 0;
            IRQ.Unset();
        }

        private void DefineRegisters()
        {
            // CR1 @ 0x00: Control Register 1
            Registers.CR1.Define(this)
                .WithFlag(0, out speEnabled, name: "SPE",
                    changeCallback: (_, val) =>
                    {
                        if(!val)
                        {
                            // Disable: clear FIFOs and completion flags
                            receiveFifo.Clear();
                            txComplete = false;
                            endOfTransfer = false;
                            cstart.Value = false;
                        }
                    })
                .WithFlag(8, name: "MASRX")
                .WithFlag(9, out cstart, FieldMode.Read | FieldMode.Set,
                    name: "CSTART",
                    changeCallback: (_, val) =>
                    {
                        if(val)
                        {
                            // New transfer: clear previous EOT/TXC
                            endOfTransfer = false;
                            txComplete = false;
                        }
                    })
                .WithFlag(10, name: "CSUSP")
                .WithFlag(11, name: "HDDIR")
                .WithFlag(12, name: "SSI")
                .WithFlag(13, name: "CRC33_17")
                .WithFlag(14, name: "RCRCINI")
                .WithFlag(15, name: "TCRCINI")
                .WithFlag(16, name: "IOLOCK")
                .WithReservedBits(17, 15);

            // CR2 @ 0x04: Control Register 2 (TSIZE)
            Registers.CR2.Define(this)
                .WithValueField(0, 32, name: "CR2");

            // CFG1 @ 0x08: Configuration Register 1
            Registers.CFG1.Define(this)
                .WithValueField(0, 32, name: "CFG1");

            // CFG2 @ 0x0C: Configuration Register 2
            Registers.CFG2.Define(this)
                .WithValueField(0, 32, name: "CFG2");

            // IER @ 0x10: Interrupt Enable Register
            Registers.IER.Define(this)
                .WithValueField(0, 32,
                    writeCallback: (_, val) =>
                    {
                        ierValue = (uint)val;
                        UpdateInterrupt();
                    },
                    name: "IER");

            // SR @ 0x14: Status Register (dynamic)
            Registers.SR.Define(this)
                .WithFlag(0, FieldMode.Read, name: "RXP",
                    valueProviderCallback: _ => receiveFifo.Count > 0)
                .WithFlag(1, FieldMode.Read, name: "TXP",
                    valueProviderCallback: _ => speEnabled.Value)
                .WithFlag(2, FieldMode.Read, name: "DXP",
                    valueProviderCallback: _ =>
                        speEnabled.Value && receiveFifo.Count > 0)
                .WithFlag(3, FieldMode.Read, name: "EOT",
                    valueProviderCallback: _ => endOfTransfer)
                .WithFlag(4, FieldMode.Read, name: "TXTF")
                .WithFlag(5, FieldMode.Read, name: "UDR")
                .WithFlag(6, FieldMode.Read, name: "OVR")
                .WithFlag(7, FieldMode.Read, name: "CRCE")
                .WithFlag(8, FieldMode.Read, name: "TIFRE")
                .WithFlag(9, FieldMode.Read, name: "MODF")
                .WithFlag(10, FieldMode.Read, name: "TSERF")
                .WithFlag(11, FieldMode.Read, name: "SUSP")
                .WithFlag(12, FieldMode.Read, name: "TXC",
                    valueProviderCallback: _ => txComplete)
                .WithValueField(13, 2, FieldMode.Read, name: "RXPLVL",
                    valueProviderCallback: _ =>
                        (ulong)(receiveFifo.Count > 3 ?
                            3 : receiveFifo.Count))
                .WithFlag(15, FieldMode.Read, name: "RXWNE",
                    valueProviderCallback: _ => receiveFifo.Count > 0)
                .WithValueField(16, 16, FieldMode.Read, name: "CTSIZE");

            // IFCR @ 0x18: Interrupt/Status Flags Clear (W1C)
            Registers.IFCR.Define(this)
                .WithReservedBits(0, 3)
                .WithFlag(3, FieldMode.Write, name: "EOTC",
                    writeCallback: (_, val) =>
                    {
                        if(val)
                        {
                            endOfTransfer = false;
                            cstart.Value = false;
                            UpdateInterrupt();
                        }
                    })
                .WithFlag(4, FieldMode.Write, name: "TXTFC")
                .WithFlag(5, FieldMode.Write, name: "UDRC")
                .WithFlag(6, FieldMode.Write, name: "OVRC")
                .WithFlag(7, FieldMode.Write, name: "CRCEC")
                .WithFlag(8, FieldMode.Write, name: "TIFREC")
                .WithFlag(9, FieldMode.Write, name: "MODFC")
                .WithFlag(10, FieldMode.Write, name: "TSERFC")
                .WithFlag(11, FieldMode.Write, name: "SUSPC")
                .WithReservedBits(12, 20);

            // TXDR @ 0x20: Transmit Data Register
            // Write while SPE: loopback into RX FIFO (L3)
            Registers.TXDR.Define(this)
                .WithValueField(0, 32, FieldMode.Write, name: "TXDR",
                    writeCallback: (_, value) =>
                    {
                        if(!speEnabled.Value)
                        {
                            this.Log(LogLevel.Warning,
                                "TXDR write while SPE=0 ignored");
                            return;
                        }

                        // Instant loopback: TX byte appears in RX
                        receiveFifo.Enqueue((uint)value);
                        txComplete = true;
                        endOfTransfer = true;
                        UpdateInterrupt();
                    });

            // RXDR @ 0x30: Receive Data Register
            Registers.RXDR.Define(this)
                .WithValueField(0, 32, FieldMode.Read, name: "RXDR",
                    valueProviderCallback: _ =>
                    {
                        if(receiveFifo.Count == 0)
                        {
                            this.Log(LogLevel.Warning,
                                "RXDR read while FIFO empty");
                            return 0;
                        }

                        return receiveFifo.Dequeue();
                    });

            // CRC / underrun stubs so driver config writes do not fault
            Registers.CRCPOLY.Define(this)
                .WithValueField(0, 32, name: "CRCPOLY");

            Registers.TXCRC.Define(this)
                .WithValueField(0, 32, FieldMode.Read, name: "TXCRC");

            Registers.RXCRC.Define(this)
                .WithValueField(0, 32, FieldMode.Read, name: "RXCRC");

            Registers.UDRDR.Define(this)
                .WithValueField(0, 32, name: "UDRDR");

            Registers.I2SCFGR.Define(this)
                .WithValueField(0, 32, name: "I2SCFGR");
        }

        private readonly Queue<uint> receiveFifo;
        private IFlagRegisterField speEnabled;
        private IFlagRegisterField cstart;
        private bool txComplete;
        private bool endOfTransfer;
        private uint ierValue;

        private void UpdateInterrupt()
        {
            // EOTIE is bit 3 in IER (STM32N6 SPI_IER_EOTIE)
            var eotie = (ierValue & (1u << 3)) != 0;
            IRQ.Set(eotie && endOfTransfer);
        }

        private enum Registers : long
        {
            CR1     = 0x00,
            CR2     = 0x04,
            CFG1    = 0x08,
            CFG2    = 0x0C,
            IER     = 0x10,
            SR      = 0x14,
            IFCR    = 0x18,
            TXDR    = 0x20,
            RXDR    = 0x30,
            CRCPOLY = 0x40,
            TXCRC   = 0x44,
            RXCRC   = 0x48,
            UDRDR   = 0x4C,
            I2SCFGR = 0x50,
        }
    }
}
