set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
find_package(Qt6 6.11 REQUIRED COMPONENTS Core Gui Quick QuickControls2 Concurrent)
qt_standard_project_setup(REQUIRES 6.5)
include(${CMAKE_CURRENT_LIST_DIR}/IosRtMidi.cmake)
qt_add_executable(FAConnectionTest
    src/ios/main.cpp src/ios/ConnectionProbe.cpp src/ios/ConnectionProbe.h
    src/midi/SysexEngine.cpp src/midi/RolandChecksum.cpp src/midi/AddressMap.cpp
    src/platform/AutoDetectRolandPlatform.cpp src/platform/RolandFAPlatform.cpp
    src/platform/RolandFantomPlatform.cpp)
target_include_directories(FAConnectionTest PRIVATE src)
target_link_libraries(FAConnectionTest PRIVATE Qt6::Core Qt6::Gui Qt6::Quick Qt6::QuickControls2 Qt6::Concurrent rtmidi)
qt_add_qml_module(FAConnectionTest URI FAConnectionTest VERSION 1.0 QML_FILES qml/ConnectionTest.qml)
set(FAEDITOR_IOS_TEAM "K8DLLLGA28" CACHE STRING "Apple development team")
set_target_properties(FAConnectionTest PROPERTIES
    MACOSX_BUNDLE TRUE
    MACOSX_BUNDLE_GUI_IDENTIFIER "com.righthere.faeditor.connectiontest"
    MACOSX_BUNDLE_BUNDLE_NAME "FA Connection Test"
    MACOSX_BUNDLE_SHORT_VERSION_STRING "0.1"
    MACOSX_BUNDLE_BUNDLE_VERSION "1"
    XCODE_ATTRIBUTE_CODE_SIGN_STYLE "Automatic"
    XCODE_ATTRIBUTE_DEVELOPMENT_TEAM "${FAEDITOR_IOS_TEAM}"
    XCODE_ATTRIBUTE_TARGETED_DEVICE_FAMILY "1,2")
