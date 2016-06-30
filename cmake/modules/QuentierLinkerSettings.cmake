if(CMAKE_COMPILER_IS_GNUCXX OR (${CMAKE_CXX_COMPILER_ID} MATCHES "Clang"))
  # Use ld.gold if it is available and isn't disabled explicitly
  option(USE_LD_GOLD "Use GNU gold linker" ON)
  if (USE_LD_GOLD)
    execute_process(COMMAND ${CMAKE_C_COMPILER} -fuse-ld=gold -Wl,--version ERROR_QUIET OUTPUT_VARIABLE LD_VERSION)
    if ("${LD_VERSION}" MATCHES "GNU gold")
      message(STATUS "Using GNU gold linker")
      set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fuse-ld=gold")
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fuse-ld=gold")
      set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -ldl")
    else ()
      message(STATUS "GNU gold linker isn't available, using the default system linker.")
    endif ()
  endif ()
endif()

