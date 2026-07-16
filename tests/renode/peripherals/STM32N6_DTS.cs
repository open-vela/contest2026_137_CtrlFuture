//
// Copyright (c) 2026 CtrlFuture
//
// SPDX-License-Identifier: Apache-2.0
//
// STM32N6 Digital Temperature Sensor (DTS) model for Renode.
//
// L2 model: register read/write with a simple "trigger a
// measurement, read back an externally-injected value" behavior
// chain layered on top of the existing placeholder register set
// (see IMPORTANT note below for why the register offsets
// themselves are not touched).
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
// as a reference when writing a real DTS driver. The offsets are
// kept unchanged from the previous revision (only adding behavior,
// not moving addresses) since there is no NuttX DTS driver in tree
// that a register-layout fix would need to track.
//
// Registers (placeholder, not CMSIS):
//   CFGR1    @ 0x00: Configuration Register 1. Bit 0 ("START", not
//                     a real CMSIS bit) triggers a measurement:
//                     TEMPHYSR is loaded from the externally
//                     injected SIMTEMPR value and TEMPHYSR_VALID
//                     (bit 31 of TEMPHYSR) is set.
//   CFGR2    @ 0x04: Configuration Register 2
//   T0VALR1  @ 0x08: T0 Value Register 1 (read-only, always 0;
//                     kept for backward compatibility with
//                     existing tests)
//   T0VALR2  @ 0x0C: T0 Value Register 2 (read-only, always 0)
//   TSLPTR   @ 0x14: Low-Power Timeout Register
//   TEMPHYSR @ 0x18: Temperature Hypervisor Register (read-only).
//                     Bits [30:0] hold the last measured
//                     temperature sample; bit 31 (VALID) is set
//                     once a measurement has completed.
//   SIMTEMPR @ 0x1F8: team-defined test-only register (not CMSIS,
//                     same convention as STM32N6_ADC.cs's SIMDR).
//                     0x1F8 is far past this model's real register
//                     block (which ends at TEMPHYSR @ 0x18) and the
//                     CMSIS DTS_TypeDef's real end (the Sensor1
//                     block's last register, TSIRQTESTR, ends at
//                     offset 0x110), so it cannot alias any current
//                     or future real register. It is also placed
//                     just before 0x200, where stm32n647x0.repl
//                     tags the rest of the APB4_PERIPH_D address
//                     range as an unimplemented-peripheral stub;
//                     Size below is kept at 0x200 so this model's
//                     own address range does not overlap that Tag.
//                     Holds the simulated temperature value a test
//                     injects before triggering CFGR1.START.
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

        public long Size => 0x200;

        private void DefineRegisters()
        {
            // CFGR1 @ 0x00: Configuration Register 1
            Registers.CFGR1.Define(this)
                .WithFlag(0, writeCallback: (_, val) =>
                    {
                        if (val)
                        {
                            TriggerMeasurement();
                        }
                    }, name: "START")
                .WithValueField(1, 31, name: "CFGR1_REST");

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
            // (read-only). Bits [30:0] = last sample, bit 31 = VALID.
            Registers.TEMPHYSR.Define(this)
                .WithValueField(0, 31, FieldMode.Read,
                    valueProviderCallback: _ => lastTemperatureSample,
                    name: "TEMP_VALUE")
                .WithFlag(31, out measurementValid, FieldMode.Read,
                    name: "VALID");

            // SIMTEMPR @ 0x1F8: not a CMSIS register, see header note.
            Registers.SIMTEMPR.Define(this)
                .WithValueField(0, 32, writeCallback: (_, val) =>
                    {
                        simulatedTemperature = (uint)val;
                    },
                    valueProviderCallback: _ => simulatedTemperature,
                    name: "SIMTEMPR");
        }

        private void TriggerMeasurement()
        {
            lastTemperatureSample = simulatedTemperature & 0x7FFFFFFF;
            measurementValid.Value = true;
        }

        private IFlagRegisterField measurementValid;
        private uint simulatedTemperature;
        private uint lastTemperatureSample;

        private enum Registers : long
        {
            CFGR1    = 0x00,
            CFGR2    = 0x04,
            T0VALR1  = 0x08,
            T0VALR2  = 0x0C,
            TSLPTR   = 0x14,
            TEMPHYSR = 0x18,
            SIMTEMPR = 0x1F8,
        }
    }
}
