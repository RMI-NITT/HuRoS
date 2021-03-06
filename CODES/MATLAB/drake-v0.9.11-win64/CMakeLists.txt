                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                tps://github.com/RobotLocomotion/xfoil.git)
set(xfoil_GIT_TAG fde0a9368dd451c93604051fc5704e120073800c)
set(xfoil_IS_PUBLIC TRUE)

# and now for the examples
set(LittleDog_GIT_REPOSITORY https://github.com/RobotLocomotion/LittleDog.git)
set(LittleDog_GIT_TAG a21aef548c33386243c40e315b1c70d83c122430)

find_program(MAKE_EXECUTABLE make)
if (NOT MAKE_EXECUTABLE)
	message(FATAL_ERROR "couldn't find gnu make")
endif()
if (${CMAKE_GENERATOR} STREQUAL "Unix Makefiles")
	set(PODS_MAKE_COMMAND "$(MAKE)")   # so we can pass through commandline arguments.
else()
	set(PODS_MAKE_COMMAND ${MAKE_EXECUTABLE})
endif()

# process optional projects
# note: keep drake in this loop in case externals depend on drake (e.g. the director might in the near future)
set(EXTERNAL_PROJECTS)
foreach(proj IN ITEMS
		cmake
		eigen
		gtk
		lcm
		libbot
		bullet
		spotless
		director
		signalscope
		octomap
		snopt
		gurobi
		mosek
		iris
		yalmip
		gloptipoly
		bertini
		sedumi
		avl
		xfoil
		meshconverters
		drake)
	string(TOUPPER ${proj} proj_upper)
	if ( ${proj} STREQUAL "drake" OR
			${proj} STREQUAL "cmake" OR
			WITH_${proj_upper} OR
	(WITH_ALL_SUPPORTED_EXTERNALS AND DEFINED WITH_${proj_upper}) OR
	(WITH_ALL_PUBLIC_EXTERNALS AND ${proj}_IS_PUBLIC AND DEFINED WITH_${proj_upper}) )
		list(APPEND EXTERNAL_PROJECTS ${proj})
	elseif(REMOVE_UNUSED_EXTERNALS AND IS_DIRECTORY ${proj})
		message(STATUS "removing unused project: ${proj}")
		if (NOT ${proj}_NO_BUILD)
			execute_process(COMMAND ${MAKE_EXECUTABLE} BUILD_PREFIX=${CMAKE_INSTALL_PREFIX} BUILD_TYPE=${CMAKE_BUILD_TYPE} clean
					WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/${proj})
		endif()
		execute_process(COMMAND ${CMAKE_COMMAND} -E remove_directory "${PROJECT_SOURCE_DIR}/${proj}")
	endif()
endforeach()

set(EXTERNAL_SOURCE_DIRS)
foreach (proj ${EXTERNAL_PROJECTS})
	set(deps)
	foreach(dep ${${proj}_dependencies})
		list(FIND EXTERNAL_PROJECTS ${dep} find_result)
		if (${dep} STREQUAL "drake" OR NOT find_result EQUAL -1)
			list(APPEND deps ${dep})
		endif()
	endforeach()

	if (NOT ${proj} STREQUAL "drake")
		if (NOT ${proj}_GIT_CLONE_DIR)
			set(${proj}_GIT_CLONE_DIR "${PROJECT_SOURCE_DIR}/externals/${proj}")
		endif()
		if (NOT ${proj}_SOURCE_DIR)
			set(${proj}_SOURCE_DIR ${${proj}_GIT_CLONE_DIR})
		endif()
		if (NOT DEFINED ${proj}_CONFIGURE_COMMAND AND ${proj}_IS_CMAKE_POD)
			set(${proj}_CONFIGURE_COMMAND ${CMAKE_COMMAND} -E make_directory ${${proj}_SOURCE_DIR}/pod-build
					COMMAND ${CMAKE_COMMAND} -E chdir ${${proj}_SOURCE_DIR}/pod-build ${CMAKE_COMMAND} -G${CMAKE_GENERATOR} -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX} -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} ..)
		endif()
		if (NOT DEFINED ${proj}_BUILD_COMMAND)
			if (${proj}_IS_CMAKE_POD)
				set(${proj}_BUILD_COMMAND ${CMAKE_COMMAND} -E chdir ${${proj}_SOURCE_DIR} ${CMAKE_COMMAND} --build pod-build --config ${CMAKE_BUILD_TYPE} --target install)
			elseif(${proj}_NO_BUILD)
				set(${proj}_BUILD_COMMAND "")
			else()# then use the PODS gnu make system
				set(${proj}_BUILD_COMMAND ${PODS_MAKE_COMMAND} BUILD_PREFIX=${CMAKE_INSTALL_PREFIX} BUILD_TYPE=${CMAKE_BUILD_TYPE})
			endif()
		endif()
		set(EXTERNAL_SOURCE_DIRS ${EXTERNAL_SOURCE_DIRS} ${${proj}_SOURCE_DIR} "\n")

		message(STATUS "Preparing to build ${proj} with dependencies: ${deps}")

		# separate download target so I can make the download-all custom command as recommended in:
		#   http://comments.gmane.org/gmane.comp.programming.tools.cmake.user/53002
		if (AUTO_UPDATE_EXTERNALS)
			ExternalProject_Add(download-${proj}
					DOWNLOAD_DIR ${${proj}_GIT_CLONE_DIR}
					SOURCE_DIR ${${proj}_GIT_CLONE_DIR}
					GIT_REPOSITORY ${${proj}_GIT_REPOSITORY}
					GIT_TAG ${${proj}_GIT_TAG}
					CONFIGURE_COMMAND ""
					BUILD_COMMAND ""
					INSTALL_COMMAND ""
					)
		else()
			ExternalProject_Add(download-${proj}
					DOWNLOAD_DIR ${${proj}_GIT_CLONE_DIR}
					SOURCE_DIR ${${proj}_GIT_CLONE_DIR}
					GIT_REPOSITORY ${${proj}_GIT_REPOSITORY}
					GIT_TAG ${${proj}_GIT_TAG}
					UPDATE_COMMAND ""
					CONFIGURE_COMMAND ""
					BUILD_COMMAND ""
					INSTALL_COMMAND ""
					)
		endif()

		# now the actual project
		ExternalProject_Add(${proj}
				SOURCE_DIR ${${proj}_SOURCE_DIR}
				#      BINARY_DIR ${${proj}_SOURCE_DIR}/pod-build
				DOWNLOAD_COMMAND ""
				UPDATE_COMMAND ""
				CONFIGURE_COMMAND "${${proj}_CONFIGURE_COMMAND}"
				BUILD_IN_SOURCE 1
				BUILD_COMMAND "${${proj}_BUILD_COMMAND}"
				INSTALL_COMMAND ""
				DEPENDS download-${proj} ${deps}
				${${proj}_ADDITIONAL_BUILD_ARGS}
				)
		#    message(STATUS "${proj}_BUILD_COMMAND: ${${proj}_BUILD_COMMAND}")

	else()
		message(STATUS "Preparing to build ${proj} with dependencies: ${deps}")

		ExternalProject_Add(${proj}
				SOURCE_DIR "${PROJECT_SOURCE_DIR}/drake"
				DOWNLOAD_COMMAND ""
				UPDATE_COMMAND ""
				#      CONFIGURE_COMMAND ""
				#      BUILD_IN_SOURCE 1
				#      BUILD_COMMAND ${PODS_MAKE_COMMAND} BUILD_PREFIX=${CMAKE_INSTALL_PREFIX} BUILD_TYPE=${CMAKE_BUILD_TYPE}
				CONFIGURE_COMMAND ${CMAKE_COMMAND} -E make_directory ${PROJECT_SOURCE_DIR}/drake/pod-build
				COMMAND ${CMAKE_COMMAND} -E chdir ${PROJECT_SOURCE_DIR}/drake/pod-build ${CMAKE_COMMAND} -G${CMAKE_GENERATOR} -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX} -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} ..
				BUILD_COMMAND ${CMAKE_COMMAND} --build ${PROJECT_SOURCE_DIR}/drake/pod-build --config ${CMAKE_BUILD_TYPE} --target install
				INSTALL_COMMAND ""
				DEPENDS ${deps}
				${${proj}_ADDITIONAL_BUILD_ARGS}
				#		BUILD_ALWAYS 1
				)
	endif(NOT ${proj} STREQUAL "drake")


	# once we require cmake version >= 3.1, then we can zap this and just use the BUILD_ALWAYS flags above
	ExternalProject_Add_Step(${proj} forceconfigure
			COMMAND ${CMAKE_COMMAND} -E echo "Force configure of ${proj}"
			DEPENDEES update
			DEPENDERS configure
			ALWAYS 1)

