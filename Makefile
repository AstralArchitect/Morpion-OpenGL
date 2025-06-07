# Target to compile the project
all:
	meson compile -C build/

# Target to run the compiled binary
run: all
ifeq ($(OS),Windows_NT)
	.\addToPathAndRun.bat
else
	./build/main
endif

# Target to set up the build directory
setupClang:
	meson setup build --native-file=native-clang.txt
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
resetupGcc: clean setupGcc