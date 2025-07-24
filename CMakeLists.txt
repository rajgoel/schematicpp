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

cmake_minimum_required(VERSION 3.12)
project(schematic++ VERSION 0.5.0)

add_definitions(-DVERSION="${CMAKE_PROJECT_VERSION}") 

set(CMAKE_CXX_STANDARD 17)

find_library(XERCES_LIB xerces-c)
if(NOT XERCES_LIB)
  message(FATAL_ERROR "xerces-c library not found.\nTo install run: sudo apt install libxerces-c-dev")
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR})

set(OUTPUT_BASE_CLASS "${PROJECT_SOURCE_DIR}/src/BaseClass.h")

# Convert output class to file with respective string literals
file(READ "${PROJECT_SOURCE_DIR}/lib/XMLObject.h" XMLObject_H)
file(READ "${PROJECT_SOURCE_DIR}/lib/XMLObject.cpp" XMLObject_CPP)
file(WRITE "${OUTPUT_BASE_CLASS}" "// schematic++ v${CMAKE_PROJECT_VERSION}\n\n")
file(APPEND "${OUTPUT_BASE_CLASS}" "const char *XMLObject_H =\n")
file(APPEND "${OUTPUT_BASE_CLASS}" "R\"\"\"(// schematic++ v${CMAKE_PROJECT_VERSION}\n")
file(APPEND "${OUTPUT_BASE_CLASS}" "${XMLObject_H})\"\"\";\n")
file(APPEND "${OUTPUT_BASE_CLASS}" "const char *XMLObject_CPP =\n")
file(APPEND "${OUTPUT_BASE_CLASS}" "R\"\"\"(${XMLObject_CPP})\"\"\";\n")

add_executable(schematic++
    src/main.cpp
    src/XercesString.cpp
    src/Class.cpp
    src/BuiltInClasses.cpp
)

target_link_libraries(schematic++ xerces-c)

install(TARGETS schematic++)