endforeach()
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/drake_external_source_dirs.txt ${EXTERNAL_SOURCE_DIRS})

# todo: add a custom target for release_filelist

add_custom_target(download-all)
add_custom_target(clean-all)
add_custom_target(status)
set(PROJECT_LIST)
foreach (proj ${EXTERNAL_PROJECTS})
	if (NOT ${proj} STREQUAL "drake")
		add_dependencies(download-all download-${proj})
		ExternalProject_Get_Property(download-${proj} SOURCE_DIR)
		add_custom_target(status-${proj}
				COMMAND ${GIT_EXECUTABLE} status
				WORKING_DIRECTORY ${SOURCE_DIR})
		add_dependencies(status status-${proj})
	endif()

	ExternalProject_Get_Property(${proj} SOURCE_DIR)
	if (NOT ${proj}_NO_BUILD)
		add_custom_target(clean-${proj}
				COMMAND ${PODS_MAKE_COMMAND} BUILD_PREFIX=${CMAKE_INSTALL_PREFIX} BUILD_TYPE=${CMAKE_BUILD_TYPE} clean
				WORKING_DIRECTORY ${SOURCE_DIR})
		add_dependencies(clean-all clean-${proj})
	endif()
	list(APPEND PROJECT_LIST ${SOURCE_DIR})
