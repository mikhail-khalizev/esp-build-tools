# --- Initialize. ---

FROM ubuntu as tools-base

ENV HOME=/root
WORKDIR ${HOME}
SHELL ["/bin/bash", "-c"]


# --- Install xtensa gcc. ---

FROM tools-base as tools-xtensa-gcc

RUN    apt-get update \
    && apt-get install -y \
            build-essential gawk bison texinfo autoconf automake unzip help2man flex file libtool man libtool-bin libncurses5 libncurses5-dev \
            git wget libncurses-dev flex bison python-click python-pyelftools cmake ninja-build ccache \
            gawk grep gettext python python-dev automake bison flex texinfo libtool libtool-bin \
            gperf \
    && rm -rf /var/lib/apt/lists/*

# git branch version from https://docs.espressif.com/projects/esp-idf/en/stable/get-started/linux-setup-scratch.html#compile-the-toolchain-from-source
ARG TOOLCHAIN_BRANCH=xtensa-1.22.x

COPY *.patch  my-docker/

# NOTE. Когда esp-idf официально перейдет на GCC 8.2 стоит изменить команду копирования patch на 'cp ~/my-docker/*.patch `./get-gcc-patch-dir.sh`'.
# COPY get-gcc-patch-dir.sh  my-docker/

RUN    mkdir ~/esp \
    && cd ~/esp \
    && git clone -b $TOOLCHAIN_BRANCH --single-branch --depth 1 https://github.com/espressif/crosstool-NG.git \
    \
    && cd crosstool-NG \
    && ./bootstrap && ./configure --enable-local && make -j8 \
    \
    && sed -i 's: *-fno-rtti *::g' ./samples/xtensa-esp32-elf/crosstool.config \
    && ./ct-ng xtensa-esp32-elf \
    && echo "CT_ALLOW_BUILD_AS_ROOT=y" >> .config \
    && echo "CT_ALLOW_BUILD_AS_ROOT_SURE=y" >> .config \
    && cp ~/my-docker/*.patch ./patches/gcc/5.2.0/ \
    \
    && ./ct-ng build || (tail -n 100 build.log; exit 1) \
    && chmod -R u+w builds/xtensa-esp32-elf \
    && mv ./builds/xtensa-esp32-elf ~/esp/ \
    && cd ~ \
    && rm -rf ~/esp/crosstool-NG ~/my-docker ~/esp/xtensa-esp32-elf/build.log.bz2

ENV PATH="${HOME}/esp/xtensa-esp32-elf/bin:${PATH}"


# --- Install esp-idf build apps. ---

FROM tools-base as tools-pre

RUN    apt-get update \
    && apt-get install -y \
            gcc git wget make libncurses-dev flex bison gperf \
            python python-pip python-setuptools python-serial python-cryptography python-future python-pyparsing \
    && rm -rf /var/lib/apt/lists/*

COPY --from=tools-xtensa-gcc /root/esp/xtensa-esp32-elf/ /root/esp/xtensa-esp32-elf/
ENV PATH="${HOME}/esp/xtensa-esp32-elf/bin:${PATH}"


# --- Install esp-idf, arduino-esp32, esp-mdf. ---
# TODO Add esp-adf?

# git branch version from https://docs.espressif.com/projects/esp-idf/en/stable/get-started/index.html#get-esp-idf

ARG IDF_BRANCH=release/v3.3
ARG ARDUINO_BRANCH=idf-release/v3.3

ENV IDF_PATH ${HOME}/esp/esp-idf
ENV MDF_PATH ${HOME}/esp/esp-mdf
ENV PATH="${HOME}/esp/esp-idf/components/esptool_py/esptool:${HOME}/esp/esp-idf/components/espcoredump:${HOME}/esp/esp-idf/components/partition_table:${HOME}/esp/esp-idf/tools:${PATH}"

RUN    cd ~/esp \
    && git clone -b $IDF_BRANCH --single-branch --depth 1 --recursive https://github.com/espressif/esp-idf.git \
    && cd esp-idf \
    && git describe --always --tags --dirty > version.txt \
    && rm -rf .git .gitmodules \
    \
    && printf "\nSUPPORTED_TOOLCHAIN_COMMIT_DESC = `xtensa-esp32-elf-gcc --version | sed -E -n 's|.*\(crosstool-NG (.*)\).*|\1|gp'`\n" >> $IDF_PATH/tools/toolchain_versions.mk \
    && printf  "SUPPORTED_TOOLCHAIN_GCC_VERSIONS = `xtensa-esp32-elf-gcc -dumpversion`\n" >> $IDF_PATH/tools/toolchain_versions.mk \
    && sed -i 's: *-fno-rtti *: :g' $IDF_PATH/make/project.mk \
    && sed -i 's:static bool abort_called;:bool abort_called;:g' $IDF_PATH/components/esp32/panic.c `# Used in 'xt_exception_handler_isr' method.` \
    \
    `# Add modules from '$IDF_PATH/tools/tiny-test-fw/requirements.txt'.` \
    && printf "pyyaml\njunit_xml\nnetifaces\nmatplotlib\n" >> $IDF_PATH/requirements.txt \
    && python -m pip install --user -r $IDF_PATH/requirements.txt \
    \
    \
    && cd $IDF_PATH/components \
    && git clone -b $ARDUINO_BRANCH --single-branch --depth 1 --recursive https://github.com/espressif/arduino-esp32 arduino \
    && cd arduino \
    && git describe --always --tags --dirty > version.txt \
    && rm -rf .git .gitmodules \
    \
    `# Fix 'iothubtransport_mqtt_common.c' compilation error.` \
    && printf "\nCFLAGS += -Wno-maybe-uninitialized\n" >> $IDF_PATH/components/arduino/component.mk \
    `# && sed -i 's: *-fno-rtti *: :g' $IDF_PATH/components/arduino/component.mk` \
    \
    \
    && cd ~/esp \
    && git clone --single-branch --depth 1 https://github.com/espressif/esp-mdf.git \
    && cd esp-mdf \
    && git describe --always --tags --dirty > version.txt \
    && rm -rf .git .gitmodules \
    \
    && mv $IDF_PATH/make/project.mk $IDF_PATH/make/project-orig.mk

COPY project.mk  esp/esp-idf/make/


# --- Check that example project builds successfully. ---

FROM tools-pre as tools-check

RUN    cd ~/esp \
    && cp -r $IDF_PATH/examples/get-started/hello_world . \
    && cd ~/esp/hello_world \
    && cp $IDF_PATH/components/arduino/tools/sdk/sdkconfig . \
    && make -j8 defconfig \
    && make -j8

# --- Result target. ---

FROM tools-pre as tools
