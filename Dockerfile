FROM ubuntu:20.04

ENV DEBIAN_FRONTEND=noninteractive
ENV TZ=UTC
ENV PATH="/usr/local/sbin:/usr/local/bin:$PATH"

RUN apt-get update && apt-get install -y \
    build-essential \
    git \
    autotools-dev \
    autoconf \
    automake \
    libtool \
    gperf \
    bison \
    flex \
    libgmp-dev \
    libssl-dev \
    pkg-config \
    python3 \
    iproute2 \
    iputils-ping \
    gdb \
    && rm -rf /var/lib/apt/lists/*

# Build strongSwan from source with VICI enabled
RUN git clone https://github.com/strongswan/strongswan.git /tmp/strongswan && \
    cd /tmp/strongswan && \
    ./autogen.sh && \
    ./configure --enable-vici --enable-swanctl --disable-starter --sysconfdir=/etc --enable-shared && \
    make && \
    make install && \
    ldconfig && \
    cp src/libcharon/plugins/vici/libvici.h /usr/local/include/ && \
    cp src/libcharon/plugins/vici/.libs/libvici.a /usr/local/lib/ 2>/dev/null || true && \
    rm -rf /tmp/strongswan

COPY entrypoint.sh /entrypoint.sh
RUN chmod +x /entrypoint.sh

WORKDIR /app
COPY vici_src/ /app/vici_src/

ENTRYPOINT ["/entrypoint.sh"]