endforeach()

# process optional examples
foreach (example IN ITEMS
		LittleDog)
	string(TOUPPER ${example} example_upper)
	if (EXAMPLES_${example_upper})
		message(STATUS "Installation will include extra example: ${example}")
		ExternalProject_Add(download-${example}
				DOWNLOAD_DIR ${PROJECT_SOURCE_DIR}/drake/examples/${example}
				SOURCE_DIR ${PROJECT_SOURCE_DIR}/drake/examples/${example}
				GIT_REPOSITORY ${${example}_GIT_REPOSITORY}
				GIT_TAG ${${example}_GIT_TAG}
				CONFIGURE_COMMAND ""
				BUILD_COMMAND ""
				INSTALL_COMMAND ""
				)
		add_dependencies(drake download-${example})
		add_dependencies(download-all download-${example})
	endif()
endforeach()

string(REPLACE ";" " " PROJECT_LIST "${PROJECT_LIST}")
add_custom_target(list-project-dirs COMMAND echo "${PROJECT_LIST}")

## grab and install precompiled snopt

# todo: look for snopt_c
if (snopt_c_FOUND OR WITH_SNOPT)
  set(WITH_SNOPT_PRECOMPILED OFF)
endif()
if (WITH_SNOPT_PRECOMPILED)
  message(STATUS "Preparing to install precompiled snopt")
  ExternalProject_Add(download-snopt-precompiled
		URL "http://drake002.csail.mit.edu/drake/drakeSnopt.zip"
		SOURCE_DIR "${CMAKE_BINARY_DIR}/snopt-precompiled"
		CONFIGURE_COMMAND ""
		BUILD_COMMAND cmake -E copy_directory ${CMAKE_BINARY_DIR}/snopt-precompiled/ ${PROJECT_SOURCE_DIR}/drake/solvers/
		INSTALL_COMMAND "")
  add_dependencies(download-all download-snopt-precompiled)
  add_dependencies(drake download-snopt-precompiled) # just in case: make sure any compiled drake version happens after precompiled install
endif()

