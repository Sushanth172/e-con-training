# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.10

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/sushanth/Git/e-con-training/V4L2Library/libv4l2dev

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/sushanth/Git/e-con-training/V4L2Library/libv4l2dev/build

# Include any dependencies generated for this target.
include CMakeFiles/v4l2dev.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/v4l2dev.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/v4l2dev.dir/flags.make

CMakeFiles/v4l2dev.dir/v4l2dev.cpp.o: CMakeFiles/v4l2dev.dir/flags.make
CMakeFiles/v4l2dev.dir/v4l2dev.cpp.o: ../v4l2dev.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/sushanth/Git/e-con-training/V4L2Library/libv4l2dev/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/v4l2dev.dir/v4l2dev.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/v4l2dev.dir/v4l2dev.cpp.o -c /home/sushanth/Git/e-con-training/V4L2Library/libv4l2dev/v4l2dev.cpp

CMakeFiles/v4l2dev.dir/v4l2dev.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/v4l2dev.dir/v4l2dev.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/sushanth/Git/e-con-training/V4L2Library/libv4l2dev/v4l2dev.cpp > CMakeFiles/v4l2dev.dir/v4l2dev.cpp.i

CMakeFiles/v4l2dev.dir/v4l2dev.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/v4l2dev.dir/v4l2dev.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/sushanth/Git/e-con-training/V4L2Library/libv4l2dev/v4l2dev.cpp -o CMakeFiles/v4l2dev.dir/v4l2dev.cpp.s

CMakeFiles/v4l2dev.dir/v4l2dev.cpp.o.requires:

.PHONY : CMakeFiles/v4l2dev.dir/v4l2dev.cpp.o.requires

CMakeFiles/v4l2dev.dir/v4l2dev.cpp.o.provides: CMakeFiles/v4l2dev.dir/v4l2dev.cpp.o.requires
	$(MAKE) -f CMakeFiles/v4l2dev.dir/build.make CMakeFiles/v4l2dev.dir/v4l2dev.cpp.o.provides.build
.PHONY : CMakeFiles/v4l2dev.dir/v4l2dev.cpp.o.provides

CMakeFiles/v4l2dev.dir/v4l2dev.cpp.o.provides.build: CMakeFiles/v4l2dev.dir/v4l2dev.cpp.o


# Object files for target v4l2dev
v4l2dev_OBJECTS = \
"CMakeFiles/v4l2dev.dir/v4l2dev.cpp.o"

# External object files for target v4l2dev
v4l2dev_EXTERNAL_OBJECTS =

libv4l2dev.so.1.0.0: CMakeFiles/v4l2dev.dir/v4l2dev.cpp.o
libv4l2dev.so.1.0.0: CMakeFiles/v4l2dev.dir/build.make
libv4l2dev.so.1.0.0: CMakeFiles/v4l2dev.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/sushanth/Git/e-con-training/V4L2Library/libv4l2dev/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX shared library libv4l2dev.so"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/v4l2dev.dir/link.txt --verbose=$(VERBOSE)
	$(CMAKE_COMMAND) -E cmake_symlink_library libv4l2dev.so.1.0.0 libv4l2dev.so.1 libv4l2dev.so

libv4l2dev.so.1: libv4l2dev.so.1.0.0
	@$(CMAKE_COMMAND) -E touch_nocreate libv4l2dev.so.1

libv4l2dev.so: libv4l2dev.so.1.0.0
	@$(CMAKE_COMMAND) -E touch_nocreate libv4l2dev.so

# Rule to build all files generated by this target.
CMakeFiles/v4l2dev.dir/build: libv4l2dev.so

.PHONY : CMakeFiles/v4l2dev.dir/build

CMakeFiles/v4l2dev.dir/requires: CMakeFiles/v4l2dev.dir/v4l2dev.cpp.o.requires

.PHONY : CMakeFiles/v4l2dev.dir/requires

CMakeFiles/v4l2dev.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/v4l2dev.dir/cmake_clean.cmake
.PHONY : CMakeFiles/v4l2dev.dir/clean

CMakeFiles/v4l2dev.dir/depend:
	cd /home/sushanth/Git/e-con-training/V4L2Library/libv4l2dev/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/sushanth/Git/e-con-training/V4L2Library/libv4l2dev /home/sushanth/Git/e-con-training/V4L2Library/libv4l2dev /home/sushanth/Git/e-con-training/V4L2Library/libv4l2dev/build /home/sushanth/Git/e-con-training/V4L2Library/libv4l2dev/build /home/sushanth/Git/e-con-training/V4L2Library/libv4l2dev/build/CMakeFiles/v4l2dev.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/v4l2dev.dir/depend

