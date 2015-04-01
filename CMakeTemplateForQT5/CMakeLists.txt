cmake_minimum_required(VERSION 2.8.4)

set(PROJECT_NAME helloCLion)

project(${PROJECT_NAME})

# -----------------------------------
# setup global paths
# -----------------------------------
set(PROJECT_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../)
get_filename_component(PROJECT_ROOT ${PROJECT_ROOT} ABSOLUTE)

# force the 'build' dir to be the bin dir, rather than the default project dir
set(CMAKE_CURRENT_BINARY_DIR ${PROJECT_ROOT}/build)
include(SourceFiles.cmake)

# -----------------------------------
# Qt stuff:
#   1. setup paths for libs,
#   2. detect packages,
#   3. handle generted files
# -----------------------------------
set(QT_CMAKE_PATH ${PROJECT_ROOT}/../libs/Qt/5.3/clang_64/lib/cmake)
get_filename_component(QT_CMAKE_PATH ${QT_CMAKE_PATH} ABSOLUTE)
set(Qt5Core_DIR ${QT_CMAKE_PATH}/Qt5Core)
set(Qt5Widgets_DIR ${QT_CMAKE_PATH}/Qt5Widgets)

find_package(Qt5Core REQUIRED)
find_package(Qt5Widgets REQUIRED)

include(CustQtHelper.cmake)
include_directories(${CUST_QT5_GENERATED_FILE_DIR})
cust_qt5_wrap_ui(PROJECT_ALL_UICCED ${PROJECT_FILES_QUI})
cust_qt5_wrap_rc(PROJECT_ALL_QRCCED ${PROJECT_FILES_QRC})
cust_qt5_wrap_cpp(${PROJECT_SOURCE_ROOT} PROJECT_ALL_MOCCED ${PROJECT_FILES_MOCCABLE})

# -----------------------------------
#   the project target
# -----------------------------------

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
#add_definitions("-std=c++11")
add_executable(${PROJECT_NAME}
    ${PROJECT_FILES_ALLRAW}
    ${PROJECT_ALL_UICCED}
    ${PROJECT_ALL_MOCCED}
    ${PROJECT_ALL_QRCCED}
    )

target_link_libraries(${PROJECT_NAME} Qt5::Core)
target_link_libraries(${PROJECT_NAME} Qt5::Widgets)

set(PROJECT_OUTPUT ${PROJECT_ROOT}/build)

set_target_properties(${PROJECT_NAME} PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_OUTPUT}
    LIBRARY_OUTPUT_DIRECTORY ${PROJECT_OUTPUT}
    RUNTIME_OUTPUT_DIRECTORY ${PROJECT_OUTPUT}
    RUNTIME_OUTPUT_DIRECTORY_DEBUG ${PROJECT_OUTPUT}
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${PROJECT_OUTPUT}
    PROJECT_LABEL ${PROJECT_NAME}
    DEBUG_POSTFIX .d
   # OUTPUT_NAME ${PROJECT_NAME}.${CUSTDEF_ARCH_WINNAME}
    )
