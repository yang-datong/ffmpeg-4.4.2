#!/bin/bash

export LD_LIBRARY_PATH=libavcodec:libavdevice:libavfilter:libavformat:libavresample:libavutil:libpostproc:libswresample:libswscale:../x265_3.6/build/linux_amd64/lib/:../x264-master/build/lib/

export DYLD_LIBRARY_PATH=libavcodec:libavdevice:libavfilter:libavformat:libavresample:libavutil:libpostproc:libswresample:libswscale:../x265_3.6/build/linux_amd64/lib/:../x264-master/build/lib/

gdb ffmpeg_g \
	-ex "directory libavcodec:libavdevice:libavfilter:libavformat:libavresample:libavutil:libpostproc:libswresample:libswscale" \
	-ex "set args -i demo.mp4 demo.h264 -y" \
	-ex "b main" \
	-ex "r"
