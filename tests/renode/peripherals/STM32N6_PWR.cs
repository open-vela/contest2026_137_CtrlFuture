// STM32N6_PWR - Power controller peripheral
//
// Register map from CMSIS stm32n647xx.h PWR_TypeDef:
//   CR1    @ 0x000: Control register 1
//   CR2    @ 0x004: Control register 2
//   CR3    @ 0x008: Control register 3
//   CR4    @ 0x00C: Control register 4
//   VOSCR  @ 0x020: Voltage scaling control
//   BDCR1  @ 0x024: Backup domain control register 1
//   BDCR2  @ 0x028: Backup domain control register 2
//   DBPCR  @ 0x02C: Disable backup protection control register
//   CPUCR  @ 0x030: CPU control
//   SVMCR1 @ 0x034: Supply voltage monitoring control 1
//   SVMCR2 @ 0x038: Supply voltage monitoring control 2
//   SVMCR3 @ 0x03C: Supply voltage monitoring control 3
//
// Behavior:
//   - VOSCR.VOS (bit 0): write voltage scale, VOSRDY (bit 1) mirrors write
//   - VOSCR.ACTVOS (bit 16): mirrors VOS after write
//   - VOSCR.ACTVOSRDY (bit 17): mirrors VOSRDY
//   - DBPCR.DBP (bit 0): plain read/write, backing
//     stm32n6_pwr_enablebkp()'s read-modify-write + "was it already
//     writable" return value.

using Antmicro.Renode.Core;
using Antmicro.Renode.Core.Structure.Registers;
using Antmicro.Renode.Peripherals.Bus;

namespace Antmicro.Renode.Peripherals.Miscellaneous
{
    public class STM32N6_PWR : BasicDoubleWordPeripheral, IKnownSize
    {
        public STM32N6_PWR(IMachine machine) : base(machine)
        {
            DefineRegisters();
        }

        public long Size => 0x80;

        private void DefineRegisters()
        {
            Registers.VOSCR.Define(this)
                .WithFlag(0, out vos, name: "VOS")
                .WithFlag(1, FieldMode.Read,
                    valueProviderCallback: _ => written, name: "VOSRDY")
                .WithReservedBits(2, 14)
                .WithFlag(16, FieldMode.Read,
                    valueProviderCallback: _ => vos.Value, name: "ACTVOS")
                .WithFlag(17, FieldMode.Read,
                    valueProviderCallback: _ => written, name: "ACTVOSRDY")
                .WithWriteCallback((_, __) => written = true);

            Registers.DBPCR.Define(this)
                .WithFlag(0, name: "DBP")
                .WithReservedBits(1, 31);
        }

        private IFlagRegisterField vos;
        private bool written;

        private enum Registers : long
        {
            CR1 = 0x000,
            CR2 = 0x004,
            CR3 = 0x008,
            CR4 = 0x00C,
            VOSCR = 0x020,
            BDCR1 = 0x024,
            BDCR2 = 0x028,
            DBPCR = 0x02C,
            CPUCR = 0x030,
            SVMCR1 = 0x034,
            SVMCR2 = 0x038,
            SVMCR3 = 0x03C,
        }
    }
}
