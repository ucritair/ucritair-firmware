# uCritAir Firmware

This project uses a Docker-based development environment to ensure a consistent, reproducible build process for all developers. This repository includes a `docker-compose.yaml` file that automates the entire environment setup.

> **Why Docker?**
> By using Docker, you don't need to manually install the Zephyr SDK, toolchains, or any system dependencies on your host machine. The entire build environment is self-contained, preventing conflicts with your local setup and guaranteeing that the firmware builds the same way for everyone.

> **Note:** Zephyr is huge—we are sorry. This Docker image is approximately 17GB. You will likely need to increase Docker Desktop's allowed storage space.

---

## Prerequisites

Install the following tools on your host machine:

1. **Git:** To clone this repository.
2. **Docker & Docker Compose:** [Docker Desktop](https://www.docker.com/products/docker-desktop/) is recommended.
3. **dfu-util & Platform-Specific Setup:** For flashing firmware. The setup differs by OS.

### macOS

Use [Homebrew](https://brew.sh/):

```bash
brew install dfu-util
```

### Linux

1. Install `dfu-util`:

```bash
sudo apt-get update && sudo apt-get install dfu-util
```

2. Set udev rules:

```bash
echo 'SUBSYSTEM=="usb", ATTRS{idVendor}=="0483", ATTRS{idProduct}=="df11", MODE="0666"' | sudo tee /etc/udev/rules.d/99-dfu.rules
```

3. Reload rules:

```bash
sudo udevadm control --reload-rules
```

### Windows

You must run the Zadig driver replacement twice, due to the device entering a second-stage DFU mode after the first DFU action:

1. Place `dfu-util` binary in your PATH.
2. Download and use **[Zadig Tool](https://zadig.akeo.ie/)**.

**Driver Installation Steps:**

* Connect your device in DFU mode initially.
* Run Zadig, check `Options` → `List All Devices`.
* Select the `STM32 BOOTLOADER` device, choose `WinUSB`, and click "Replace Driver".
* Use `dfu-util` to flash the firmware; this action will trigger the device to enter a second-stage DFU mode.
* Re-run Zadig and repeat the process, selecting the new device entry again and installing the `WinUSB` driver.

> **Detailed guide:** Refer to the **[uCritAir Web DFU page](https://ucritair.github.io/ucritair-webdfu/)** for detailed instructions.

---

## Getting Started

### Step 1: Clone the Repository

```bash
git clone https://github.com/ucritair/ucritair-firmware.git
cd ucritair-firmware
```

### Step 2: Build the Firmware

Launch the development container:

```bash
docker compose run --rm dev
```

Inside the container, run:

```bash
./utils/build.sh --embedded
```

Compiled firmware will be available at:

* Embedded files: `ucritair-firmware/zephyrapp/build`
* Desktop files: `ucritair-firmware/zephyrapp/game/build`

### Step 3: Flashing the Firmware

From the host machine, exit the container:

```bash
exit
```

Connect device in DFU mode and flash firmware:

```bash
dfu-util --download ucritair-firmware/zephyrapp/build/zephyr/zephyr.signed.bin
```

Your device now runs the latest firmware.

---

## Day-to-Day Development Workflow

* **Edit Code:** Modify source code on host machine.
* **Build in Docker:** From project root, run:

```bash
docker compose run --rm dev
```

Then inside the container:

```bash
./utils/build.sh --embedded
```

* **Flash from Host:** Exit the container and run `dfu-util` as above.
