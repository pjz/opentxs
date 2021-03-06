# Copyright (c) 2018 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
set(cxx-sources
  Accounts.cpp
  Bip47Channels.cpp
  BlockchainTransactions.cpp
  Contacts.cpp
  Contexts.cpp
  Credentials.cpp
  Issuers.cpp
  Node.cpp
  Mailbox.cpp
  Nym.cpp
  Nyms.cpp
  PaymentWorkflows.cpp
  PeerReplies.cpp
  PeerRequests.cpp
  Root.cpp
  Seeds.cpp
  Servers.cpp
  Thread.cpp
  Threads.cpp
  Tree.cpp
  Units.cpp
)

set(cxx-headers
  Accounts.hpp
  Bip47Channels.hpp
  BlockchainTransactions.hpp
  Contacts.hpp
  Contexts.hpp
  Credentials.hpp
  Issuers.hpp
  Node.hpp
  Mailbox.hpp
  Nym.hpp
  Nyms.hpp
  PaymentWorkflows.hpp
  PeerReplies.hpp
  PeerRequests.hpp
  Root.hpp
  Seeds.hpp
  Servers.hpp
  Thread.hpp
  Threads.hpp
  Tree.hpp
  Units.hpp
)

set(MODULE_NAME opentxs-storage-tree)
if(WIN32)
  # suppress warnings about exported internal symbols (global log stream objects)
  add_definitions(-DOTLOG_IMPORT)

  # suppress linker warnings about locallly defined imported symbols
  set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /ignore:4049 /ignore:4217")

  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../cmake/windll.rc.in
    ${CMAKE_CURRENT_BINARY_DIR}/module.rc
    @ONLY
  )

  # we have an extra Windows-specific file to compile
  add_library(${MODULE_NAME} OBJECT
    ${cxx-sources}
    ${cxx-headers}
    ${CMAKE_CURRENT_BINARY_DIR}/module.rc
  )
else()
  add_library(${MODULE_NAME} OBJECT
    ${cxx-sources}
    ${cxx-headers}
  )
endif()

set_property(TARGET ${MODULE_NAME} PROPERTY POSITION_INDEPENDENT_CODE 1)
set_lib_property(${MODULE_NAME})
