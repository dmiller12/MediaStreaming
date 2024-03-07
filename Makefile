
OS := $(shell uname)

ifeq ($(OS), Linux)
    DOCKER_DISPLAY = --volume="/tmp/.X11-unix:/tmp/.X11-unix:rw" \
		--env DISPLAY=${DISPLAY} --device /dev/dri:/dev/dri
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

.PHONY: term
term: build
	docker run ${DOCKER_ARGS} --rm --net=host --privileged -it media_streamer /bin/bash

.PHONY: player
player: build
	docker run ${DOCKER_ARGS} --rm --net=host --privileged  media_streamer build/player/player

.PHONY: server
server: build
	docker run ${DOCKER_ARGS} --rm --net=host --privileged  media_streamer build/server/server
