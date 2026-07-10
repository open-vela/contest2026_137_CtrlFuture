#!/usr/bin/env python3
"""Run Renode smoke test with UART log capture."""

import subprocess
import telnetlib
import time
import os
import re
import sys

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
RESC = os.path.join(SCRIPT_DIR, "smoke-test.resc")
UART_LOG = os.path.join(SCRIPT_DIR, "uart_output.log")

# Clean up
if os.path.exists(UART_LOG):
    os.unlink(UART_LOG)

proc = subprocess.Popen(["renode", "--disable-xwt", RESC])
time.sleep(3)

try:
    tn = telnetlib.Telnet("127.0.0.1", 1234, timeout=5)
    # Drain banner
    time.sleep(0.5)
    try:
        tn.read_until(b"(stm32n6)", timeout=3)
    except Exception:
        pass

    def cmd(c):
        """Send a monitor command and return the response text."""
        tn.write((c + "\n").encode())
        # Wait for the next prompt
        data = tn.read_until(b"(stm32n6)", timeout=5)
        text = data.decode(errors="replace")
        # Strip ANSI codes
        text = re.sub(r'\x1b\[[0-9;]*m', '', text)
        # Strip the echoed command and prompt
        text = text.strip()
        # Remove the command echo (first line) and prompt (last line)
        lines = text.split('\n')
        # Filter out empty lines, command echo, and prompt
        result_lines = []
        for line in lines:
            line = line.strip().replace('\r', '')
            if line and line != c and '(stm32n6)' not in line:
                result_lines.append(line)
        return '\n'.join(result_lines)

    # Enable UART logging
    cmd("logFile @{} -m usart1".format(UART_LOG))

    # Snapshot 1: t=3s
    print("=== Snapshot 1 (t=3s) ===", flush=True)
    pc = cmd("cpu PC")
    sp = cmd("cpu SP")
    systick = cmd("sysbus ReadDoubleWord 0xE000E010")
    systick_rvr = cmd("sysbus ReadDoubleWord 0xE000E014")
    usart_isr = cmd("sysbus ReadDoubleWord 0x4200101C")
    usart_cr1 = cmd("sysbus ReadDoubleWord 0x42001000")
    usart_brr = cmd("sysbus ReadDoubleWord 0x4200100C")
    usart_tdr = cmd("sysbus ReadDoubleWord 0x42001028")
    print(f"  PC={pc} SP={sp}", flush=True)
    print(f"  SysTick_CTRL={systick} RVR={systick_rvr}", flush=True)
    print(f"  USART_ISR={usart_isr} CR1={usart_cr1} "
          f"BRR={usart_brr} TDR={usart_tdr}", flush=True)

    # Snapshot 2: t=8s
    time.sleep(5)
    print("=== Snapshot 2 (t=8s) ===", flush=True)
    pc2 = cmd("cpu PC")
    sp2 = cmd("cpu SP")
    systick2 = cmd("sysbus ReadDoubleWord 0xE000E010")
    usart_isr2 = cmd("sysbus ReadDoubleWord 0x4200101C")
    print(f"  PC={pc2} SP={sp2}", flush=True)
    print(f"  SysTick_CTRL={systick2} USART_ISR={usart_isr2}", flush=True)

    # Check NuttX state
    print("=== NuttX State ===", flush=True)
    readytorun = cmd("sysbus ReadDoubleWord 0x340349E4")
    idletcb = cmd("sysbus ReadDoubleWord 0x34035D90")
    print(f"  g_readytorun={readytorun} g_idletcb_pid={idletcb}", flush=True)

    # NVIC
    iser0 = cmd("sysbus ReadDoubleWord 0xE000E100")
    iser4 = cmd("sysbus ReadDoubleWord 0xE000E110")
    ispr = cmd("sysbus ReadDoubleWord 0xE000E200")
    icsr = cmd("sysbus ReadDoubleWord 0xE000ED04")
    primask = cmd("cpu PRIMASK")
    print(f"  NVIC ISER0(0-31)={iser0} ISER4(128-159)={iser4}", flush=True)
    print(f"  NVIC ISPR[0]={ispr} ICSR={icsr}", flush=True)
    print(f"  PRIMASK={primask}", flush=True)

    # Check memory at specific locations
    print("=== Memory Dumps ===", flush=True)
    # Check if _stext has valid code
    stext = cmd("sysbus ReadDoubleWord 0x34000400")
    print(f"  [0x34000400]={stext}", flush=True)

    tn.close()

    # Check UART log
    print("\n=== UART Log ===", flush=True)
    if os.path.exists(UART_LOG):
        with open(UART_LOG, "rb") as f:
            uart_data = f.read()
        print(f"  Size: {len(uart_data)} bytes", flush=True)
        if uart_data:
            print(f"  Hex: {uart_data[:100].hex()}", flush=True)
            try:
                print(f"  Text: {uart_data[:200].decode('ascii', errors='replace')}",
                      flush=True)
            except Exception:
                pass
        else:
            print("  (empty)", flush=True)
    else:
        print("  No log file created", flush=True)

except Exception as e:
    print(f"Error: {e}", flush=True)
    import traceback
    traceback.print_exc()

proc.terminate()
try:
    proc.wait(timeout=3)
except Exception:
    proc.kill()
print("\nDONE", flush=True)
