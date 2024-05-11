

get aravis device name using 
```bash
arv-tool-0.8
```

This is the camera-name field specified in `server/src/server.c`
To get v4l device (ie USB webcam)

```bash
v4l2-ctl --list-devices
```

start server with
`make server`

start player with
```
xhost +local:docker
make player
```

