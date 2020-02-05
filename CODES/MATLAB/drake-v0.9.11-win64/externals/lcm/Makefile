
# support only a few flags that might be passed through:
#   BUILD_PREFIX, BUILD_TYPE, CMAKE_FLAGS*
CMAKE_FLAGS+=$(strip $(CMAKE_FLAGS1) $(CMAKE_FLAGS2) $(CMAKE_FLAGS3) $(CMAKE_FLAGS4) $(CMAKE_FLAGS5) $(CMAKE_FLAGS6) $(CMAKE_FLAGS7) $(CMAKE_FLAGS8) $(CMAKE_FLAGS9) $(CMAKE_FLAGS10) $(CMAKE_FLAGS11) $(CMAKE_FLAGS12) $(CMAKE_FLAGS13) $(CMAKE_FLAGS14) $(CMAKE_FLAGS15) $(CMAKE_FLAGS16) $(CMAKE_FLAGS17) $(CMAKE_FLAGS18) $(CMAKE_FLAGS19) $(CMAKE_FLAGS20))
ifneq "$(BUILD_PREFIX)" ""
  CMAKE_FLAGS+=-DCMAKE_INSTALL_PREFIX="$(BUILD_PREFIX)"
endif
ifeq "$(BUILD_TYPE)" ""
  BUILD_TYPE="Release"
endif
CMAKE_FLAGS+=-DCMAKE_BUILD_TYPE=$(BUILD_TYPE)
CMAKE_CONFIG=--config $(BUILD_TYPE)

.PHONY: all
all: configure
	cmake --build lcm-1.0.0/pod-build $(CMAKE_CONFIG) --target install

lcm-1.0.0/pod-build:
	cmake -E make_directory lcm-1.0.0/pod-build

.PHONY: configure
configure: lcm-1.0.0/pod-build
	@echo Configuring with CMAKE_FLAGS: $(CMAKE_FLAGS)
	@cd lcm-1.0.0/pod-build && cmake $(CMAKE_FLAGS) ..

.PHONY: options
options: configure
ifeq ($(OS),Windows_NT)
	cmake-gui lcm-1.0.0/pod-build
else
	ccmake lcm-1.0.0/pod-build
endif

.PHONY: clean
clean:
	cmake --build lcm-1.0.0/pod-build --target clean
	cmake -E remove_directory lcm-1.0.0/pod-build
	cmake -E remove_directory build

# other (custom) targets are passed through to the cmake-generated Makefile
%::
	cmake --build lcm-1.0.0/pod-build $(CMAKE_CONFIG) --target $@

# Default to a less-verbose build.  If you want all the gory compiler output,
# run "make VERBOSE=1"
$(VERBOSE).SILENT:


### rules specific to this project (everything above is the generic pods makefile wrapper of a cmake project)
