#!/bin/bash

#设置动态库执行路径，在动态库存在于用户目录时
export LD_LIBRARY_PATH=libavcodec:libavdevice:libavfilter:libavformat:libavresample:libavutil:libpostproc:libswresample:libswscale:../x265_3.6/build/linux:../x264-master/build/lib

#程序执行需要的参数

args='-i ./demo_10_frames_cut_1_frames.h265 output.yuv -y'
#args='-hwaccel_device /dev/dri/renderD128 -i demo.mp4 output.yuv -y' #没进avctx->codec->receive_frame()

./ffmpeg_g ${args}
