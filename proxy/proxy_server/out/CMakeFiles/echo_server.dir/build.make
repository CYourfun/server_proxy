# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.5

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
CMAKE_SOURCE_DIR = /home/zzy/Work/server/proxy/proxy_server

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/zzy/Work/server/proxy/proxy_server/out

# Include any dependencies generated for this target.
include CMakeFiles/echo_server.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/echo_server.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/echo_server.dir/flags.make

CMakeFiles/echo_server.dir/main.c.o: CMakeFiles/echo_server.dir/flags.make
CMakeFiles/echo_server.dir/main.c.o: ../main.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/zzy/Work/server/proxy/proxy_server/out/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object CMakeFiles/echo_server.dir/main.c.o"
	/usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/echo_server.dir/main.c.o   -c /home/zzy/Work/server/proxy/proxy_server/main.c

CMakeFiles/echo_server.dir/main.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/echo_server.dir/main.c.i"
	/usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/zzy/Work/server/proxy/proxy_server/main.c > CMakeFiles/echo_server.dir/main.c.i

CMakeFiles/echo_server.dir/main.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/echo_server.dir/main.c.s"
	/usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/zzy/Work/server/proxy/proxy_server/main.c -o CMakeFiles/echo_server.dir/main.c.s

CMakeFiles/echo_server.dir/main.c.o.requires:

.PHONY : CMakeFiles/echo_server.dir/main.c.o.requires

CMakeFiles/echo_server.dir/main.c.o.provides: CMakeFiles/echo_server.dir/main.c.o.requires
	$(MAKE) -f CMakeFiles/echo_server.dir/build.make CMakeFiles/echo_server.dir/main.c.o.provides.build
.PHONY : CMakeFiles/echo_server.dir/main.c.o.provides

CMakeFiles/echo_server.dir/main.c.o.provides.build: CMakeFiles/echo_server.dir/main.c.o


CMakeFiles/echo_server.dir/ev2.c.o: CMakeFiles/echo_server.dir/flags.make
CMakeFiles/echo_server.dir/ev2.c.o: ../ev2.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/zzy/Work/server/proxy/proxy_server/out/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building C object CMakeFiles/echo_server.dir/ev2.c.o"
	/usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/echo_server.dir/ev2.c.o   -c /home/zzy/Work/server/proxy/proxy_server/ev2.c

CMakeFiles/echo_server.dir/ev2.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/echo_server.dir/ev2.c.i"
	/usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/zzy/Work/server/proxy/proxy_server/ev2.c > CMakeFiles/echo_server.dir/ev2.c.i

CMakeFiles/echo_server.dir/ev2.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/echo_server.dir/ev2.c.s"
	/usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/zzy/Work/server/proxy/proxy_server/ev2.c -o CMakeFiles/echo_server.dir/ev2.c.s

CMakeFiles/echo_server.dir/ev2.c.o.requires:

.PHONY : CMakeFiles/echo_server.dir/ev2.c.o.requires

CMakeFiles/echo_server.dir/ev2.c.o.provides: CMakeFiles/echo_server.dir/ev2.c.o.requires
	$(MAKE) -f CMakeFiles/echo_server.dir/build.make CMakeFiles/echo_server.dir/ev2.c.o.provides.build
.PHONY : CMakeFiles/echo_server.dir/ev2.c.o.provides

CMakeFiles/echo_server.dir/ev2.c.o.provides.build: CMakeFiles/echo_server.dir/ev2.c.o


CMakeFiles/echo_server.dir/echo_server.c.o: CMakeFiles/echo_server.dir/flags.make
CMakeFiles/echo_server.dir/echo_server.c.o: ../echo_server.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/zzy/Work/server/proxy/proxy_server/out/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building C object CMakeFiles/echo_server.dir/echo_server.c.o"
	/usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/echo_server.dir/echo_server.c.o   -c /home/zzy/Work/server/proxy/proxy_server/echo_server.c

CMakeFiles/echo_server.dir/echo_server.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/echo_server.dir/echo_server.c.i"
	/usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/zzy/Work/server/proxy/proxy_server/echo_server.c > CMakeFiles/echo_server.dir/echo_server.c.i

CMakeFiles/echo_server.dir/echo_server.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/echo_server.dir/echo_server.c.s"
	/usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/zzy/Work/server/proxy/proxy_server/echo_server.c -o CMakeFiles/echo_server.dir/echo_server.c.s

CMakeFiles/echo_server.dir/echo_server.c.o.requires:

.PHONY : CMakeFiles/echo_server.dir/echo_server.c.o.requires

CMakeFiles/echo_server.dir/echo_server.c.o.provides: CMakeFiles/echo_server.dir/echo_server.c.o.requires
	$(MAKE) -f CMakeFiles/echo_server.dir/build.make CMakeFiles/echo_server.dir/echo_server.c.o.provides.build
.PHONY : CMakeFiles/echo_server.dir/echo_server.c.o.provides

CMakeFiles/echo_server.dir/echo_server.c.o.provides.build: CMakeFiles/echo_server.dir/echo_server.c.o


# Object files for target echo_server
echo_server_OBJECTS = \
"CMakeFiles/echo_server.dir/main.c.o" \
"CMakeFiles/echo_server.dir/ev2.c.o" \
"CMakeFiles/echo_server.dir/echo_server.c.o"

# External object files for target echo_server
echo_server_EXTERNAL_OBJECTS =

echo_server: CMakeFiles/echo_server.dir/main.c.o
echo_server: CMakeFiles/echo_server.dir/ev2.c.o
echo_server: CMakeFiles/echo_server.dir/echo_server.c.o
echo_server: CMakeFiles/echo_server.dir/build.make
echo_server: CMakeFiles/echo_server.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/zzy/Work/server/proxy/proxy_server/out/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Linking C executable echo_server"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/echo_server.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/echo_server.dir/build: echo_server

.PHONY : CMakeFiles/echo_server.dir/build

CMakeFiles/echo_server.dir/requires: CMakeFiles/echo_server.dir/main.c.o.requires
CMakeFiles/echo_server.dir/requires: CMakeFiles/echo_server.dir/ev2.c.o.requires
CMakeFiles/echo_server.dir/requires: CMakeFiles/echo_server.dir/echo_server.c.o.requires

.PHONY : CMakeFiles/echo_server.dir/requires

CMakeFiles/echo_server.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/echo_server.dir/cmake_clean.cmake
.PHONY : CMakeFiles/echo_server.dir/clean

CMakeFiles/echo_server.dir/depend:
	cd /home/zzy/Work/server/proxy/proxy_server/out && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/zzy/Work/server/proxy/proxy_server /home/zzy/Work/server/proxy/proxy_server /home/zzy/Work/server/proxy/proxy_server/out /home/zzy/Work/server/proxy/proxy_server/out /home/zzy/Work/server/proxy/proxy_server/out/CMakeFiles/echo_server.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/echo_server.dir/depend

