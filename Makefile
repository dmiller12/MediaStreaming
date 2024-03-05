
build: Dockerfile
	docker build -t media_streamer .

run: build
	docker run -it media_streamer
