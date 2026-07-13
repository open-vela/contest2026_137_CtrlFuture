//
// Copyright (c) 2026 CtrlFuture
//
// SPDX-License-Identifier: Apache-2.0
//
// STM32N6 Digital Temperature Sensor (DTS) model for Renode.
// Minimal model: register read/write without actual temperature sensing.
//
// Registers:
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
