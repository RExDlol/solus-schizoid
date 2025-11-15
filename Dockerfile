FROM debian:stable-slim

# Instalar dependências do DPP e C++
RUN apt-get update && apt-get install -y \
    g++ cmake git libssl-dev libfmt-dev libopus-dev libsodium-dev \
    && rm -rf /var/lib/apt/lists/*

# Baixar DPP
RUN git clone https://github.com/brainboxdotcc/DPP.git /opt/dpp \
    && mkdir /opt/dpp/build \
    && cd /opt/dpp/build \
    && cmake .. \
    && make -j4 \
    && make install

# Copiar o código do bot
WORKDIR /app
COPY . .

# Compilar o bot
RUN g++ -std=c++20 main.cpp -o bot -ldpp

# Rodar o bot
CMD ["./bot"]
