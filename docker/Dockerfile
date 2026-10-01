FROM docker.io/library/debian:stable-slim

# Gather Unix dependencies
RUN dpkg --add-architecture i386
RUN apt-get update -y
RUN apt-get install git unzip wget wine wine32 wine64 -y

# Set up the wineprefix
RUN wine wineboot

# Gather and set up build dependencies (MSVC 4.1, MSVC 2.2 for MECH2.EXE, and CMake for Windows)
RUN git clone https://github.com/madebr/msvc410 ~/.wine/drive_c/msvc
# MASM 6.11 (ML) for the original's hand-written assembly objects, pinned to a known commit
RUN git clone https://github.com/shengyanli1982/MASM611 ~/.wine/drive_c/masm \
 && git -C ~/.wine/drive_c/masm checkout 36b14330ade0f3347d7bc7ba373304f279c81221
RUN git clone https://github.com/archaic-msvc/msvc220 ~/.wine/drive_c/msvc22 \
 && git -C ~/.wine/drive_c/msvc22 checkout c0fff2aecaa88576c685de79173be4ead4dbe42c
RUN wget https://github.com/Kitware/CMake/releases/download/v3.26.6/cmake-3.26.6-windows-i386.zip
RUN unzip cmake-3.26.6-windows-i386.zip -d ~/.wine/drive_c
RUN mv ~/.wine/drive_c/cmake-3.26.6-windows-i386 ~/.wine/drive_c/cmake
RUN rm cmake-3.26.6-windows-i386.zip

# Set up entrypoint script to perform the build
COPY entrypoint.sh entrypoint.sh
RUN chmod +x entrypoint.sh
ENTRYPOINT [ "/bin/bash", "./entrypoint.sh" ]
