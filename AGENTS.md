# AGENTS.md

## Cursor Cloud specific instructions

### Project overview

ZSWatch is an embedded C firmware project for a BLE smartwatch prototype targeting the **nRF5340 DK** with an **ST X-NUCLEO-IKS01A3** sensor shield. It runs on **Zephyr RTOS** via the **nRF Connect SDK v3.1.0**.

### Toolchain locations

| Component | Path |
|---|---|
| nRF Connect SDK (includes Zephyr) | `~/ncs` |
| Zephyr SDK (ARM cross-compiler) | `/tmp/zephyr-sdk-0.17.1` |
| `west` meta-tool | `~/.local/bin/west` |

### Required environment variables

Before running any build command, export:

```sh
export PATH="$HOME/.local/bin:$PATH"
export ZEPHYR_BASE=~/ncs/zephyr
export ZEPHYR_SDK_INSTALL_DIR=/tmp/zephyr-sdk-0.17.1
```

### Build

```sh
cd /workspace/App/ZSWatch
west build -b nrf5340dk/nrf5340/cpuapp --sysbuild -- -DSHIELD=x_nucleo_iks01a3
```

Clean rebuild: add `--pristine` or delete `build/` first.

### Lint

Zephyr's `checkpatch.pl` is the primary linting tool:

```sh
$ZEPHYR_BASE/scripts/checkpatch.pl --no-tree -f <file.c>
```

`clang-format` is also available for formatting checks.

### Testing

No automated test suite exists for the main ZSWatch app. This firmware targets physical hardware (nRF5340 DK); end-to-end testing requires the board and a BLE client (e.g. nRF Connect mobile app). The build itself (which compiles all source and links the ELF) is the primary CI-verifiable check.

### Key gotchas

- `ninja-build` must be installed (`apt install ninja-build`) as Zephyr's CMake uses Ninja by default.
- The Zephyr SDK is installed to `/tmp/` — it will not survive VM reboots. The update script reinstalls it.
- `CONFIG_ZSL=y` in `prj.conf` enables the zscilib sensor fusion library (pulled in via the nRF Connect SDK modules).
- `sysbuild.conf` enables `SB_CONFIG_NETCORE_HCI_IPC=y` so `west build --sysbuild` also builds the BLE network-core firmware (`hci_ipc`).
- The `west` tool must be on PATH (`~/.local/bin`).
