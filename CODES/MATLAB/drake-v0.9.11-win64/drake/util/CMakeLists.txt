                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                ash\n"
     "\n"
     "# Usage:\n"
     "#   % drake_debug_mex.sh [args]\n"
     "# will set up the environment and then run:\n"
     "#   % args pod-build/bin/drake-debug-mex\n"
     "#\n"
     "# For example,\n"
     "#   % drake_debug_mex.sh\n"
     "# will simply run the executable,\n"
     "#   % drake_debug_mex.sh gdb\n"
     "# will run gdb on drake-debug-mex, and\n"
     "#   % drake_debug_mex.sh valgrind --leak-check=full --dsymutil=yes --track-origins=yes --xml=yes\n"
     "# will run valgrind with the appropriate arguments passed in.\n"
     "#\n"
     "# It's not pretty, but seems to work for the use\n"
     "# cases I've imagined so far. - RussT\n"
     "\n"
    )

if (APPLE)
  file(APPEND ${CMAKE_BINARY_DIR}/bin/drake_debug_mex.sh
       "export DYLD_LIBRARY_PATH=$DYLD_LIBRARY_PATH:${MATLAB_ROOT}/bin/${MATLAB_CPU}\n"
       "export DYLD_FORCE_FLAT_NAMESPACE=1\n"
       "export DYLD_INSERT_LIBRARIES=${CMAKE_BINARY_DIR}/lib/libdebugMex.dylib\n"
      )
else()
  file(APPEND ${CMAKE_BINARY_DIR}/bin/drake_debug_mex.sh
       "export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:${MATLAB_ROOT}/bin/${MATLAB_CPU}\n"
       "export LD_PRELOAD=${CMAKE_BINARY_DIR}/lib/libdebugMex.so\n"
      )
endif()

file(APPEND ${CMAKE_BINARY_DIR}/bin/drake_debug_mex.sh
     "\n"
     "\"\$@\" ${CMAKE_BINARY_DIR}/bin/drake_debug_mex\n"
    )

install(FILES ${CMAKE_BINARY_DIR}/bin/drake_debug_mex.sh
        DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
        PERMISSIONS OWNER_READ OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
endif(0) #NOT WIN32)
