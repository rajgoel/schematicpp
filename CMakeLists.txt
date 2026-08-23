# File:   CMakeLists.txt
# Authors: Asvin Goel (rajgoel)
#
# LICENSE:
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

cmake_minimum_required(VERSION 3.26.4)
project(schematic++ VERSION 0.8.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)

# Only when schematic++ is built on its own, so that a consumer's choice always wins.
if(PROJECT_IS_TOP_LEVEL)
  if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
  endif()
  set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
endif()

# Xerces-C is declared rather than resolved from the system, so that a build succeeds on a machine with
# nothing installed. An installed copy satisfying the requirement is used and nothing is built; otherwise
# the pinned version is fetched and built.
include(FetchContent)
FetchContent_Declare(XercesC
  GIT_REPOSITORY https://github.com/apache/xerces-c.git
  GIT_TAG        v3.2.4
  FIND_PACKAGE_ARGS 3.2
)
FetchContent_MakeAvailable(XercesC)

# The two resolution paths name the target differently: CMake's FindXercesC module yields
# XercesC::XercesC, while building Xerces from source defines xerces-c.
if(TARGET XercesC::XercesC)
  set(XERCES_TARGET XercesC::XercesC)
  message(STATUS "schematic++: using an already-installed Xerces-C")
else()
  set(XERCES_TARGET xerces-c)
  message(STATUS "schematic++: building Xerces-C from source")
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR})

set(OUTPUT_BASE_CLASS "${PROJECT_SOURCE_DIR}/src/BaseClass.h")

# Convert output class to file with respective string literals
file(READ "${PROJECT_SOURCE_DIR}/lib/XMLObject.h" XMLObject_H)
file(READ "${PROJECT_SOURCE_DIR}/lib/XMLObject.cpp" XMLObject_CPP)
file(WRITE "${OUTPUT_BASE_CLASS}" "// schematic++ v${PROJECT_VERSION}\n\n")
file(APPEND "${OUTPUT_BASE_CLASS}" "const char *XMLObject_H =\n")
file(APPEND "${OUTPUT_BASE_CLASS}" "R\"\"\"(// schematic++ v${PROJECT_VERSION}\n")
file(APPEND "${OUTPUT_BASE_CLASS}" "${XMLObject_H})\"\"\";\n")
file(APPEND "${OUTPUT_BASE_CLASS}" "const char *XMLObject_CPP =\n")
file(APPEND "${OUTPUT_BASE_CLASS}" "R\"\"\"(${XMLObject_CPP})\"\"\";\n")

add_executable(schematic++
    src/main.cpp
    src/XercesString.cpp
    src/Class.cpp
    src/BuiltInClasses.cpp
)

# A consumer names the same target whether schematic++ was built in its tree or resolved from a prefix.
# The namespace is the package name, the target the artefact it provides.
add_executable(schematicpp::schematic++ ALIAS schematic++)

target_link_libraries(schematic++ PRIVATE ${XERCES_TARGET})

# VERSION was previously set with add_definitions and CMAKE_PROJECT_VERSION, which names the top-level
# project's version. As a subproject of a consumer that would have been the consumer's version.
target_compile_definitions(schematic++ PRIVATE VERSION="${PROJECT_VERSION}")

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

# The executable is exported so that a consumer imports a target and runs it through $<TARGET_FILE:...>
# rather than searching a path and hoping the version is right.
install(TARGETS schematic++
  EXPORT schematicpp-targets
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(EXPORT schematicpp-targets
  FILE      schematicpp-targets.cmake
  NAMESPACE schematicpp::
  DESTINATION ${CMAKE_INSTALL_DATADIR}/cmake/schematicpp
)

configure_package_config_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/cmake/schematicppConfig.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/schematicppConfig.cmake
  INSTALL_DESTINATION ${CMAKE_INSTALL_DATADIR}/cmake/schematicpp
)

# schematic++ is at 0.8.0, below 1.0.0, so compatibility is SameMinorVersion: a consumer requiring 0.8.0
# accepts any later 0.8.x and refuses 0.9.0.
write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/schematicppConfigVersion.cmake
  VERSION       ${PROJECT_VERSION}
  COMPATIBILITY SameMinorVersion
)

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/schematicppConfig.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/schematicppConfigVersion.cmake
  DESTINATION ${CMAKE_INSTALL_DATADIR}/cmake/schematicpp
)

