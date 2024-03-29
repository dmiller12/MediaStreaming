FROM ubuntu:jammy-20240227

ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && \
	apt-get install -y \
	libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev libgstreamer-plugins-bad1.0-dev gstreamer1.0-plugins-base gstreamer1.0-plugins-good gstreamer1.0-plugins-bad gstreamer1.0-plugins-ugly gstreamer1.0-libav gstreamer1.0-tools gstreamer1.0-x gstreamer1.0-alsa gstreamer1.0-gl gstreamer1.0-gtk3 gstreamer1.0-qt5 gstreamer1.0-pulseaudio gstreamer1.0-rtsp libgstrtspserver-1.0-dev libgtk-3-dev gstreamer1.0-vaapi libaravis-0.8-0 libcanberra-gtk-module libcanberra-gtk3-module \
	cmake build-essential

RUN ln -snf /usr/share/zoneinfo/America/Edmonton /etc/localtime && echo America/Edmonton > /etc/timezone

# Dev specific stuff
RUN apt-get install -y clangd-12 && update-alternatives --install /usr/bin/clangd clangd /usr/bin/clangd-12 100

WORKDIR /app

COPY player player
COPY server server
COPY CMakeLists.txt CMakeLists.txt

RUN mkdir build && \
	cd build && \
	cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=1 .. && \
	make

RUN ln -s  build/compile_commands.json compile_commands.jason

CMD ["bash"]
