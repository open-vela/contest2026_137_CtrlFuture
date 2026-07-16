//
// Copyright (c) 2026 CtrlFuture
//
// SPDX-License-Identifier: Apache-2.0
//
// STM32N6 CSI-2 MIPI Interface model for Renode.
// Minimal model: register read/write without actual camera input.
//
// IMPORTANT: this is a team-defined placeholder register layout,
// NOT the real CMSIS CSI_TypeDef.  The real STM32N6 CSI-2 MIPI Host
// + PHY controller (verified in stm32n647xx.h) is a much larger IP
// with CR=0x00, PCR=0x04, VC0CFGR1..4 starting at 0x10, IER0=0x80,
// IER1=0x84, SR0=0x90, SR1=0x94, FCR0=0x100, FCR1=0x104, plus a PHY
// register block starting around 0x1000 (PRCR/PMCR/PFCR).  This
// model's CR/SR/IER/IFR at 0x00/0x04/0x08/0x0C do not correspond to
// any of those real registers -- do NOT use this layout as a
// reference when writing a real CSI driver.
//
// Registers (placeholder, not CMSIS):
//   CR  @ 0x00: Configuration Register
//   SR  @ 0x04: Status Register
//   IER @ 0x08: Interrupt Enable Register
//   IFR @ 0x0C: Interrupt Flag Register
//

using Antmicro.Renode.Core;
using Antmicro.Renode.Core.Structure.Registers;
using Antmicro.Renode.Logging;
using Antmicro.Renode.Peripherals.Bus;

namespace Antmicro.Renode.Peripherals.Miscellaneous
{
    public class STM32N6_CSI : BasicDoubleWordPeripheral, IKnownSize
    {
        public STM32N6_CSI(IMachine machine) : base(machine)
        {
            DefineRegisters();
        }

        public long Size => 0x400;

        private void DefineRegisters()
        {
            // CR @ 0x00: Configuration Register
            Registers.CR.Define(this)
                .WithValueField(0, 32, name: "CR");

            // SR @ 0x04: Status Register (read-only)
            Registers.SR.Define(this)
                .WithValueField(0, 32, FieldMode.Read, name: "SR");

            // IER @ 0x08: Interrupt Enable Register
            Registers.IER.Define(this)
                .WithValueField(0, 32, name: "IER");

            // IFR @ 0x0C: Interrupt Flag Register (write-to-clear)
            Registers.IFR.Define(this)
                .WithValueField(0, 32, name: "IFR");
        }

        private enum Registers : long
        {
            CR  = 0x00,
            SR  = 0x04,
            IER = 0x08,
            IFR = 0x0C,
        }
    }
}
