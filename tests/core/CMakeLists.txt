# Copyright (c) 2018 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

set(name unittests-opentxs)

set(cxx-sources
  Test_Data.cpp
)

include_directories(
  ${PROJECT_SOURCE_DIR}/include
  ${GTEST_INCLUDE_DIRS}
)

add_subdirectory(crypto)

add_executable(${name} ${cxx-sources})
target_link_libraries(${name} opentxs ${GTEST_BOTH_LIBRARIES})
set_target_properties(${name} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/tests)
add_test(${name} ${PROJECT_BINARY_DIR}/tests/${name} --gtest_output=xml:gtestresults.xml)
