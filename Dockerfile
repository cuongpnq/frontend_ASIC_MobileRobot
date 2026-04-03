FROM ubuntu:20.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    ninja-build \
    qtbase5-dev \
    qtdeclarative5-dev \
    qtwayland5 \
    libqt5waylandclient5 \
    qml-module-qtquick2 \
    qml-module-qtquick-controls2 \
    qml-module-qtquick-layouts \
    qml-module-qtgraphicaleffects \
    libgl1-mesa-dev \
    libxkbcommon-x11-0 \
    libxcb-xinerama0 \
    libwayland-client0 \
    libwayland-egl1 \
    libqt5virtualkeyboard5-dev \
    qml-module-qtquick-virtualkeyboard \
    qtvirtualkeyboard-plugin \
    qml-module-qt-labs-folderlistmodel \
    qml-module-qt-labs-settings \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

CMD ["/bin/bash"]