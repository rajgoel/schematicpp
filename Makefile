# Delegating wrapper. CMake is the build system; this file only shortens the commands that are typed
# often. Everything here is reachable with cmake --build directly.
#
# Configure once with:  cmake --preset release

JOBS := $(shell nproc)

.PHONY: all
all:
	cmake --build build/release --parallel $(JOBS)
