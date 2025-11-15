FROM debian:stable-slim

# Instalar dependências para compilar DPP
RUN apt-get update && apt-get install -y \
    g++ \
    cmake \
    git \
    make \
    libssl-dev \
    libsodium-dev \
    libfmt-dev \
    libcurl4-openssl-dev \
    zlib1g-dev \
    libopus-dev \
    && rm -rf /var/lib/apt/lists/*

# Clonar e compilar DPP
RUN git clone --recursive https://github.com/brainboxdotcc/DPP.git /opt/dpp \
    && mkdir /opt/dpp/build \
    && cd /opt/dpp/build \
    && cmake .. -DCMAKE_BUILD_TYPE=Release \
    && make -j4 \
    && make install

# Copiar código do bot
WORKDIR /app
COPY . .

# Compilar o bot
RUN g++ -std=c++20 main.cpp -o bot -ldpp -lpthread

# Executar o bot
CMD ["./bot"]
