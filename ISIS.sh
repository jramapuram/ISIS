#!/bin/bash
DBGDIR="Debug"
RLSDIR="Release"

if [ -d "$DBGDIR" ]; then
	echo "Detected Debug Dir,Setting v4L Preload"
	cd "$DBGDIR"
	export LD_PRELOAD=/usr/lib/x86_64-linux-gnu/libv4l/v4l1compat.so ISIS
	./ISIS
elif [-d "$RLSDIR" ]; then
	echo "Detected Release Dir,Setting v4L Preload"
	cd "$RLSDIR"
	export LD_PRELOAD=/usr/lib/x86_64-linux-gnu/libv4l/v4l1compat.so ISIS
	./ISIS
else
	echo "Error,no release or debug directories detected!"
fi
