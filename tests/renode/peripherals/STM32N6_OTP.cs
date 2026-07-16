//
// Copyright (c) 2026 CtrlFuture
//
// SPDX-License-Identifier: Apache-2.0
//
// STM32N6 OTP (One-Time Programmable Memory) model for Renode.
// Minimal model: register read/write without actual OTP operations.
//
// IMPORTANT: this is a team-defined placeholder register layout,
// NOT the real CMSIS BSEC_TypeDef.  STM32N6 OTP is implemented via
// the BSEC (Boot and Security) fuse controller, which is a fuse
// array of 384 32-bit words (FVRw[384] @ 0x000-0x5FC) plus lock/
// status register blocks (SPLOCKx/SWLOCKx/SRLOCKx/OTPVLDRx/SFSRx
// starting around 0x800) and a control block around 0xC00-0xE44
// (OTPCR=0xC04, WDR=0xC08, SR=0xE40, OTPSR=0xE44 -- verified in
// stm32n647xx.h).  This model's CR/SR/AR/DR at 0x00/0x04/0x08/0x0C
// do not correspond to any of those real registers -- do NOT use
// this layout as a reference when writing a real BSEC/OTP driver.
//
// Registers (placeholder, not CMSIS):
//   CR  @ 0x00: Control Register
//   SR  @ 0x04: Status Register
//   AR  @ 0x08: Address Register
//   DR  @ 0x0C: Data Register
//

using Antmicro.Renode.Core;
using Antmicro.Renode.Core.Structure.Registers;
using Antmicro.Renode.Logging;
using Antmicro.Renode.Peripherals.Bus;

namespace Antmicro.Renode.Peripherals.Miscellaneous
{
    public class STM32N6_OTP : BasicDoubleWordPeripheral, IKnownSize
    {
        public STM32N6_OTP(IMachine machine) : base(machine)
        {
            DefineRegisters();
        }

        public long Size => 0x400;

        private void DefineRegisters()
        {
            // CR @ 0x00: Control Register
            Registers.CR.Define(this)
                .WithValueField(0, 32, name: "CR");

            // SR @ 0x04: Status Register (read-only)
            Registers.SR.Define(this)
                .WithValueField(0, 32, FieldMode.Read, name: "SR");

            // AR @ 0x08: Address Register
            Registers.AR.Define(this)
                .WithValueField(0, 32, name: "AR");

            // DR @ 0x0C: Data Register
            Registers.DR.Define(this)
                .WithValueField(0, 32, name: "DR");
        }

        private enum Registers : long
        {
            CR  = 0x00,
            SR  = 0x04,
            AR  = 0x08,
            DR  = 0x0C,
        }
    }
}
