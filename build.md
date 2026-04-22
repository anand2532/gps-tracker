# GPS Tracker Build and Flash Guide

This project uses PlatformIO with the Arduino framework for the ESP32 FireBeetle board.

## Board and Environment

- PlatformIO environment: `firebeetle32`
- MCU platform: `espressif32`
- Board: `firebeetle32` (FireBeetle-ESP32)
- Framework: `arduino`

## Prerequisites

Install PlatformIO Core (CLI):

```bash
python3 -m pip install --user -U platformio
```

Verify installation:

```bash
pio --version
```

## Build the Firmware

From the project root:

```bash
pio run
```

This compiles the firmware for `firebeetle32` and creates output files under:

- `.pio/build/firebeetle32/firmware.bin`
- `.pio/build/firebeetle32/firmware.elf`

## Flash (Upload) to the ESP32

1. Connect the board over USB.
2. Run:

```bash
pio run -t upload
```

If auto-detection fails, specify the serial port explicitly (example):

```bash
pio run -t upload --upload-port /dev/ttyUSB0
```

## Serial Monitor

After flashing, open serial output:

```bash
pio device monitor -b 115200
```

If needed, specify the monitor port:

```bash
pio device monitor -b 115200 -p /dev/ttyUSB0
```

Exit monitor with `Ctrl+]`.

## One-Command Build + Flash

```bash
pio run -t upload
```

This command builds automatically before upload, so a separate `pio run` is optional.

## Common Linux Port Checks

List serial devices:

```bash
ls /dev/ttyUSB* /dev/ttyACM* 2>/dev/null
```

See recent kernel USB/serial logs:

```bash
dmesg | rg -i "tty|usb|cp210|ch340|ftdi"
```

## Typical Workflow

1. Edit code in `src/main.cpp`.
2. Build: `pio run`
3. Flash: `pio run -t upload`
4. Monitor logs: `pio device monitor -b 115200`

## Current Firmware Behavior (AP Web Dashboard)

The current firmware serves a full web dashboard directly from the ESP32:

- ESP32 runs in Access Point mode.
- Captive-portal style redirects are enabled for common probe endpoints.
- Frontend assets are served from LittleFS.
- `/api/gps` returns live dummy telemetry for the map and status cards.

### Dummy GPS Data (Delhi)

The firmware currently uses simulated GPS values centered around Delhi (Connaught Place area):

- Latitude anchor: `28.6139`
- Longitude anchor: `77.2090`
- `valid` is always true in dummy mode so data appears immediately.
- Speed, satellites, signal, and battery are simulated with non-blocking updates.

### Web Map Notes

- Map initial center is Delhi (same as firmware dummy origin).
- Marker updates every ~1-2 seconds from `/api/gps`.
- `Center map` button flies to the current marker position.
- Leaflet assets are served locally from `/vendor/leaflet/` (no internet required).
- Offline tiles are served from `/tiles/{z}/{x}/{y}.png` for a small Delhi area.

### Required Deploy Sequence For Web Changes

When firmware or frontend changes are made, use:

```bash
pio run
pio run -t upload
pio run -t uploadfs
```

If HTML/CSS/JS changes are not visible on phone/browser, re-run `pio run -t uploadfs`.

### Offline Delhi Map (AP-Only Mode)

This project now supports offline map operation in AP mode:

- Local Leaflet files:
  - `data/vendor/leaflet/leaflet.js`
  - `data/vendor/leaflet/leaflet.css`
- Local offline tile pack:
  - `data/tiles/12/...`
  - `data/tiles/13/...`
  - `data/tiles/14/...`

To deploy offline map updates, always upload filesystem after changes:

```bash
pio run -t buildfs
pio run -t uploadfs
```

If you expand tile coverage and `buildfs` fails due to size, reduce tile count or zoom range.

## Troubleshooting

- **Permission denied on serial port**: add your user to `dialout`, then re-login.
  ```bash
  sudo usermod -aG dialout $USER
  ```
- **Board not detected**: try another USB cable (data-capable), port, or press reset/boot as required by your board.
- **Wrong port**: pass `--upload-port` and `-p` explicitly.
- **Upload fails intermittently**: close other serial monitor apps and retry.

