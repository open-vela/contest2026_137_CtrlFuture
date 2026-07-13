//
// Copyright (c) 2026 CtrlFuture
//
// SPDX-License-Identifier: Apache-2.0
//
// STM32N6 DCMIPP model for Renode.
// Minimal model: register read/write without actual camera input.
//
// Registers:
//   CR   @ 0x00: Control Register
//   SR   @ 0x04: Status Register
//   IER  @ 0x10: Interrupt Enable Register
//   SRCR @ 0x14: Set and Clear Register
//   PIPE0 registers @ 0x100-0x1FF: Display pipe
//   PIPE1 registers @ 0x200-0x2FF: NN inference pipe
//

using Antmicro.Renode.Core;
using Antmicro.Renode.Core.Structure.Registers;
using Antmicro.Renode.Logging;
using Antmicro.Renode.Peripherals.Bus;

namespace Antmicro.Renode.Peripherals.Miscellaneous
{
    public class STM32N6_DCMIPP : BasicDoubleWordPeripheral, IKnownSize
    {
        public STM32N6_DCMIPP(IMachine machine) : base(machine)
        {
            DefineRegisters();
        }

        public long Size => 0x400;

        private void DefineRegisters()
        {
            // CR @ 0x00: Control Register
            Registers.CR.Define(this)
                .WithValueField(0, 32, name: "CR");

            // SR @ 0x04: Status Register
            Registers.SR.Define(this)
                .WithValueField(0, 32, name: "SR");

            // IER @ 0x10: Interrupt Enable Register
            Registers.IER.Define(this)
                .WithValueField(0, 32, name: "IER");

            // SRCR @ 0x14: Set and Clear Register
            Registers.SRCR.Define(this)
                .WithValueField(0, 32, name: "SRCR");

            // PIPE0 registers @ 0x100-0x1FF: Display pipe
            // P0CR @ 0x100: Pipe 0 Configuration
            Registers.P0CR.Define(this)
                .WithValueField(0, 32, name: "P0CR");

            // P0SR @ 0x104: Pipe 0 Status
            Registers.P0SR.Define(this)
                .WithValueField(0, 32, name: "P0SR");

            // P0IER @ 0x108: Pipe 0 Interrupt Enable
            Registers.P0IER.Define(this)
                .WithValueField(0, 32, name: "P0IER");

            // P0SRCR @ 0x10C: Pipe 0 Set and Clear
            Registers.P0SRCR.Define(this)
                .WithValueField(0, 32, name: "P0SRCR");

            // PIPE1 registers @ 0x200-0x2FF: NN inference pipe
            // P1CR @ 0x200: Pipe 1 Configuration
            Registers.P1CR.Define(this)
                .WithValueField(0, 32, name: "P1CR");

            // P1SR @ 0x204: Pipe 1 Status
            Registers.P1SR.Define(this)
                .WithValueField(0, 32, name: "P1SR");

            // P1IER @ 0x208: Pipe 1 Interrupt Enable
            Registers.P1IER.Define(this)
                .WithValueField(0, 32, name: "P1IER");

            // P1SRCR @ 0x20C: Pipe 1 Set and Clear
            Registers.P1SRCR.Define(this)
                .WithValueField(0, 32, name: "P1SRCR");
        }

        private enum Registers : long
        {
            CR = 0x00,
            SR = 0x04,
            IER = 0x10,
            SRCR = 0x14,
            // Pipe 0 (display)
            P0CR = 0x100,
            P0SR = 0x104,
            P0IER = 0x108,
            P0SRCR = 0x10C,
            // Pipe 1 (NN inference)
            P1CR = 0x200,
            P1SR = 0x204,
            P1IER = 0x208,
            P1SRCR = 0x20C,
        }
    }
}
