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
//   AHB1ENR  @ 0x0250: GPDMA1EN(0), ADC1EN(9), ADC2EN(10)
//   AHB2ENR  @ 0x0254: DCMIPPEN(0), SHA2EN(10), RSAEN(11)
//   AHB3ENR  @ 0x0258: RNG1EN(0)
//   AHB4ENR  @ 0x025C: GPIOAEN(0)..GPIOQEN(15), RCCEN(27)
//   AHB5ENR  @ 0x0260: DMA2DEN(0), XSPI1EN(1), SDMMC1EN(4), EMACEN(5), OTGEN(9)
//   APB1ENR1 @ 0x0264: USART2EN(17)..I2C2EN(22), IWDGEN(24), RTCEN(26)
//   APB1ENR2 @ 0x0268: WWDGEN(11), LPTIM1EN(21)..LPTIM5EN(25)
//   APB2ENR  @ 0x026C: USART1EN(4), USART6EN(5), UART9EN(7), USART10EN(8)
//   APB4ENR1 @ 0x0274: EXTIEN(0), I2C4EN(6), LPUART1EN(11), SPDIFEN(16),
//                      DAC1EN(21), COMPEN(25), VREFEN(26), RTCAPBEN(27)
//   APB5ENR  @ 0x027C: TIM1EN(0), TIM8EN(1), LPTIM1EN(2)..LPTIM5EN(6),
//                      TIM15EN(16)..TIM18EN(19)
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

            // AHB1ENR @ 0x0250: AHB1 peripheral clock enable
            Registers.AHB1ENR.Define(this)
                .WithFlag(0, name: "GPDMA1EN")
                .WithReservedBits(1, 8)
                .WithFlag(9, name: "ADC1EN")
                .WithFlag(10, name: "ADC2EN")
                .WithReservedBits(11, 21);

            // AHB2ENR @ 0x0254: AHB2 peripheral clock enable
            Registers.AHB2ENR.Define(this)
                .WithFlag(0, name: "DCMIPPEN")
                .WithReservedBits(1, 9)
                .WithFlag(10, name: "SHA2EN")
                .WithFlag(11, name: "RSAEN")
                .WithReservedBits(12, 20);

            // AHB3ENR @ 0x0258: AHB3 peripheral clock enable
            Registers.AHB3ENR.Define(this)
                .WithFlag(0, name: "RNG1EN")
                .WithReservedBits(1, 31);

            // AHB5ENR @ 0x0260: AHB5 peripheral clock enable
            Registers.AHB5ENR.Define(this)
                .WithFlag(0, name: "DMA2DEN")
                .WithFlag(1, name: "XSPI1EN")
                .WithFlag(4, name: "SDMMC1EN")
                .WithFlag(5, name: "EMACEN")
                .WithFlag(9, name: "OTGEN")
                .WithReservedBits(10, 22);

            // APB1ENR1 @ 0x0264: APB1 peripheral clock enable 1
            Registers.APB1ENR1.Define(this)
                .WithReservedBits(0, 17)
                .WithFlag(17, name: "USART2EN")
                .WithFlag(18, name: "USART3EN")
                .WithFlag(19, name: "UART4EN")
                .WithFlag(20, name: "UART5EN")
                .WithFlag(21, name: "I2C1EN")
                .WithFlag(22, name: "I2C2EN")
                .WithReservedBits(23, 1)
                .WithFlag(24, name: "IWDGEN")
                .WithReservedBits(25, 1)
                .WithFlag(26, name: "RTCEN")
                .WithReservedBits(27, 5);

            // APB1ENR2 @ 0x0268: APB1 peripheral clock enable 2
            Registers.APB1ENR2.Define(this)
                .WithReservedBits(0, 11)
                .WithFlag(11, name: "WWDGEN")
                .WithReservedBits(12, 9)
                .WithFlag(21, name: "LPTIM1EN")
                .WithFlag(22, name: "LPTIM2EN")
                .WithFlag(23, name: "LPTIM3EN")
                .WithFlag(24, name: "LPTIM4EN")
                .WithFlag(25, name: "LPTIM5EN")
                .WithReservedBits(26, 6);

            // APB2ENR @ 0x026C: APB2 peripheral clock enable
            Registers.APB2ENR.Define(this)
                .WithReservedBits(0, 4)
                .WithFlag(4, name: "USART1EN")
                .WithFlag(5, name: "USART6EN")
                .WithReservedBits(6, 1)
                .WithFlag(7, name: "UART9EN")
                .WithFlag(8, name: "USART10EN")
                .WithReservedBits(9, 23);

            // APB4ENR1 @ 0x0274: APB4 peripheral clock enable 1
            Registers.APB4ENR1.Define(this)
                .WithFlag(0, name: "EXTIEN")
                .WithReservedBits(1, 5)
                .WithFlag(6, name: "I2C4EN")
                .WithReservedBits(7, 4)
                .WithFlag(11, name: "LPUART1EN")
                .WithReservedBits(12, 4)
                .WithFlag(16, name: "SPDIFEN")
                .WithReservedBits(17, 4)
                .WithFlag(21, name: "DAC1EN")
                .WithFlag(25, name: "COMPEN")
                .WithFlag(26, name: "VREFEN")
                .WithFlag(27, name: "RTCAPBEN")
                .WithReservedBits(28, 4);

            // APB5ENR @ 0x027C: APB5 peripheral clock enable
            Registers.APB5ENR.Define(this)
                .WithFlag(0, name: "TIM1EN")
                .WithFlag(1, name: "TIM8EN")
                .WithReservedBits(2, 14)
                .WithFlag(16, name: "TIM15EN")
                .WithFlag(17, name: "TIM16EN")
                .WithFlag(18, name: "TIM17EN")
                .WithFlag(19, name: "TIM18EN")
                .WithReservedBits(20, 12);
        }

        private IFlagRegisterField hsion;
        private IFlagRegisterField hseon;
        private IFlagRegisterField pll1on;

        private enum Registers : long
        {
            CR = 0x0000,
            SR = 0x0004,
            CFGR1 = 0x0018,
            AHB1ENR = 0x0250,
            AHB2ENR = 0x0254,
            AHB3ENR = 0x0258,
            AHB4ENR = 0x025C,
            AHB5ENR = 0x0260,
            APB1ENR1 = 0x0264,
            APB1ENR2 = 0x0268,
            APB2ENR = 0x026C,
            APB4ENR1 = 0x0274,
            APB5ENR = 0x027C,
        }
    }
}
