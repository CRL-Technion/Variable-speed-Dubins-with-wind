# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.20

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

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

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /home/doron/.local/share/JetBrains/Toolbox/apps/CLion/ch-0/212.4746.93/bin/cmake/linux/bin/cmake

# The command to remove a file.
RM = /home/doron/.local/share/JetBrains/Toolbox/apps/CLion/ch-0/212.4746.93/bin/cmake/linux/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/doron/Desktop/git-clones/Variable-speed-Dubins-with-wind

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/doron/Desktop/git-clones/Variable-speed-Dubins-with-wind/cmake-build-debug

# Include any dependencies generated for this target.
include CMakeFiles/testEndpointAllCands.dir/depend.make
# Include the progress variables for this target.
include CMakeFiles/testEndpointAllCands.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/testEndpointAllCands.dir/flags.make

CMakeFiles/testEndpointAllCands.dir/programs/testEndpointAllCands.cpp.o: CMakeFiles/testEndpointAllCands.dir/flags.make
CMakeFiles/testEndpointAllCands.dir/programs/testEndpointAllCands.cpp.o: ../programs/testEndpointAllCands.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/doron/Desktop/git-clones/Variable-speed-Dubins-with-wind/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/testEndpointAllCands.dir/programs/testEndpointAllCands.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/testEndpointAllCands.dir/programs/testEndpointAllCands.cpp.o -c /home/doron/Desktop/git-clones/Variable-speed-Dubins-with-wind/programs/testEndpointAllCands.cpp

CMakeFiles/testEndpointAllCands.dir/programs/testEndpointAllCands.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/testEndpointAllCands.dir/programs/testEndpointAllCands.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/doron/Desktop/git-clones/Variable-speed-Dubins-with-wind/programs/testEndpointAllCands.cpp > CMakeFiles/testEndpointAllCands.dir/programs/testEndpointAllCands.cpp.i

CMakeFiles/testEndpointAllCands.dir/programs/testEndpointAllCands.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/testEndpointAllCands.dir/programs/testEndpointAllCands.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/doron/Desktop/git-clones/Variable-speed-Dubins-with-wind/programs/testEndpointAllCands.cpp -o CMakeFiles/testEndpointAllCands.dir/programs/testEndpointAllCands.cpp.s

# Object files for target testEndpointAllCands
testEndpointAllCands_OBJECTS = \
"CMakeFiles/testEndpointAllCands.dir/programs/testEndpointAllCands.cpp.o"

# External object files for target testEndpointAllCands
testEndpointAllCands_EXTERNAL_OBJECTS =

../bin/testEndpointAllCands: CMakeFiles/testEndpointAllCands.dir/programs/testEndpointAllCands.cpp.o
../bin/testEndpointAllCands: CMakeFiles/testEndpointAllCands.dir/build.make
../bin/testEndpointAllCands: ../lib/libVarSpeedDubins.a
../bin/testEndpointAllCands: CMakeFiles/testEndpointAllCands.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/doron/Desktop/git-clones/Variable-speed-Dubins-with-wind/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable ../bin/testEndpointAllCands"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/testEndpointAllCands.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/testEndpointAllCands.dir/build: ../bin/testEndpointAllCands
.PHONY : CMakeFiles/testEndpointAllCands.dir/build

CMakeFiles/testEndpointAllCands.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/testEndpointAllCands.dir/cmake_clean.cmake
.PHONY : CMakeFiles/testEndpointAllCands.dir/clean

CMakeFiles/testEndpointAllCands.dir/depend:
	cd /home/doron/Desktop/git-clones/Variable-speed-Dubins-with-wind/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/doron/Desktop/git-clones/Variable-speed-Dubins-with-wind /home/doron/Desktop/git-clones/Variable-speed-Dubins-with-wind /home/doron/Desktop/git-clones/Variable-speed-Dubins-with-wind/cmake-build-debug /home/doron/Desktop/git-clones/Variable-speed-Dubins-with-wind/cmake-build-debug /home/doron/Desktop/git-clones/Variable-speed-Dubins-with-wind/cmake-build-debug/CMakeFiles/testEndpointAllCands.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/testEndpointAllCands.dir/depend

