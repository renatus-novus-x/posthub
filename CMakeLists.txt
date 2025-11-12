cmake_minimum_required(VERSION 3.6)

project(posthub)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${PROJECT_SOURCE_DIR}/bin)

file(GLOB_RECURSE SRC "src/*.c")
add_executable(posthub ${SRC}
)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT posthub)
set(CMAKE_CONFIGURATION_TYPES "Debug;Release")
set(CMAKE_SUPPRESS_REGENERATION true)

if(MSVC)
  add_definitions(-D_UNICODE -DUNICODE -D_USE_MATH_DEFINES -DNOMINMAX -D_CRT_SECURE_NO_WARNINGS)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++latest")
  #set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS")
  #set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /ENTRY:mainCRTStartup")
  set_target_properties(posthub PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "$(ProjectDir)/../bin")

  set(CompilerFlags
    CMAKE_CXX_FLAGS
    CMAKE_CXX_FLAGS_DEBUG
    CMAKE_CXX_FLAGS_RELEASE
    CMAKE_C_FLAGS
    CMAKE_C_FLAGS_DEBUG
    CMAKE_C_FLAGS_RELEASE
  )
  #foreach(CompilerFlag ${CompilerFlags})
  #  string(REPLACE "/MD" "/MT" ${CompilerFlag} "${${CompilerFlag}}")
  #endforeach()

add_custom_command(
  TARGET ${PROJECT_NAME}
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E make_directory "${PROJECT_SOURCE_DIR}/bin"
)
endif(MSVC)

if(CMAKE_SYSTEM_NAME MATCHES "Darwin")
  set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64")
endif()

if(UNIX AND NOT APPLE)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

target_link_libraries(posthub)
