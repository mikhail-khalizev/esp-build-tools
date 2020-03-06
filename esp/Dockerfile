# --- Initialize. ---

FROM ubuntu as base

ENV HOME=/root
WORKDIR ${HOME}
SHELL ["/bin/bash", "-c"]


# --- Install xtensa gcc. ---

FROM base as tools-xtensa-gcc

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

ENV PATH="${HOME}/esp/xtensa-esp32-elf/bin:${PATH}"

RUN    mkdir ~/esp \
    && cd ~/esp \
    && git clone -b $TOOLCHAIN_BRANCH --single-branch --depth 1 https://github.com/espressif/crosstool-NG.git \
    \
    && cd crosstool-NG \
    && ./bootstrap \
    && ./configure --enable-local \
    && make -j8 \
    \
    && ./ct-ng xtensa-esp32-elf \
    && sed -i 's: *-fno-rtti *::g' ./.config \
    && echo "CT_ALLOW_BUILD_AS_ROOT=y" >> .config \
    && echo "CT_ALLOW_BUILD_AS_ROOT_SURE=y" >> .config \
    && echo "CT_PREFIX_DIR=${HOME}/esp/xtensa-esp32-elf" >> .config \
    && cp ~/my-docker/*.patch ./patches/gcc/5.2.0/ \
    \
    && mkdir ~/src \
    && ./ct-ng build || (tail -n 100 build.log; exit 1) \
    \
    && cd ~ \
    && rm -rf ~/src ~/my-docker ~/esp/crosstool-NG ~/esp/xtensa-esp32-elf/build.log.bz2 \
    \
    && xtensa-esp32-elf-gcc -v


# --- Install esp-idf. ---

# git branch version from https://docs.espressif.com/projects/esp-idf/en/stable/get-started/index.html#get-esp-idf

FROM base as tools

RUN    apt-get update \
    && apt-get install -y \
            gcc git wget make libncurses-dev flex bison gperf \
            python python-pip python-setuptools python-serial python-cryptography python-future python-pyparsing \
    && rm -rf /var/lib/apt/lists/*

COPY --from=tools-xtensa-gcc /root/esp/xtensa-esp32-elf/ /root/esp/xtensa-esp32-elf/
ENV PATH="${HOME}/esp/xtensa-esp32-elf/bin:${PATH}"

ARG IDF_BRANCH=release/v3.3

ENV IDF_PATH ${HOME}/esp/esp-idf
ENV PATH="${HOME}/esp/esp-idf/components/esptool_py/esptool:${HOME}/esp/esp-idf/components/espcoredump:${HOME}/esp/esp-idf/components/partition_table:${HOME}/esp/esp-idf/tools:${PATH}"

# Install python requirements in separate docker image slot.
RUN    cd ~/esp \
    && git clone -b $IDF_BRANCH --single-branch --depth 1 https://github.com/espressif/esp-idf.git \
    && cd esp-idf \
    \
    `# Add modules from '$IDF_PATH/tools/tiny-test-fw/requirements.txt'.` \
    && printf "pyyaml\njunit_xml\nnetifaces\nmatplotlib\n" >> $IDF_PATH/requirements.txt \
    && python -m pip install --user -r $IDF_PATH/requirements.txt \
    \
    && cd ~ \
    && rm -rf ~/esp/esp-idf

# Install esp-idf.
RUN    set +x \
    && cd ~/esp \
    && git clone -b $IDF_BRANCH --single-branch --depth 1000 --recursive https://github.com/espressif/esp-idf.git \
    && cd esp-idf \
    && git describe --always --tags > version.txt \
    && git describe --always --tags --abbrev=0 > tag.txt \
    \
    && printf "\nSUPPORTED_TOOLCHAIN_COMMIT_DESC = `xtensa-esp32-elf-gcc --version | sed -E -n 's|.*\(crosstool-NG (.*)\).*|\1|gp'`\n" >> $IDF_PATH/tools/toolchain_versions.mk \
    && printf  "SUPPORTED_TOOLCHAIN_GCC_VERSIONS = `xtensa-esp32-elf-gcc -dumpversion`\n" >> $IDF_PATH/tools/toolchain_versions.mk \
    && sed -i 's: *-fno-rtti *: :g' $IDF_PATH/make/project.mk \
    && sed -i 's:# define ASIO_NO_TYPEID:// # define ASIO_NO_TYPEID:g' $IDF_PATH/components/asio/port/include/esp_asio_config.h \
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
