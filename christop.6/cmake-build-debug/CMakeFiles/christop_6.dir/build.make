# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.12

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
CMAKE_COMMAND = /Applications/CLion.app/Contents/bin/cmake/mac/bin/cmake

# The command to remove a file.
RM = /Applications/CLion.app/Contents/bin/cmake/mac/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /Users/christhompson/Desktop/OS2/christop.6

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /Users/christhompson/Desktop/OS2/christop.6/cmake-build-debug

# Include any dependencies generated for this target.
include CMakeFiles/christop_6.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/christop_6.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/christop_6.dir/flags.make

CMakeFiles/christop_6.dir/master.c.o: CMakeFiles/christop_6.dir/flags.make
CMakeFiles/christop_6.dir/master.c.o: ../master.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/christhompson/Desktop/OS2/christop.6/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object CMakeFiles/christop_6.dir/master.c.o"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/christop_6.dir/master.c.o   -c /Users/christhompson/Desktop/OS2/christop.6/master.c

CMakeFiles/christop_6.dir/master.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/christop_6.dir/master.c.i"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /Users/christhompson/Desktop/OS2/christop.6/master.c > CMakeFiles/christop_6.dir/master.c.i

CMakeFiles/christop_6.dir/master.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/christop_6.dir/master.c.s"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /Users/christhompson/Desktop/OS2/christop.6/master.c -o CMakeFiles/christop_6.dir/master.c.s

CMakeFiles/christop_6.dir/child.c.o: CMakeFiles/christop_6.dir/flags.make
CMakeFiles/christop_6.dir/child.c.o: ../child.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/christhompson/Desktop/OS2/christop.6/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building C object CMakeFiles/christop_6.dir/child.c.o"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/christop_6.dir/child.c.o   -c /Users/christhompson/Desktop/OS2/christop.6/child.c

CMakeFiles/christop_6.dir/child.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/christop_6.dir/child.c.i"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /Users/christhompson/Desktop/OS2/christop.6/child.c > CMakeFiles/christop_6.dir/child.c.i

CMakeFiles/christop_6.dir/child.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/christop_6.dir/child.c.s"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /Users/christhompson/Desktop/OS2/christop.6/child.c -o CMakeFiles/christop_6.dir/child.c.s

# Object files for target christop_6
christop_6_OBJECTS = \
"CMakeFiles/christop_6.dir/master.c.o" \
"CMakeFiles/christop_6.dir/child.c.o"

# External object files for target christop_6
christop_6_EXTERNAL_OBJECTS =

christop_6: CMakeFiles/christop_6.dir/master.c.o
christop_6: CMakeFiles/christop_6.dir/child.c.o
christop_6: CMakeFiles/christop_6.dir/build.make
christop_6: CMakeFiles/christop_6.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/Users/christhompson/Desktop/OS2/christop.6/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Linking C executable christop_6"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/christop_6.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/christop_6.dir/build: christop_6

.PHONY : CMakeFiles/christop_6.dir/build

CMakeFiles/christop_6.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/christop_6.dir/cmake_clean.cmake
.PHONY : CMakeFiles/christop_6.dir/clean

CMakeFiles/christop_6.dir/depend:
	cd /Users/christhompson/Desktop/OS2/christop.6/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Users/christhompson/Desktop/OS2/christop.6 /Users/christhompson/Desktop/OS2/christop.6 /Users/christhompson/Desktop/OS2/christop.6/cmake-build-debug /Users/christhompson/Desktop/OS2/christop.6/cmake-build-debug /Users/christhompson/Desktop/OS2/christop.6/cmake-build-debug/CMakeFiles/christop_6.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/christop_6.dir/depend

