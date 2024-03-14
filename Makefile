
OS := $(shell uname)

ifeq ($(OS), Linux)
    DOCKER_DISPLAY = --volume="/tmp/.X11-unix:/tmp/.X11-unix:rw" \
		--env DISPLAY=${DISPLAY}
else ifeq ($(OS), Darwin)
    DOCKER_DISPLAY = --volume="/tmp/.X11-unix:/tmp/.X11-unix:rw" \
		--env DISPLAY=host.docker.internal:0
endif

DOCKER_VOLUMES = 
DOCKER_ENV_VARS = 

DOCKER_ARGS = ${DOCKER_VOLUMES} ${DOCKER_ENV_VARS} ${DOCKER_DISPLAY}

.PHONY: build
build: Dockerfile
	docker build -t media_streamer .

.PHONY: player
player: build
	docker run ${DOCKER_ARGS} --rm --net=host --privileged -e GST_DEBUG=3 media_streamer build/player/player -h "192.168.1.11"

.PHONY: player-test
player-test: build
	docker run ${DOCKER_ARGS} --rm --net=host --privileged -e GTK_DEBUG=interactive  media_streamer build/player/player

.PHONY: server
server: build
	docker run ${DOCKER_ARGS} --rm --net=host --privileged -v /dev:/dev media_streamer build/server/server 

.PHONY: server-test
server-test: build
	docker run ${DOCKER_ARGS} --rm --net=host --privileged  media_streamer build/server/server test

.PHONY: dev
dev: build
	docker run $(DOCKER_AGS) -it --rm --net=host --privileged -v $(PWD)/server:/app/server -v $(PWD)/player:/app/player -v $(PWD)/.clang-format:/app/.clang-format --name MediaStreaming media_streamer /bin/bash
