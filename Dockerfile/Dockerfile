#
# Builds a cross-platform (x86_64 and aarch64) development environment
# for a specific Zephyr-based firmware project.
#
# It pins the Zephyr version to a specific commit, ensures all modules are
# version-compatible, applies project-specific patches, and clones
# the target firmware repository.
#

# Use the official Ubuntu 24.04 image. Docker will automatically select
# the correct base image for the build architecture (amd64 or arm64).
FROM ubuntu:24.04

# Set non-interactive frontend for package installers.
ENV DEBIAN_FRONTEND=noninteractive

# This build argument is automatically populated by Docker with the target
# architecture (e.g., "amd64", "arm64"), enabling multi-platform builds.
ARG TARGETARCH

#
# Layer 1: System Dependencies
#
RUN apt-get update && apt-get install --no-install-recommends -y \
    git cmake ninja-build gperf ccache make dfu-util device-tree-compiler \
    python3-dev python3-pip python3-setuptools python3-tk python3-venv python3-wheel \
    wget xz-utils file gcc g++ libsdl2-dev libarchive-dev libglew-dev libglfw3-dev\
    zlib1g-dev gawk bison flex libncurses5-dev gettext libpng-dev ffmpeg\
    && if [ "${TARGETARCH}" = "amd64" ]; then \
        echo "Installing x86_64 specific packages (multilib)..." && \
        apt-get install -y --no-install-recommends gcc-multilib g++-multilib; \
    fi \
    && rm -rf /var/lib/apt/lists/*

#
# Layer 2: Python Virtual Environment
#
WORKDIR /zephyrproject
RUN python3 -m venv venv

#
# Layer 3: Core Zephyr Source Setup
#
RUN . venv/bin/activate && \
    pip install --upgrade pip && \
    pip install west && \
    pip install pygame &&\
    pip install pillow &&\
    pip install PyOpenGL &&\
    pip install glfw &&\
    pip install imgui-bundle &&\
    pip install playsound3 &&\
    pip install numpy &&\
    git clone https://github.com/zephyrproject-rtos/zephyr.git zephyr && \
    cd zephyr && \
    git checkout 03959a20f7834ffe50a51df2b2e45ef87d1b610b && \
    west init -l . && \
    west update && \
    west zephyr-export && \
    pip install -r scripts/requirements.txt

#
# Layer 4: Zephyr SDK Installation
#
RUN \
    set -e && \
    case ${TARGETARCH} in \
        "amd64") SDK_ARCH="x86_64" ;; \
        "arm64") SDK_ARCH="aarch64" ;; \
        *) echo "Unsupported architecture: ${TARGETARCH}" ; exit 1 ;; \
    esac && \
    echo "Downloading SDK for ${SDK_ARCH}..." && \
    wget "https://github.com/zephyrproject-rtos/sdk-ng/releases/download/v0.16.8/zephyr-sdk-0.16.8_linux-${SDK_ARCH}.tar.xz" && \
    tar xvf "zephyr-sdk-0.16.8_linux-${SDK_ARCH}.tar.xz" && \
    rm "zephyr-sdk-0.16.8_linux-${SDK_ARCH}.tar.xz" && \
    cd zephyr-sdk-0.16.8 && \
    ./setup.sh -t all -h -c

#
# Layer 5: Project-Specific Zephyr Patching
#
RUN . venv/bin/activate && \
    echo "Downloading and applying zephyr.patch..." && \
    wget -O zephyr.patch https://raw.githubusercontent.com/ucritair/ucritair-firmware/main/zephyr.patch && \
    cd zephyr && \
    git apply ../zephyr.patch

#
# Layer 6: Clone Firmware Repository
#
RUN git clone https://github.com/ucritair/ucritair-firmware.git /ucritair-firmware

#
# Layer 7: Final Environment Configuration
#
ENV ZEPHYR_TOOLCHAIN_VARIANT=zephyr
ENV ZEPHYR_SDK_INSTALL_DIR=/zephyrproject/zephyr-sdk-0.16.8
ENV ZEPHYR_BASE=/zephyrproject/zephyr

#
# Layer 8: Container Entrypoint
#
COPY entrypoint.sh /usr/local/bin/entrypoint.sh
RUN chmod +x /usr/local/bin/entrypoint.sh
ENTRYPOINT ["/usr/local/bin/entrypoint.sh"]

#
# Layer 9: Final Working Directory and Default Command
#
WORKDIR /ucritair-firmware/zephyrapp/game
CMD ["/bin/bash"]