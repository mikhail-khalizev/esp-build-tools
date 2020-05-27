# --- Initialize. ---

FROM ubuntu:bionic as base

ENV HOME=/root
WORKDIR ${HOME}
SHELL ["/bin/bash", "-c"]


# --- Install xtensa gcc. ---

FROM base as tools-xtensa-gcc

RUN    export DEBIAN_FRONTEND=noninteractive \
    && apt-get update \
    && apt-get install -y \
            build-essential gawk bison texinfo autoconf automake unzip help2man flex file libtool man libtool-bin libncurses5 libncurses5-dev \
            git wget libncurses-dev flex bison python-click python-pyelftools cmake ninja-build ccache \
            gawk grep gettext python python-dev automake bison flex texinfo libtool libtool-bin \
            gperf \
    && rm -rf /var/lib/apt/lists/*

# git branch version from https://docs.espressif.com/projects/esp-idf/en/stable/get-started/linux-setup-scratch.html#compile-the-toolchain-from-source
ARG TOOLCHAIN_BRANCH=esp-2019r2

COPY get-gcc-patch-dir.sh *.patch  my-docker/
COPY pkg/* ${HOME}/src/

ENV PATH="/xtensa-esp32-elf/bin:${PATH}"

RUN    set -euo pipefail \
    && git clone -b $TOOLCHAIN_BRANCH --single-branch --depth 1 --recursive https://github.com/espressif/crosstool-NG.git /crosstool-NG \
    && cd /crosstool-NG \
    && ./bootstrap \
    && ./configure --enable-local \
    && make -j8 \
    \
    && cd /crosstool-NG \
    && ./ct-ng xtensa-esp32-elf \
    && echo "CT_ALLOW_BUILD_AS_ROOT=y" >> .config \
    && echo "CT_ALLOW_BUILD_AS_ROOT_SURE=y" >> .config \
    && echo "CT_PREFIX_DIR=/xtensa-esp32-elf" >> .config \
    && cp ~/my-docker/*.patch `~/my-docker/get-gcc-patch-dir.sh` \
    \
    && ./ct-ng build || (tail -n 100 build.log; exit 1) \
    \
    && cd ~ \
    && rm -rf ~/src ~/my-docker /crosstool-NG /xtensa-esp32-elf/build.log.bz2 \
    \
    && xtensa-esp32-elf-gcc -v


# --- Install esp-idf. ---

# git branch version from https://docs.espressif.com/projects/esp-idf/en/stable/get-started/index.html#get-esp-idf

FROM base as tools

RUN    export DEBIAN_FRONTEND=noninteractive \
    && apt-get update \
    && apt-get install -y \
            gcc git wget make libncurses-dev flex bison gperf \
            python python-pip python-setuptools python-serial python-cryptography python-future python-pyparsing \
    && rm -rf /var/lib/apt/lists/*

COPY --from=tools-xtensa-gcc /xtensa-esp32-elf/ /xtensa-esp32-elf/
ENV PATH="/xtensa-esp32-elf/bin:${PATH}"

ARG IDF_BRANCH=release/v4.0

ENV IDF_PATH /esp-idf
ENV PATH="${IDF_PATH}/components/esptool_py/esptool:${IDF_PATH}/components/espcoredump:${IDF_PATH}/components/partition_table:${IDF_PATH}/tools:${PATH}"

# Install python requirements in separate docker image slot.
RUN    cd / \
    && git clone -b $IDF_BRANCH --single-branch --depth 1 https://github.com/espressif/esp-idf.git \
    && cd $IDF_PATH \
    \
    `# Add modules from '$IDF_PATH/tools/tiny-test-fw/requirements.txt'.` \
    && printf "pyyaml\njunit_xml\nnetifaces\nmatplotlib\n" >> $IDF_PATH/requirements.txt \
    && python -m pip install --user -r $IDF_PATH/requirements.txt \
    \
    && cd ~ \
    && rm -rf $IDF_PATH

# Install esp-idf.
RUN    set +x \
    && cd / \
    && git clone -b $IDF_BRANCH --single-branch --depth 1000 --recursive https://github.com/espressif/esp-idf.git \
    && cd $IDF_PATH \
    && git describe --always --tags > version.txt \
    && git describe --always --tags --abbrev=0 > tag.txt \
    && ln -s $IDF_PATH ~/esp/esp-idf \
    \
    \
    `# Check that example project builds successfully.` \
    && cd ~/esp \
    && cd $IDF_PATH/examples/get-started/hello_world \
    && echo "CONFIG_CXX_EXCEPTIONS=y" >> sdkconfig \
    && echo "CONFIG_CXX_EXCEPTIONS_EMG_POOL_SIZE=128" >> sdkconfig \
    && make -j8 defconfig \
    && make -j8 \
    \
    \
    `# Reduce size` \
    && cd ~/esp \
    && find \( -name examples -o -name doc -o -name docs -o -name testdata -o -name '.git*' -o -name '.*.yml' \) -exec rm -rf {} + \
    && cd $IDF_PATH/components \
    && find \( -name tests \) -exec rm -rf {} + \
    && rm -rf ~/my-docker
