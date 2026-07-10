//
// Copyright (c) 2026 CtrlFuture
//
// SPDX-License-Identifier: Apache-2.0
//
// STM32N6 RCC (Reset and Clock Control) model for Renode.
// Bit positions match CMSIS stm32n647xx.h RCC_TypeDef.
//
// Key behavior:
//   - CR.HSION (bit 3) set -> SR.HSIRDY (bit 3) reads as 1
//   - CR.HSEON (bit 4) set -> SR.HSERDY (bit 4) reads as 1
//   - CR.PLL1ON (bit 8) set -> SR.PLL1RDY (bit 8) reads as 1
//   - Peripheral clock enable registers (AHB/APB ENR) are RW
//
// CMSIS register map:
//   CR       @ 0x0000: HSION(3), HSEON(4), PLL1ON(8), ...
//   SR       @ 0x0004: HSIRDY(3), HSERDY(4), PLL1RDY(8), ...
//   CFGR1    @ 0x0018: Clock mux SW[2:0]
//   AHB4ENR  @ 0x025C: GPIOxEN
//   APB2ENR  @ 0x026C: USART1EN(4)
//

using Antmicro.Renode.Core;
using Antmicro.Renode.Core.Structure.Registers;
using Antmicro.Renode.Logging;
using Antmicro.Renode.Peripherals.Bus;

namespace Antmicro.Renode.Peripherals.Miscellaneous
{
    public class STM32N6_RCC : BasicDoubleWordPeripheral, IKnownSize
    {
        public STM32N6_RCC(IMachine machine) : base(machine)
        {
            DefineRegisters();
        }

        public long Size => 0x400;

        private void DefineRegisters()
        {
            // CR @ 0x0000: Clock control register
            // HSION (bit 3), HSEON (bit 4), PLL1ON (bit 8)
            // Reset value: 0x00000000
            Registers.CR.Define(this)
                .WithReservedBits(0, 3)
                .WithFlag(3, out hsion, name: "HSION")
                .WithFlag(4, out hseon, name: "HSEON")
                .WithReservedBits(5, 3)
                .WithFlag(8, out pll1on, name: "PLL1ON")
                .WithReservedBits(9, 23);

            // SR @ 0x0004: Clock status register
            // HSIRDY (bit 3), HSERDY (bit 4), PLL1RDY (bit 8)
            // Reset value: 0x00000000
            Registers.SR.Define(this)
                .WithReservedBits(0, 3)
                .WithFlag(3, FieldMode.Read,
                    valueProviderCallback: _ => hsion.Value, name: "HSIRDY")
                .WithFlag(4, FieldMode.Read,
                    valueProviderCallback: _ => hseon.Value, name: "HSERDY")
                .WithReservedBits(5, 3)
                .WithFlag(8, FieldMode.Read,
                    valueProviderCallback: _ => pll1on.Value, name: "PLL1RDY")
                .WithReservedBits(9, 23);

            // CFGR1 @ 0x0018: Clock configuration register 1
            Registers.CFGR1.Define(this)
                .WithValueField(0, 32, name: "CFGR1");

            // AHB4ENR @ 0x025C: AHB4 peripheral clock enable
            // GPIOAEN(0), GPIOBEN(1), GPIOCEN(2), GPIODEN(3),
            // GPIOEEN(4), GPIOFEN(5), GPIOGEN(6), GPIOHEN(7),
            // GPIONEN(12), GPIOOEN(13), GPIOPEN(14), GPIOQEN(15)
            Registers.AHB4ENR.Define(this)
                .WithFlag(0, name: "GPIOAEN")
                .WithFlag(1, name: "GPIOBEN")
                .WithFlag(2, name: "GPIOCEN")
                .WithFlag(3, name: "GPIODEN")
                .WithFlag(4, name: "GPIOEEN")
                .WithFlag(5, name: "GPIOFEN")
                .WithFlag(6, name: "GPIOGEN")
                .WithFlag(7, name: "GPIOHEN")
                .WithReservedBits(8, 4)
                .WithFlag(12, name: "GPIONEN")
                .WithFlag(13, name: "GPIOOEN")
                .WithFlag(14, name: "GPIOPEN")
                .WithFlag(15, name: "GPIOQEN")
                .WithReservedBits(16, 16);

            // APB2ENR @ 0x026C: APB2 peripheral clock enable
            // USART1EN(4)
            Registers.APB2ENR.Define(this)
                .WithReservedBits(0, 4)
                .WithFlag(4, name: "USART1EN")
                .WithReservedBits(5, 27);
        }

        private IFlagRegisterField hsion;
        private IFlagRegisterField hseon;
        private IFlagRegisterField pll1on;

        private enum Registers : long
        {
            CR = 0x0000,
            SR = 0x0004,
            CFGR1 = 0x0018,
            AHB4ENR = 0x025C,
            APB2ENR = 0x026C,
        }
    }
}
