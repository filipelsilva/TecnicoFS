# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.17

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
CMAKE_COMMAND = /home/sofia/.local/share/JetBrains/Toolbox/apps/CLion/ch-0/202.7660.37/bin/cmake/linux/bin/cmake

# The command to remove a file.
RM = /home/sofia/.local/share/JetBrains/Toolbox/apps/CLion/ch-0/202.7660.37/bin/cmake/linux/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/sofia/Desktop/TecnicoFS

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/sofia/Desktop/TecnicoFS/cmake-build-debug

# Include any dependencies generated for this target.
include CMakeFiles/TecnicoFS.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/TecnicoFS.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/TecnicoFS.dir/flags.make

CMakeFiles/TecnicoFS.dir/fs/operations.c.o: CMakeFiles/TecnicoFS.dir/flags.make
CMakeFiles/TecnicoFS.dir/fs/operations.c.o: ../fs/operations.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/sofia/Desktop/TecnicoFS/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object CMakeFiles/TecnicoFS.dir/fs/operations.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/TecnicoFS.dir/fs/operations.c.o   -c /home/sofia/Desktop/TecnicoFS/fs/operations.c

CMakeFiles/TecnicoFS.dir/fs/operations.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/TecnicoFS.dir/fs/operations.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/sofia/Desktop/TecnicoFS/fs/operations.c > CMakeFiles/TecnicoFS.dir/fs/operations.c.i

CMakeFiles/TecnicoFS.dir/fs/operations.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/TecnicoFS.dir/fs/operations.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/sofia/Desktop/TecnicoFS/fs/operations.c -o CMakeFiles/TecnicoFS.dir/fs/operations.c.s

CMakeFiles/TecnicoFS.dir/fs/state.c.o: CMakeFiles/TecnicoFS.dir/flags.make
CMakeFiles/TecnicoFS.dir/fs/state.c.o: ../fs/state.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/sofia/Desktop/TecnicoFS/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building C object CMakeFiles/TecnicoFS.dir/fs/state.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/TecnicoFS.dir/fs/state.c.o   -c /home/sofia/Desktop/TecnicoFS/fs/state.c

CMakeFiles/TecnicoFS.dir/fs/state.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/TecnicoFS.dir/fs/state.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/sofia/Desktop/TecnicoFS/fs/state.c > CMakeFiles/TecnicoFS.dir/fs/state.c.i

CMakeFiles/TecnicoFS.dir/fs/state.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/TecnicoFS.dir/fs/state.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/sofia/Desktop/TecnicoFS/fs/state.c -o CMakeFiles/TecnicoFS.dir/fs/state.c.s

CMakeFiles/TecnicoFS.dir/main.c.o: CMakeFiles/TecnicoFS.dir/flags.make
CMakeFiles/TecnicoFS.dir/main.c.o: ../main.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/sofia/Desktop/TecnicoFS/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building C object CMakeFiles/TecnicoFS.dir/main.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/TecnicoFS.dir/main.c.o   -c /home/sofia/Desktop/TecnicoFS/main.c

CMakeFiles/TecnicoFS.dir/main.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/TecnicoFS.dir/main.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/sofia/Desktop/TecnicoFS/main.c > CMakeFiles/TecnicoFS.dir/main.c.i

CMakeFiles/TecnicoFS.dir/main.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/TecnicoFS.dir/main.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/sofia/Desktop/TecnicoFS/main.c -o CMakeFiles/TecnicoFS.dir/main.c.s

# Object files for target TecnicoFS
TecnicoFS_OBJECTS = \
"CMakeFiles/TecnicoFS.dir/fs/operations.c.o" \
"CMakeFiles/TecnicoFS.dir/fs/state.c.o" \
"CMakeFiles/TecnicoFS.dir/main.c.o"

# External object files for target TecnicoFS
TecnicoFS_EXTERNAL_OBJECTS =

TecnicoFS: CMakeFiles/TecnicoFS.dir/fs/operations.c.o
TecnicoFS: CMakeFiles/TecnicoFS.dir/fs/state.c.o
TecnicoFS: CMakeFiles/TecnicoFS.dir/main.c.o
TecnicoFS: CMakeFiles/TecnicoFS.dir/build.make
TecnicoFS: CMakeFiles/TecnicoFS.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/sofia/Desktop/TecnicoFS/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Linking C executable TecnicoFS"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/TecnicoFS.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/TecnicoFS.dir/build: TecnicoFS

.PHONY : CMakeFiles/TecnicoFS.dir/build

CMakeFiles/TecnicoFS.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/TecnicoFS.dir/cmake_clean.cmake
.PHONY : CMakeFiles/TecnicoFS.dir/clean

CMakeFiles/TecnicoFS.dir/depend:
	cd /home/sofia/Desktop/TecnicoFS/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/sofia/Desktop/TecnicoFS /home/sofia/Desktop/TecnicoFS /home/sofia/Desktop/TecnicoFS/cmake-build-debug /home/sofia/Desktop/TecnicoFS/cmake-build-debug /home/sofia/Desktop/TecnicoFS/cmake-build-debug/CMakeFiles/TecnicoFS.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/TecnicoFS.dir/depend

