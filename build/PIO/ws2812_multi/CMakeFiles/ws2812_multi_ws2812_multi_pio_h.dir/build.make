# CMAKE generated file: DO NOT EDIT!
# Generated by "NMake Makefiles" Generator, CMake Version 3.20

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE
NULL=nul
!ENDIF
SHELL = cmd.exe

# The CMake executable.
CMAKE_COMMAND = "C:\Program Files\CMake\bin\cmake.exe"

# The command to remove a file.
RM = "C:\Program Files\CMake\bin\cmake.exe" -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = C:\Users\Dan\Documents\Programming\Pico\mypico

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = C:\Users\Dan\Documents\Programming\Pico\mypico\build

# Utility rule file for ws2812_multi_ws2812_multi_pio_h.

# Include any custom commands dependencies for this target.
include PIO\ws2812_multi\CMakeFiles\ws2812_multi_ws2812_multi_pio_h.dir\compiler_depend.make

# Include the progress variables for this target.
include PIO\ws2812_multi\CMakeFiles\ws2812_multi_ws2812_multi_pio_h.dir\progress.make

PIO\ws2812_multi\CMakeFiles\ws2812_multi_ws2812_multi_pio_h: PIO\ws2812_multi\ws2812_multi.pio.h
	cd C:\Users\Dan\Documents\Programming\Pico\mypico\build\PIO\ws2812_multi
	cd C:\Users\Dan\Documents\Programming\Pico\mypico\build

PIO\ws2812_multi\ws2812_multi.pio.h: ..\PIO\ws2812_multi\ws2812_multi.pio
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir=C:\Users\Dan\Documents\Programming\Pico\mypico\build\CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Generating ws2812_multi.pio.h"
	cd C:\Users\Dan\Documents\Programming\Pico\mypico\build\PIO\ws2812_multi
	..\..\pioasm\pioasm.exe -o c-sdk C:/Users/Dan/Documents/Programming/Pico/mypico/PIO/ws2812_multi/ws2812_multi.pio C:/Users/Dan/Documents/Programming/Pico/mypico/build/PIO/ws2812_multi/ws2812_multi.pio.h
	cd C:\Users\Dan\Documents\Programming\Pico\mypico\build

ws2812_multi_ws2812_multi_pio_h: PIO\ws2812_multi\CMakeFiles\ws2812_multi_ws2812_multi_pio_h
ws2812_multi_ws2812_multi_pio_h: PIO\ws2812_multi\ws2812_multi.pio.h
ws2812_multi_ws2812_multi_pio_h: PIO\ws2812_multi\CMakeFiles\ws2812_multi_ws2812_multi_pio_h.dir\build.make
.PHONY : ws2812_multi_ws2812_multi_pio_h

# Rule to build all files generated by this target.
PIO\ws2812_multi\CMakeFiles\ws2812_multi_ws2812_multi_pio_h.dir\build: ws2812_multi_ws2812_multi_pio_h
.PHONY : PIO\ws2812_multi\CMakeFiles\ws2812_multi_ws2812_multi_pio_h.dir\build

PIO\ws2812_multi\CMakeFiles\ws2812_multi_ws2812_multi_pio_h.dir\clean:
	cd C:\Users\Dan\Documents\Programming\Pico\mypico\build\PIO\ws2812_multi
	$(CMAKE_COMMAND) -P CMakeFiles\ws2812_multi_ws2812_multi_pio_h.dir\cmake_clean.cmake
	cd C:\Users\Dan\Documents\Programming\Pico\mypico\build
.PHONY : PIO\ws2812_multi\CMakeFiles\ws2812_multi_ws2812_multi_pio_h.dir\clean

PIO\ws2812_multi\CMakeFiles\ws2812_multi_ws2812_multi_pio_h.dir\depend:
	$(CMAKE_COMMAND) -E cmake_depends "NMake Makefiles" C:\Users\Dan\Documents\Programming\Pico\mypico C:\Users\Dan\Documents\Programming\Pico\mypico\PIO\ws2812_multi C:\Users\Dan\Documents\Programming\Pico\mypico\build C:\Users\Dan\Documents\Programming\Pico\mypico\build\PIO\ws2812_multi C:\Users\Dan\Documents\Programming\Pico\mypico\build\PIO\ws2812_multi\CMakeFiles\ws2812_multi_ws2812_multi_pio_h.dir\DependInfo.cmake --color=$(COLOR)
.PHONY : PIO\ws2812_multi\CMakeFiles\ws2812_multi_ws2812_multi_pio_h.dir\depend

