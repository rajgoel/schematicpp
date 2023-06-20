cmake_minimum_required(VERSION 3.1)
project(BPMNParser)

# List of subprojects
set(SUBPROJECTS "BPMN")

# Set folder for the library to be created
set(LIB_DIR "lib" CACHE STRING "Library directory")
message("-- Library will be created in folder: ${LIB_DIR}")

# Allow users to specify parameters for building an executable, e.g.: 
# cmake -DMAIN=main.cpp -DEXE=bpmnParser ..
# If nothing is specified, only the library will be built
set(MAIN "" CACHE STRING "Filename for main")
set(EXE "" CACHE STRING "Filename for executable")

if (MAIN AND EXE)
  message("-- Executable '${EXE}' will be created from file: ${MAIN}")
endif()

# Set output directories
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/${LIB_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR})

# Create output directory for the library
file(MAKE_DIRECTORY ${LIB_DIR})

# Create lists of all headers and sources
set(HEADERS "XMLObject.h")
set(SOURCES "XMLObject.cpp")
foreach(SUBPROJECT ${SUBPROJECTS})
  include(${SUBPROJECT}/CMakeLists.txt)
  foreach(HEADER ${${SUBPROJECT}_HEADERS})
    list(APPEND HEADERS ${HEADER})
  endforeach()
  foreach(SOURCE ${${SUBPROJECT}_SOURCES})
    list(APPEND SOURCES ${SOURCE})
  endforeach()
endforeach()

find_program(CCACHE_FOUND "ccache")
set(CCACHE_SUPPORT ON CACHE BOOL "Enable ccache support")
if (CCACHE_FOUND AND CCACHE_SUPPORT)
  set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "ccache")
  set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK "ccache")
endif()

# Generate the single header file
set(SINGLE_HEADER_PATH "${LIB_DIR}/${CMAKE_PROJECT_NAME}.h")
file(WRITE ${SINGLE_HEADER_PATH} "// Automatically generated single header file\n")

foreach(HEADER ${HEADERS})
  file(READ ${HEADER} HEADER_CONTENTS)
  string(REGEX REPLACE "#include \".*\"\n" "" HEADER_CONTENTS "${HEADER_CONTENTS}")
  file(APPEND ${SINGLE_HEADER_PATH} "${HEADER_CONTENTS}")
endforeach()


# Set the C++ standard
set(CMAKE_CXX_STANDARD 20)

# Create the object library
set(LIB "${CMAKE_PROJECT_NAME}")
set(OBJECTLIB "${LIB}_OBJECTS")
add_library(${OBJECTLIB} OBJECT ${SOURCES})

# Set optimization flags for the target based on the compiler
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  target_compile_options(${OBJECTLIB} PRIVATE -O3)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  target_compile_options(${OBJECTLIB} PRIVATE -O3)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  target_compile_options(${OBJECTLIB} PRIVATE /Ox)
endif()

# Create the library
add_library(${LIB} STATIC $<TARGET_OBJECTS:${OBJECTLIB}>)

# Create the executable
if (MAIN AND EXE)
  add_executable(${EXE} ${MAIN} $<TARGET_OBJECTS:${OBJECTLIB}>)
  target_link_libraries(${EXE} ${LIB} xerces-c)
endif()
