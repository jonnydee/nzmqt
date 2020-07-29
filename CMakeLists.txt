cmake_minimum_required(VERSION 3.8.2)

project(nzmqt VERSION 3.2.1 LANGUAGES CXX)

# Setup the QT5 path
set(Qt5_DIR /opt/qt/5.15.0/lib/cmake/Qt5 CACHE PATH "Path for QT cmake")

# Setup the CONFIG install path
include(GNUInstallDirs)
set(
    NZMQT_CONFIG_INSTALL_DIR
    "${CMAKE_INSTALL_DATADIR}/cmake/${PROJECT_NAME}" CACHE PATH
    "Install path for nzmqtConfig.cmake"
)

find_package(cppzmq REQUIRED CONFIG)
find_package(Qt5 REQUIRED COMPONENTS Core)

# Needed for QT to resolve Q_OBJECT methods on link
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

#######
# Build
#######
add_library(nzmqt SHARED "")

target_include_directories(
    nzmqt
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
    ${cppzmq_INCLUDE_DIR}
    ${Qt5Core_INCLUDE_DIRS}
)

target_compile_definitions(
    nzmqt
    PUBLIC
    ZMQ_BUILD_DRAFT_API
    NZMQT_LIB
    NZMQT_SHAREDLIB
)

target_sources(
    nzmqt
    PRIVATE
    src/nzmqt/nzmqt.cpp
    include/nzmqt/global.hpp
    include/nzmqt/impl.hpp
    include/nzmqt/nzmqt.hpp
)

target_link_libraries(
    nzmqt
    PUBLIC
    cppzmq
    Qt5::Core
)

#########
# Install
#########
include(CMakePackageConfigHelpers)

configure_package_config_file(
    ${PROJECT_NAME}Config.cmake.in
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
    INSTALL_DESTINATION ${NZMQT_CONFIG_INSTALL_DIR}
)

install(
    DIRECTORY include/nzmqt
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(
    TARGETS nzmqt
    EXPORT ${PROJECT_NAME}-targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
    DESTINATION ${NZMQT_CONFIG_INSTALL_DIR}
)

export(
    EXPORT ${PROJECT_NAME}-targets
    FILE "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake"
)

install(
    EXPORT ${PROJECT_NAME}-targets
    FILE ${PROJECT_NAME}Targets.cmake
    DESTINATION ${NZMQT_CONFIG_INSTALL_DIR}
)
