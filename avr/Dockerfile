# --- Initialize. ---

FROM ubuntu as base

ENV HOME=/root
WORKDIR ${HOME}
SHELL ["/bin/bash", "-c"]


FROM base as tools-gcc

RUN    apt-get update \
    && apt-get install -y \
            gcc g++ gperf bison flex texinfo help2man make libncurses5-dev \
            python3-dev autoconf automake libtool libtool-bin gawk wget bzip2 xz-utils unzip \
            patch libstdc++6 rsync \
    && rm -rf /var/lib/apt/lists/*

ENV PATH="${HOME}/avr/gcc/bin:${PATH}"
    
# Link from http://crosstool-ng.github.io/download/
ARG CROSSTOOL_NG_VERSION=1.24.0
    
RUN	   wget https://github.com/crosstool-ng/crosstool-ng/archive/crosstool-ng-$CROSSTOOL_NG_VERSION.tar.gz \
	&& tar xf crosstool-ng-*.tar* \
    && rm crosstool-ng-*.tar* \
    && mv crosstool-ng* crosstool-ng \
    \
	&& cd crosstool-ng \
    && ./bootstrap \
	&& ./configure --enable-local \
    && make -j8 \
    \
    && ./ct-ng avr \
    && echo "CT_ALLOW_BUILD_AS_ROOT=y" >> .config \
    && echo "CT_ALLOW_BUILD_AS_ROOT_SURE=y" >> .config \
    && echo "CT_PREFIX_DIR=${HOME}/avr/gcc" >> .config \
    \
    && mkdir ~/src \
    && ./ct-ng build || (tail -n 100 build.log; exit 1) \
    \
    && cd ~ \
    && rm -rf src crosstool-ng avr/gcc/build.log.bz2 \
    \
    && avr-gcc -v


FROM base as tools

COPY --from=tools-gcc /root/avr/gcc/ /root/avr/gcc/
ENV PATH="${HOME}/avr/gcc/bin:${PATH}"

