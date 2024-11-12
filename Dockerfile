FROM nvidia/cuda:12.4.0-devel-ubuntu22.04
ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    gdb \
    git \
    valgrind \
    && apt-get clean

WORKDIR /usr/src/app

COPY . .

CMD ["bash"]