# Raspberry Pi Pico NEOGEO Pocket Color for MURMULATOR devboard

[MURMULATOR](https://github.com/AlexEkb4ever/MURMULATOR_classical_scheme) devboard 
Murmulator devboard have MicroSD card slot, PS/2 keyboard input and VGA output

## Build

Chip and board are selected with `PICO_PLATFORM` and `PICO_BOARD`; pins come from `boards/<board>.h`:

| Firmware prefix | Board | `PICO_PLATFORM` | `PICO_BOARD` |
|---|---|---|---|
| `m1p1` | MURMULATOR 1.x, RP2040 | `rp2040` | `murmulator` |
| `m1p2` | MURMULATOR 1.x, RP2350 | `rp2350` | `murmulator` |
| `PCp1` | Olimex RP2040-PICO-PC, Pico | `rp2040` | `olimex-pico-pc` |
| `PCp2` | Olimex RP2040-PICO-PC, Pico 2 | `rp2350` | `olimex-pico-pc` |

Example (Olimex RP2040-PICO-PC with Pico 2, HDMI, PWM sound):

```
cmake -B build -DPICO_PLATFORM=rp2350 -DPICO_BOARD=olimex-pico-pc -DHDMI=ON -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

ROMs (`.ngp`, `.ngc`) are read from `\NGPC` on the SD card. PS/2 and USB HID keyboards are supported (USB hub allowed).
