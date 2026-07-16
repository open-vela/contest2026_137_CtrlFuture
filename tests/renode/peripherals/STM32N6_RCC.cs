//
// Copyright (c) 2026 CtrlFuture
//
// SPDX-License-Identifier: Apache-2.0
//
// STM32N6 RCC (Reset and Clock Control) model for Renode.
//
// Register offsets/bitfields are cross-checked against CMSIS
// stm32n647xx.h/stm32n657xx.h RCC_TypeDef (identical on both parts)
// and against the upstream Apache NuttX STM32N6 port
// (arch/arm/src/stm32n6/hardware/stm32n6xxx_rcc.h,
// arch/arm/src/stm32n6/stm32n6xx_rcc.c), which targets STM32N657.
//
// Key behavior modeled:
//   - CR.HSION set -> SR.HSIRDY reads as 1 (same for HSEON/HSERDY).
//   - CSR (Clock control Set register, +0x0800) write sets the
//     corresponding CR.xxxON bit; CCR (Clock control Clear register,
//     +0x1000) write clears it.  This mirrors the atomic Set/Clear
//     alias pattern used by real hardware and by the driver (see
//     stm32n6_rcc.c rcc_configpll1()).
//   - CR.PLL1ON set -> SR.PLL1RDY reads as 1 immediately (no real
//     lock-time simulation, matching the existing L2-state fidelity
//     level of this model).
//   - CFGR1 latches after its first write: once CPUSWS/SYSSWS have
//     been set to the IC1/IC2+IC6+IC11 group, further writes to
//     CFGR1 are ignored (modeling the real "second write can hang
//     the part" hardware behavior documented in hardware/
//     stm32_rcc.h and in the upstream NuttX port comment).
//   - IC1/IC2/IC6/IC11 CFGR and DIVENR are plain RW state (no
//     frequency computation modeled).
//   - Peripheral clock enable registers (AHB/APB ENR) are RW, plus
//     their *ENSR (set) / *ENCR (clear) atomic aliases where the
//     driver uses them (AHB4ENSR, APB1ENSR1, APB2ENSR, APB4ENSR1).
//
// CMSIS register map (bit positions verified against
// stm32n647xx.h RCC_TypeDef and RCC_* bitfield macros):
//   CR        @ 0x0000: HSION(3), HSEON(4), PLL1ON(8)
//   SR        @ 0x0004: HSIRDY(3), HSERDY(4), PLL1RDY(8)
//   CFGR1     @ 0x0020: CPUSW(16-17)/CPUSWS(20-21),
//                       SYSSW(24-25)/SYSSWS(28-29)
//   PLL1CFGR1 @ 0x0080: SEL(28-30), DIVM(20-25), DIVN(8-19)
//   PLL1CFGR3 @ 0x0088: PDIVEN(30), PDIV1(27-29), PDIV2(24-26)
//   IC1CFGR   @ 0x00C4, IC2CFGR @ 0x00C8, IC6CFGR @ 0x00D8,
//   IC11CFGR  @ 0x00EC: SEL(28-29), INT(16-23)
//   DIVENR    @ 0x0240: IC1EN(0), IC2EN(1), IC3EN(2), IC6EN(5),
//                       IC11EN(10)
//   AHB1ENR   @ 0x0250: GPDMA1EN(0), ADC1EN(9), ADC2EN(10)
//   AHB2ENR   @ 0x0254: DCMIPPEN(0), SHA2EN(10), RSAEN(11)
//   AHB3ENR   @ 0x0258: RNG1EN(0)
//   AHB4ENR   @ 0x025C: GPIOAEN(0)..GPIOHEN(7), GPIONEN(13),
//                       GPIOOEN(14), GPIOPEN(15), GPIOQEN(16),
//                       PWREN(18)
//   AHB5ENR   @ 0x0260: DMA2DEN(0), XSPI1EN(1), SDMMC1EN(4),
//                       EMACEN(5), OTGEN(9)
//   APB1ENR1  @ 0x0264: TIM2EN(0), USART2EN(17)..I2C2EN(22)
//   APB1ENR2  @ 0x0268: WWDGEN(11), LPTIM1EN(21)..LPTIM5EN(25)
//   APB2ENR   @ 0x026C: USART1EN(4), USART6EN(5), UART9EN(7),
//                       USART10EN(8)
//   APB4ENR1  @ 0x0274: I2C4EN(6), LPUART1EN(11), SPDIFEN(16),
//                       RTCEN(16 -- see note), DAC1EN(21),
//                       COMPEN(25), VREFEN(26), RTCAPBEN(27)
//   APB5ENR   @ 0x027C: TIM1EN(0), TIM8EN(1), TIM15EN(16)..
//                       TIM18EN(19)
//
// Note: RCC_APB4ENR1_RTCEN is bit 16 per CMSIS -- the same bit
// position historically (mis)labeled SPDIFEN in this model; SPDIF
// is not part of the STM32N6 peripheral set, so bit 16 is modeled
// as RTCEN only to match the driver.
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

        public long Size => 0x2000;

        private void DefineRegisters()
        {
            // CR @ 0x0000: Clock control register (read-only from the
            // CPU side in real hardware outside of CSR/CCR aliases;
            // modeled as plain RW flags for simplicity since nothing
            // else in this model writes CR directly).
            Registers.CR.Define(this)
                .WithReservedBits(0, 3)
                .WithFlag(3, out hsion, name: "HSION")
                .WithFlag(4, out hseon, name: "HSEON")
                .WithReservedBits(5, 3)
                .WithFlag(8, out pll1on, name: "PLL1ON")
                .WithReservedBits(9, 23);

            // SR @ 0x0004: Clock status register
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

            // CSR @ 0x0800: Clock control Set register (write-1 sets
            // the matching CR.xxxON bit).
            Registers.CSR.Define(this)
                .WithReservedBits(0, 3)
                .WithFlag(3, FieldMode.Write,
                    writeCallback: (_, val) => { if (val) hsion.Value = true; },
                    name: "HSION_SET")
                .WithFlag(4, FieldMode.Write,
                    writeCallback: (_, val) => { if (val) hseon.Value = true; },
                    name: "HSEON_SET")
                .WithReservedBits(5, 3)
                .WithFlag(8, FieldMode.Write,
                    writeCallback: (_, val) => { if (val) pll1on.Value = true; },
                    name: "PLL1ON_SET")
                .WithReservedBits(9, 23);

            // CCR @ 0x1000: Clock control Clear register (write-1
            // clears the matching CR.xxxON bit).
            Registers.CCR.Define(this)
                .WithReservedBits(0, 3)
                .WithFlag(3, FieldMode.Write,
                    writeCallback: (_, val) => { if (val) hsion.Value = false; },
                    name: "HSION_CLR")
                .WithFlag(4, FieldMode.Write,
                    writeCallback: (_, val) => { if (val) hseon.Value = false; },
                    name: "HSEON_CLR")
                .WithReservedBits(5, 3)
                .WithFlag(8, FieldMode.Write,
                    writeCallback: (_, val) => { if (val) pll1on.Value = false; },
                    name: "PLL1ON_CLR")
                .WithReservedBits(9, 23);

            // CFGR1 @ 0x0020: Clock configuration register 1.
            // Models the real "latches after first write" hardware
            // behavior: once CPUSW/SYSSW have been written to select
            // the IC1/IC2+IC6+IC11 group, further writes to either
            // field are ignored (mirrors the "second CFGR1 write can
            // hang the part" behavior documented in hardware/
            // stm32_rcc.h and in the upstream NuttX port comment).
            // CPUSW and SYSSW are written together in one putreg32()
            // and their field callbacks both fire within the same
            // write transaction; the lock decision is captured once
            // (on the CPUSW callback, which fires first) into
            // acceptThisWrite so SYSSW's callback can reuse it
            // instead of re-reading a lock flag that CPUSW may have
            // just flipped.
            Registers.CFGR1.Define(this)
                .WithReservedBits(0, 16)
                .WithValueField(16, 2, out cpusw,
                    writeCallback: (oldVal, val) =>
                        {
                          acceptThisWrite = !cfgr1Locked;
                          if (!acceptThisWrite)
                            {
                              cpusw.Value = oldVal;
                            }
                          else if (val == CpuswIc1)
                            {
                              cfgr1Locked = true;
                            }
                        },
                    name: "CPUSW")
                .WithReservedBits(18, 2)
                .WithValueField(20, 2, FieldMode.Read,
                    valueProviderCallback: _ => cpusw.Value, name: "CPUSWS")
                .WithReservedBits(22, 2)
                .WithValueField(24, 2, out syssw,
                    writeCallback: (oldVal, val) =>
                        {
                          if (!acceptThisWrite)
                            {
                              syssw.Value = oldVal;
                            }
                        },
                    name: "SYSSW")
                .WithReservedBits(26, 2)
                .WithValueField(28, 2, FieldMode.Read,
                    valueProviderCallback: _ => syssw.Value, name: "SYSSWS");

            // CFGR2 @ 0x0024: Clock configuration register 2 (HPRE)
            Registers.CFGR2.Define(this)
                .WithValueField(0, 32, name: "CFGR2");

            // PLL1CFGR1 @ 0x0080: SEL(28-30), DIVM(20-25), DIVN(8-19)
            Registers.PLL1CFGR1.Define(this)
                .WithValueField(0, 32, name: "PLL1CFGR1");

            // PLL1CFGR3 @ 0x0088: PDIVEN(30), PDIV1(27-29), PDIV2(24-26)
            Registers.PLL1CFGR3.Define(this)
                .WithValueField(0, 32, name: "PLL1CFGR3");

            // IC1/IC2/IC6/IC11 CFGR: SEL(28-29), INT(16-23)
            Registers.IC1CFGR.Define(this)
                .WithValueField(0, 32, name: "IC1CFGR");
            Registers.IC2CFGR.Define(this)
                .WithValueField(0, 32, name: "IC2CFGR");
            Registers.IC3CFGR.Define(this)
                .WithValueField(0, 32, name: "IC3CFGR");
            Registers.IC6CFGR.Define(this)
                .WithValueField(0, 32, name: "IC6CFGR");
            Registers.IC11CFGR.Define(this)
                .WithValueField(0, 32, name: "IC11CFGR");

            // DIVENR @ 0x0240: IC divider enable register
            Registers.DIVENR.Define(this)
                .WithFlag(0, out ic1en, name: "IC1EN")
                .WithFlag(1, out ic2en, name: "IC2EN")
                .WithFlag(2, out ic3en, name: "IC3EN")
                .WithReservedBits(3, 2)
                .WithFlag(5, out ic6en, name: "IC6EN")
                .WithReservedBits(6, 4)
                .WithFlag(10, out ic11en, name: "IC11EN")
                .WithReservedBits(11, 21);

            // DIVENSR @ 0x0A40: IC divider enable Set register
            Registers.DIVENSR.Define(this)
                .WithFlag(0, FieldMode.Write,
                    writeCallback: (_, val) => { if (val) ic1en.Value = true; },
                    name: "IC1EN_SET")
                .WithFlag(1, FieldMode.Write,
                    writeCallback: (_, val) => { if (val) ic2en.Value = true; },
                    name: "IC2EN_SET")
                .WithFlag(2, FieldMode.Write,
                    writeCallback: (_, val) => { if (val) ic3en.Value = true; },
                    name: "IC3EN_SET")
                .WithReservedBits(3, 2)
                .WithFlag(5, FieldMode.Write,
                    writeCallback: (_, val) => { if (val) ic6en.Value = true; },
                    name: "IC6EN_SET")
                .WithReservedBits(6, 4)
                .WithFlag(10, FieldMode.Write,
                    writeCallback: (_, val) => { if (val) ic11en.Value = true; },
                    name: "IC11EN_SET")
                .WithReservedBits(11, 21);

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

            // AHB4ENR @ 0x025C: AHB4 peripheral clock enable.
            // GPIOA-H are contiguous (0-7); GPION/O/P/Q are NOT
            // contiguous with them (13/14/15/16), matching CMSIS
            // RCC_AHB4ENR_GPIOxEN_Pos exactly (verified against
            // stm32n647xx.h and the upstream NuttX port).
            Registers.AHB4ENR.Define(this)
                .WithFlag(0, name: "GPIOAEN")
                .WithFlag(1, name: "GPIOBEN")
                .WithFlag(2, name: "GPIOCEN")
                .WithFlag(3, name: "GPIODEN")
                .WithFlag(4, name: "GPIOEEN")
                .WithFlag(5, name: "GPIOFEN")
                .WithFlag(6, name: "GPIOGEN")
                .WithFlag(7, name: "GPIOHEN")
                .WithReservedBits(8, 5)
                .WithFlag(13, name: "GPIONEN")
                .WithFlag(14, name: "GPIOOEN")
                .WithFlag(15, name: "GPIOPEN")
                .WithFlag(16, name: "GPIOQEN")
                .WithReservedBits(17, 1)
                .WithFlag(18, name: "PWREN")
                .WithReservedBits(19, 13);

            // AHB4ENSR @ 0x0A5C: AHB4 clock enable Set register.
            // Only GPIO bits are modeled as set-aliased since that
            // is the only path stm32n6xx_rcc.c uses.
            Registers.AHB4ENSR.Define(this)
                .WithValueField(0, 32, FieldMode.Write,
                    writeCallback: (_, val) =>
                        {
                          var ahb4 = ReadDoubleWord((long)Registers.AHB4ENR);
                          WriteDoubleWord((long)Registers.AHB4ENR,
                                           ahb4 | (uint)val);
                        },
                    name: "AHB4ENSR");

            // APB1ENR1 @ 0x0264: APB1 peripheral clock enable 1.
            // Note: neither CMSIS nor upstream NuttX define an
            // IWDGEN bit here (IWDG has no software clock gate on
            // STM32N6); RTCEN also does NOT live in this register --
            // it is APB4ENR1 bit 16 (see below).
            Registers.APB1ENR1.Define(this)
                .WithFlag(0, name: "TIM2EN")
                .WithReservedBits(1, 16)
                .WithFlag(17, name: "USART2EN")
                .WithFlag(18, name: "USART3EN")
                .WithFlag(19, name: "UART4EN")
                .WithFlag(20, name: "UART5EN")
                .WithFlag(21, name: "I2C1EN")
                .WithFlag(22, name: "I2C2EN")
                .WithReservedBits(23, 9);

            // APB1ENSR1 @ 0x0A64: APB1 clock enable Set register 1
            Registers.APB1ENSR1.Define(this)
                .WithValueField(0, 32, FieldMode.Write,
                    writeCallback: (_, val) =>
                        {
                          var apb1 = ReadDoubleWord((long)Registers.APB1ENR1);
                          WriteDoubleWord((long)Registers.APB1ENR1,
                                           apb1 | (uint)val);
                        },
                    name: "APB1ENSR1");

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

            // APB2ENSR @ 0x0A6C: APB2 clock enable Set register
            Registers.APB2ENSR.Define(this)
                .WithValueField(0, 32, FieldMode.Write,
                    writeCallback: (_, val) =>
                        {
                          var apb2 = ReadDoubleWord((long)Registers.APB2ENR);
                          WriteDoubleWord((long)Registers.APB2ENR,
                                           apb2 | (uint)val);
                        },
                    name: "APB2ENSR");

            // APB4ENR1 @ 0x0274: APB4 peripheral clock enable 1.
            // RTCEN is bit 16 per CMSIS RCC_APB4ENR1_RTCEN.
            Registers.APB4ENR1.Define(this)
                .WithReservedBits(0, 6)
                .WithFlag(6, name: "I2C4EN")
                .WithReservedBits(7, 4)
                .WithFlag(11, name: "LPUART1EN")
                .WithReservedBits(12, 4)
                .WithFlag(16, name: "RTCEN")
                .WithReservedBits(17, 4)
                .WithFlag(21, name: "DAC1EN")
                .WithReservedBits(22, 3)
                .WithFlag(25, name: "COMPEN")
                .WithFlag(26, name: "VREFEN")
                .WithFlag(27, name: "RTCAPBEN")
                .WithReservedBits(28, 4);

            // APB4ENSR1 @ 0x0A78: APB4 clock enable Set register 1
            Registers.APB4ENSR1.Define(this)
                .WithValueField(0, 32, FieldMode.Write,
                    writeCallback: (_, val) =>
                        {
                          var apb4 = ReadDoubleWord((long)Registers.APB4ENR1);
                          WriteDoubleWord((long)Registers.APB4ENR1,
                                           apb4 | (uint)val);
                        },
                    name: "APB4ENSR1");

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
        private IValueRegisterField cpusw;
        private IValueRegisterField syssw;
        private IFlagRegisterField ic1en;
        private IFlagRegisterField ic2en;
        private IFlagRegisterField ic3en;
        private IFlagRegisterField ic6en;
        private IFlagRegisterField ic11en;
        private bool cfgr1Locked;
        private bool acceptThisWrite;

        private const ulong CpuswIc1 = 3;

        private enum Registers : long
        {
            CR = 0x0000,
            SR = 0x0004,
            CFGR1 = 0x0020,
            CFGR2 = 0x0024,
            PLL1CFGR1 = 0x0080,
            PLL1CFGR3 = 0x0088,
            IC1CFGR = 0x00C4,
            IC2CFGR = 0x00C8,
            IC3CFGR = 0x00CC,
            IC6CFGR = 0x00D8,
            IC11CFGR = 0x00EC,
            DIVENR = 0x0240,
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
            CSR = 0x0800,
            DIVENSR = 0x0A40,
            AHB4ENSR = 0x0A5C,
            APB1ENSR1 = 0x0A64,
            APB2ENSR = 0x0A6C,
            APB4ENSR1 = 0x0A78,
            CCR = 0x1000,
        }
    }
}
