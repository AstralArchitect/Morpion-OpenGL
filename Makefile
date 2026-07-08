# Target to compile the project
all:
	meson compile -C build/

# Target to run the compiled binary
run: all
	./build/main

# Target to set up the build directory
setup:
	meson setup build

# Target to clean the build directory
clean:
ifeq ($(OS),Windows_NT)
	@if exist build (rmdir build /S /Q)
else
	@if [ -d build ]; then rm -rf build; fi
endif

resetup: clean setup

build_release:
	meson setup release -Dbuildtype=release --reconfigure
	meson compile -C release
