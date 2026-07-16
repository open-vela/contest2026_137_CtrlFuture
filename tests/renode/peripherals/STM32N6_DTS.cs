//
// Copyright (c) 2026 CtrlFuture
//
// SPDX-License-Identifier: Apache-2.0
//
// STM32N6 Digital Temperature Sensor (DTS) model for Renode.
// Minimal model: register read/write without actual temperature sensing.
//
// IMPORTANT: this is a team-defined placeholder register layout,
// NOT the real CMSIS DTS_TypeDef.  The real STM32N6 DTS_TypeDef
// (verified in stm32n647xx.h) has 0x00-0x0F entirely RESERVED; the
// first real register is PVTREG_LOCKR @ 0x10, followed by
// PVTLOCK_SR=0x14, PVTTMR_CR=0x20, PVTTMR_SR=0x24, PVT_IER=0x40,
// and a TSC (temperature sensor controller) block starting at
// 0x80.  This model's CFGR1/CFGR2/T0VALR1/T0VALR2 at
// 0x00/0x04/0x08/0x0C fall entirely inside that reserved region and
// do not correspond to any real register -- do NOT use this layout
// as a reference when writing a real DTS driver.
//
// Registers (placeholder, not CMSIS):
//   CFGR1   @ 0x00: Configuration Register 1
//   CFGR2   @ 0x04: Configuration Register 2
//   T0VALR1 @ 0x08: T0 Value Register 1
//   T0VALR2 @ 0x0C: T0 Value Register 2
//   TSLPTR  @ 0x14: Low-Power Timeout Register
//   TEMPHYSR @ 0x18: Temperature Hypervisor Register
//

using Antmicro.Renode.Core;
using Antmicro.Renode.Core.Structure.Registers;
using Antmicro.Renode.Logging;
using Antmicro.Renode.Peripherals.Bus;

namespace Antmicro.Renode.Peripherals.Miscellaneous
{
    public class STM32N6_DTS : BasicDoubleWordPeripheral, IKnownSize
    {
        public STM32N6_DTS(IMachine machine) : base(machine)
        {
            DefineRegisters();
        }

        public long Size => 0x100;

        private void DefineRegisters()
        {
            // CFGR1 @ 0x00: Configuration Register 1
            Registers.CFGR1.Define(this)
                .WithValueField(0, 32, name: "CFGR1");

            // CFGR2 @ 0x04: Configuration Register 2
            Registers.CFGR2.Define(this)
                .WithValueField(0, 32, name: "CFGR2");

            // T0VALR1 @ 0x08: T0 Value Register 1 (read-only)
            Registers.T0VALR1.Define(this)
                .WithValueField(0, 32, FieldMode.Read, name: "T0VALR1");

            // T0VALR2 @ 0x0C: T0 Value Register 2 (read-only)
            Registers.T0VALR2.Define(this)
                .WithValueField(0, 32, FieldMode.Read, name: "T0VALR2");

            // TSLPTR @ 0x14: Low-Power Timeout Register
            Registers.TSLPTR.Define(this)
                .WithValueField(0, 32, name: "TSLPTR");

            // TEMPHYSR @ 0x18: Temperature Hypervisor Register
            Registers.TEMPHYSR.Define(this)
                .WithValueField(0, 32, name: "TEMPHYSR");
        }

        private enum Registers : long
        {
            CFGR1    = 0x00,
            CFGR2    = 0x04,
            T0VALR1  = 0x08,
            T0VALR2  = 0x0C,
            TSLPTR   = 0x14,
            TEMPHYSR = 0x18,
        }
    }
}
