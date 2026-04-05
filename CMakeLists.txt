cmake_minimum_required(VERSION 3.16)
project(penguinsnap VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(ECM REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Gui Widgets DBus)
find_package(KF6StatusNotifierItem REQUIRED)
find_package(KF6GlobalAccel REQUIRED)
find_package(LayerShellQt REQUIRED)

find_package(PkgConfig REQUIRED)
pkg_check_modules(TESSERACT REQUIRED tesseract)
pkg_check_modules(LEPTONICA REQUIRED lept)

include(KDEInstallDirs)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(KDECMakeSettings)

if(NOT APP_VERSION)
    execute_process(
        COMMAND git describe --tags --always
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE APP_VERSION
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
endif()
if(NOT APP_VERSION)
    set(APP_VERSION "unknown")
endif()
add_compile_definitions(APP_VERSION="${APP_VERSION}")

set(SOURCES
    src/main.cpp
    src/application.cpp
    src/screenshotmanager.cpp
    src/ocrmanager.cpp
    src/outputmanager.cpp
    src/settingsmanager.cpp
    src/hotkeymanager.cpp
    src/preferencesdialog.cpp
    src/selectionoverlay.cpp
    src/windowoverlay.cpp
    resources/resources.qrc
)

add_executable(penguinsnap ${SOURCES})

target_include_directories(penguinsnap PRIVATE
    ${TESSERACT_INCLUDE_DIRS}
    ${LEPTONICA_INCLUDE_DIRS}
)

target_link_libraries(penguinsnap
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::DBus
    KF6::StatusNotifierItem
    KF6::GlobalAccel
    LayerShellQt::Interface
    ${TESSERACT_LIBRARIES}
    ${LEPTONICA_LIBRARIES}
)

install(TARGETS penguinsnap DESTINATION ${KDE_INSTALL_BINDIR})
install(FILES resources/penguinsnap.desktop DESTINATION ${KDE_INSTALL_APPDIR})
install(FILES resources/icons/camera-black-icon.svg DESTINATION ${KDE_INSTALL_ICONDIR}/hicolor/scalable/apps RENAME penguinsnap.svg)
install(FILES resources/icons/camera-white-icon.svg DESTINATION ${KDE_INSTALL_ICONDIR}/hicolor/scalable/apps RENAME penguinsnap-light.svg)
install(FILES LICENSE DESTINATION ${KDE_INSTALL_DATADIR}/licenses/penguinsnap)
