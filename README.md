# nucleo-f401re-sensor-node

Firmware for a small sensor node on an ST NUCLEO-F401RE: it samples an analog
input once a second, reports over the board's serial port, takes a few
commands back, and toggles the LED from the user button. A watchdog reboots it
if the main loop ever stops.

Built with STM32CubeMX (HAL, Makefile toolchain) plus a few application
modules under `App/`.

## What it does

- **Console** on USART2 (the ST-LINK virtual COM port), 115200 8N1. Lines end
  with CR or LF. Commands:
  - `help`
  - `status` – uptime, sample count, last reading, LED state
  - `led on|off`
- **Sampler**: ADC1 channel 0 (PA0, Arduino `A0`) every 1000 ms, printed as
  millivolts against a 3.3 V reference: `[12 s] sample: 1650 mV`.
- **Button**: B1 (PC13) toggles LD2 (PA5) and prints `button: press N, LED on|off`.
  The interrupt only records the press; the main loop acts on it.
- **Watchdog**: IWDG, LSI/64 with a reload of 2000 (about 4 s). The main loop
  refreshes it every pass. The boot banner says why the board reset
  (`power-on or reset pin`, `software`, `watchdog`).

## Pins

| Signal | Pin  | Nucleo      | Notes                         |
| ------ | ---- | ----------- | ----------------------------- |
| ADC in | PA0  | A0 (CN8-1)  | 0 to 3.3 V                    |
| LD2    | PA5  | on board    | green LED, active high        |
| B1     | PC13 | on board    | user button, active low, EXTI |
| TX     | PA2  | ST-LINK VCP | USART2                        |
| RX     | PA3  | ST-LINK VCP | USART2, interrupt driven      |

## Build

Needs the Arm GNU toolchain (`arm-none-eabi-gcc`) and `make`.

```
make -j
```

produces `build/f401-sensor-node.elf`, `.hex` and `.bin`. `make clean` removes
`build/`.

## Flash

Any of:

- copy `build/f401-sensor-node.bin` onto the `NODE_F401RE` drive the ST-LINK
  exposes;
- `st-flash write build/f401-sensor-node.bin 0x08000000` (stlink tools);
- `openocd -f board/st_nucleo_f4.cfg -c "program build/f401-sensor-node.elf verify reset exit"`.

Then open the virtual COM port at 115200 and type `help`.

## Layout

```
App/            console.c, sampler.c, button.c (the application)
Core/           CubeMX-generated main, interrupts, MSP, HAL config
Drivers/        ST HAL and CMSIS (vendor code, under its own license)
f401-sensor-node.ioc   the CubeMX project; regenerate with "Generate Code"
Makefile        CubeMX Makefile, with App/ added
```

Application code lives in `App/` and in the `USER CODE` sections of
`Core/Src/main.c`, so regenerating from the `.ioc` keeps it.

## License

MIT, see `LICENSE`. ST's HAL and Arm's CMSIS under `Drivers/` keep the
licenses that come with them.
