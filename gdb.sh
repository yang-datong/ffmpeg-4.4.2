#!/bin/bash

#设置动态库执行路径，在动态库存在于用户目录时
export LD_LIBRARY_PATH=libavcodec:libavdevice:libavfilter:libavformat:libavresample:libavutil:libpostproc:libswresample:libswscale:../x265_3.6/build/linux:../x264-master/build/lib

#程序执行需要的参数

args='-i ./demo_10_frames.h265 output.yuv -y'
#args='-hwaccel_device /dev/dri/renderD128 -i demo.mp4 output.yuv -y' #没进avctx->codec->receive_frame()

#源代码路径(Glibc路径,Option)
ldd_version=$(ldd --version | grep ldd | awk '{print $5}')

source_dir="libavcodec:libavdevice:libavfilter:libavformat:libavresample:libavutil:libpostproc:libswresample:libswscale"

#最后传入脚步参数，配合这Vim-GDB使用
gdb ./ffmpeg_g -ex "set args ${args}" -ex "directory ${source_dir}" -ex "b main" -ex "r" "$@"
