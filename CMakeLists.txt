cmake_minimum_required(VERSION 3.21)
project(FAEditor VERSION 1.2.0 LANGUAGES CXX)

set(FAEDITOR_PRODUCT "FA" CACHE STRING "Product to build (FA or FANTOM)")
set_property(CACHE FAEDITOR_PRODUCT PROPERTY STRINGS FA FANTOM)
string(TOUPPER "${FAEDITOR_PRODUCT}" FAEDITOR_PRODUCT)
include(cmake/Products.cmake)
faeditor_product_metadata(
    "${FAEDITOR_PRODUCT}"
    APP_TARGET APP_DISPLAY_NAME APP_BUNDLE_ID APP_VERSION APP_BUNDLE_VERSION APP_QML_URI APP_QML_MAIN
)
message(STATUS "FAEditor product: ${FAEDITOR_PRODUCT} (${APP_DISPLAY_NAME}, ${APP_BUNDLE_ID})")
if(FAEDITOR_PRODUCT STREQUAL "FA")
    set(APP_ORGANIZATION_DOMAIN "faeditor.local")
else()
    set(APP_ORGANIZATION_DOMAIN "fantomeditor.local")
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

if(NOT DEFINED CMAKE_PREFIX_PATH AND EXISTS "/Volumes/Datastore/Qt/6.11.1/macos")
    set(CMAKE_PREFIX_PATH "/Volumes/Datastore/Qt/6.11.1/macos" CACHE PATH "Qt prefix")
endif()

find_package(Qt6 6.11 REQUIRED COMPONENTS
    Core
    Gui
    Widgets
    Quick
    QuickControls2
    Svg
    Concurrent
    Test
)

qt_standard_project_setup(REQUIRES 6.5)

include(FetchContent)
FetchContent_Declare(
    rtmidi
    GIT_REPOSITORY https://github.com/thestk/rtmidi.git
    GIT_TAG        6.0.0
    GIT_SHALLOW    TRUE
)
set(RTMIDI_BUILD_STATIC_LIBS ON CACHE BOOL "" FORCE)
set(RTMIDI_BUILD_TESTING OFF CACHE BOOL "" FORCE)
set(RTMIDI_API_CORE ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(rtmidi)

set(COMMON_APP_SOURCES src/main.cpp)
if(FAEDITOR_PRODUCT STREQUAL "FA")
    list(APPEND COMMON_APP_SOURCES
    src/app/AppController.cpp
    src/midi/MidiDeviceModel.cpp
    src/midi/SysexEngine.cpp
    src/midi/RolandChecksum.cpp
    src/midi/AddressMap.cpp
    src/platform/RolandFAPlatform.cpp
    src/model/StudioSetModel.cpp
    src/model/PartModel.cpp
    src/model/EffectsModel.cpp
    src/model/ToneBrowserModel.cpp
    src/model/WaveformCatalog.cpp
    src/model/StudioSetBrowserModel.cpp
    src/model/AudioFxModel.cpp
    src/model/TemporarySysexStore.cpp
    src/model/MfxModel.cpp
    src/model/MfxParamCatalog.cpp
    src/model/MfxUiHelpers.cpp
    src/model/MfxTapDelayModel.cpp
    src/model/MfxParamListModel.cpp
    src/model/SnSynthToneModel.cpp
    src/model/PcmSynthToneModel.cpp
    src/model/SnAcousticInstCatalog.cpp
    src/model/SnAcousticToneModel.cpp
    src/model/SvdImportModel.cpp
    src/model/TemporaryToneModel.cpp
    src/model/MfxPresetStore.cpp
    src/project/ProjectStore.cpp
    src/undo/UndoController.cpp
    )
else()
    list(APPEND COMMON_APP_SOURCES src/app/FantomAppController.h)
endif()

qt_add_executable(${APP_TARGET} ${COMMON_APP_SOURCES})

set_source_files_properties(qml/theme/LogicTheme.qml PROPERTIES QT_QML_SINGLETON_TYPE TRUE)
set_source_files_properties(qml/theme/FaIcons.qml PROPERTIES QT_QML_SINGLETON_TYPE TRUE)

if(FAEDITOR_PRODUCT STREQUAL "FA")
    qt_add_qml_module(${APP_TARGET}
        URI FAEditor
        VERSION 1.0
        QML_FILES
        qml/Main.qml
        qml/theme/LogicTheme.qml
        qml/theme/FaIcons.qml
        qml/components/ChannelStrip.qml
        qml/components/Fader.qml
        qml/components/PanKnob.qml
        qml/components/Toolbar.qml
        qml/components/StatusBar.qml
        qml/components/FaIcon.qml
        qml/components/FaButton.qml
        qml/components/AdsrEnvelopeGraph.qml
        qml/components/FilterResponseGraph.qml
        qml/components/LfoWaveformGraph.qml
        qml/components/CompressorTransferGraph.qml
        qml/components/MfxEqGraph.qml
        qml/components/MatrixControlCards.qml
        qml/components/MfxParamFader.qml
        qml/components/MfxParamGrid.qml
        qml/components/MfxTapDelayEditor.qml
        qml/components/MfxGenericParameterEditor.qml
        qml/components/MfxDelayEditor.qml
        qml/components/MfxEQEditor.qml
        qml/components/MfxFilterEditor.qml
        qml/components/MfxModulationEditor.qml
        qml/components/MfxCompressorEditor.qml
        qml/components/MfxAmpEditor.qml
        qml/components/MfxRotaryEditor.qml
        qml/components/MfxPipelineEditor.qml
        qml/components/MfxSlicerEditor.qml
        qml/views/MixerView.qml
        qml/views/PartInspector.qml
        qml/views/ZoneKeyboard.qml
        qml/views/EffectsPanel.qml
        qml/views/EffectsEditView.qml
        qml/views/ToneEditView.qml
        qml/views/MidiConnectDialog.qml
        qml/views/ToneBrowser.qml
        qml/views/WaveformBrowser.qml
        qml/views/StudioSetBrowser.qml
        qml/views/ChangeToneView.qml
        qml/views/AudioFxPanel.qml
        qml/views/LibraryView.qml
        qml/views/AboutDialog.qml
        qml/views/HelpDialog.qml
        qml/views/SvdImportPanel.qml
        RESOURCES
        resources/tones/soundlist.json
        resources/waves/waveforms.json
        resources/mfx_presets/presets.json
        resources/mfx_params/catalog.json
        resources/mfx_params/families.json
        resources/fonts/fa-solid-900.ttf
        resources/fonts/MaterialIcons-Regular.ttf
        resources/icons/mute.svg
        resources/icons/solo.svg
        resources/icons/midi.svg
        resources/icons/pull.svg
        resources/icons/push.svg
        resources/icons/save.svg
            resources/icons/appicon.png
    )
else()
    qt_add_qml_module(${APP_TARGET}
        URI FantomEditor
        VERSION 1.0
        QML_FILES
            qml/FantomMain.qml
            qml/theme/LogicTheme.qml
    )
endif()

target_include_directories(${APP_TARGET} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${rtmidi_SOURCE_DIR}
)

target_link_libraries(${APP_TARGET} PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::Svg
    Qt6::Concurrent
    rtmidi
)

target_compile_definitions(${APP_TARGET} PRIVATE
    FAEDITOR_PRODUCT_KEY="${FAEDITOR_PRODUCT}"
    FAEDITOR_ORGANIZATION_NAME="${APP_TARGET}"
    FAEDITOR_ORGANIZATION_DOMAIN="${APP_ORGANIZATION_DOMAIN}"
    FAEDITOR_APPLICATION_NAME="${APP_DISPLAY_NAME}"
    FAEDITOR_APPLICATION_VERSION="${APP_VERSION}"
    FAEDITOR_QML_URI="${APP_QML_URI}"
    FAEDITOR_QML_MAIN="${APP_QML_MAIN}"
)
if(FAEDITOR_PRODUCT STREQUAL "FANTOM")
    target_compile_definitions(${APP_TARGET} PRIVATE FAEDITOR_PRODUCT_FANTOM=1)
endif()

if(APPLE)
    if(NOT CMAKE_OSX_DEPLOYMENT_TARGET)
        set(CMAKE_OSX_DEPLOYMENT_TARGET "12.0" CACHE STRING "Minimum macOS version" FORCE)
    endif()

    set(FAEDITOR_ICNS "${CMAKE_CURRENT_SOURCE_DIR}/resources/icons/AppIcon.icns")
    set_source_files_properties(${FAEDITOR_ICNS} PROPERTIES
        MACOSX_PACKAGE_LOCATION Resources
    )
    if(FAEDITOR_PRODUCT STREQUAL "FA")
        target_sources(${APP_TARGET} PRIVATE ${FAEDITOR_ICNS})
    endif()

    set(FAEDITOR_ENTITLEMENTS "${CMAKE_CURRENT_SOURCE_DIR}/cmake/FAEditor.entitlements")
    set_target_properties(${APP_TARGET} PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_BUNDLE_NAME "${APP_DISPLAY_NAME}"
        MACOSX_BUNDLE_GUI_IDENTIFIER "${APP_BUNDLE_ID}"
        MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/cmake/MacOSXBundleInfo.plist.in"
        XCODE_ATTRIBUTE_CODE_SIGN_ENTITLEMENTS "${FAEDITOR_ENTITLEMENTS}"
        XCODE_ATTRIBUTE_CODE_SIGN_STYLE "Automatic"
        XCODE_ATTRIBUTE_DEVELOPMENT_TEAM "K8DLLLGA28"
        XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "Apple Development"
        XCODE_ATTRIBUTE_ENABLE_HARDENED_RUNTIME YES
        XCODE_ATTRIBUTE_ENABLE_APP_SANDBOX YES
    )
    find_library(COREMIDI_LIBRARY CoreMIDI)
    find_library(COREAUDIO_LIBRARY CoreAudio)
    find_library(COREFOUNDATION_LIBRARY CoreFoundation)
    if(FAEDITOR_PRODUCT STREQUAL "FA")
        # Keep the FA release bundle metadata explicit and regression-tested.
        set_target_properties(${APP_TARGET} PROPERTIES
            MACOSX_BUNDLE_BUNDLE_VERSION 5
            MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
            MACOSX_BUNDLE_ICON_FILE "AppIcon"
        )
    else()
        set_target_properties(${APP_TARGET} PROPERTIES
            MACOSX_BUNDLE_BUNDLE_VERSION "${APP_BUNDLE_VERSION}"
            MACOSX_BUNDLE_SHORT_VERSION_STRING "${APP_VERSION}"
        )
    endif()
    target_link_libraries(${APP_TARGET} PRIVATE
        ${COREMIDI_LIBRARY}
        ${COREAUDIO_LIBRARY}
        ${COREFOUNDATION_LIBRARY}
    )

    # Bundle Qt into the .app on every Release build / Xcode Archive (required for TestFlight).
    find_program(MACDEPLOYQT_EXECUTABLE macdeployqt
        HINTS "${Qt6_DIR}/../../../bin" "${CMAKE_PREFIX_PATH}/bin"
    )
    if(MACDEPLOYQT_EXECUTABLE)
        add_custom_command(TARGET ${APP_TARGET} POST_BUILD
            COMMAND "${CMAKE_COMMAND}"
                "-DAPP=$<TARGET_BUNDLE_DIR:${APP_TARGET}>"
                "-DCONFIG=$<CONFIG>"
                "-DMACDEPLOYQT=${MACDEPLOYQT_EXECUTABLE}"
                "-DQMLDIR=${CMAKE_CURRENT_SOURCE_DIR}/qml"
                -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/DeployQt.cmake"
            COMMENT "macdeployqt (bundle Qt for App Store / TestFlight)"
            VERBATIM
        )
        # Keep a manual target as well.
        add_custom_target(${APP_TARGET}_deploy
            COMMAND "${CMAKE_COMMAND}"
                "-DAPP=$<TARGET_BUNDLE_DIR:${APP_TARGET}>"
                "-DCONFIG=Release"
                "-DMACDEPLOYQT=${MACDEPLOYQT_EXECUTABLE}"
                "-DQMLDIR=${CMAKE_CURRENT_SOURCE_DIR}/qml"
                -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/DeployQt.cmake"
            DEPENDS ${APP_TARGET}
            COMMENT "Running macdeployqt"
            VERBATIM
        )
    else()
        message(WARNING "macdeployqt not found; Release/Archive builds will be incomplete for TestFlight")
    endif()
endif()

enable_testing()
add_test(NAME ProductConfigurationTests
    COMMAND ${CMAKE_COMMAND} -DSOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}
            -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_product_options.cmake)
qt_add_executable(FAEditorTests
    tests/test_roland.cpp
    src/midi/RolandChecksum.cpp
    src/midi/AddressMap.cpp
)
target_include_directories(FAEditorTests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_compile_definitions(FAEditorTests PRIVATE FAEDITOR_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(FAEditorTests PRIVATE Qt6::Core Qt6::Test)
add_test(NAME FAEditorTests COMMAND FAEditorTests)

qt_add_executable(FAEditorMfxTests
    tests/test_mfx_tap.cpp
    src/model/MfxModel.cpp
    src/model/MfxParamCatalog.cpp
    src/model/MfxUiHelpers.cpp
    src/model/MfxTapDelayModel.cpp
    src/model/MfxParamListModel.cpp
    src/midi/AddressMap.cpp
    src/midi/RolandChecksum.cpp
    src/midi/SysexEngine.cpp
    src/midi/MidiDeviceModel.cpp
)
target_include_directories(FAEditorMfxTests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${rtmidi_SOURCE_DIR}
)
target_link_libraries(FAEditorMfxTests PRIVATE Qt6::Core Qt6::Test rtmidi)
if(APPLE)
    target_link_libraries(FAEditorMfxTests PRIVATE
        ${COREMIDI_LIBRARY} ${COREAUDIO_LIBRARY} ${COREFOUNDATION_LIBRARY})
endif()
add_test(NAME FAEditorMfxTests COMMAND FAEditorMfxTests)

qt_add_executable(FAEditorPlatformTests
    tests/test_platform.cpp
    src/model/TemporarySysexStore.cpp
)
target_include_directories(FAEditorPlatformTests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src ${CMAKE_CURRENT_SOURCE_DIR}/tests)
target_link_libraries(FAEditorPlatformTests PRIVATE Qt6::Core Qt6::Test)
add_test(NAME FAEditorPlatformTests COMMAND FAEditorPlatformTests)

qt_add_executable(FAEditorAudioFxTests
    tests/test_audio_fx.cpp
    src/model/AudioFxModel.cpp
)
target_include_directories(FAEditorAudioFxTests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src ${CMAKE_CURRENT_SOURCE_DIR}/tests)
target_link_libraries(FAEditorAudioFxTests PRIVATE Qt6::Core Qt6::Test)
add_test(NAME FAEditorAudioFxTests COMMAND FAEditorAudioFxTests)

qt_add_executable(FAEditorMidiPlatformTests
    tests/test_midi_platform.cpp
    src/midi/MidiDeviceModel.cpp
)
target_include_directories(FAEditorMidiPlatformTests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src ${CMAKE_CURRENT_SOURCE_DIR}/tests)
target_link_libraries(FAEditorMidiPlatformTests PRIVATE Qt6::Core Qt6::Test)
add_test(NAME FAEditorMidiPlatformTests COMMAND FAEditorMidiPlatformTests)

qt_add_executable(FAEditorLibraryPerformanceTests
    tests/test_library_performance.cpp
    src/model/ToneBrowserModel.cpp
    src/model/WaveformCatalog.cpp
)
target_include_directories(FAEditorLibraryPerformanceTests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(FAEditorLibraryPerformanceTests PRIVATE Qt6::Core Qt6::Test)
add_test(NAME FAEditorLibraryPerformanceTests COMMAND FAEditorLibraryPerformanceTests)

qt_add_executable(FAEditorToneWorkflowTests
    tests/test_tone_workflow.cpp
    src/model/StudioSetModel.cpp src/model/PartModel.cpp src/model/EffectsModel.cpp
    src/model/TemporarySysexStore.cpp src/model/TemporaryToneModel.cpp
    src/model/MfxModel.cpp src/model/MfxParamCatalog.cpp src/model/MfxUiHelpers.cpp
    src/model/MfxTapDelayModel.cpp src/model/MfxParamListModel.cpp src/model/MfxPresetStore.cpp
    src/model/SnSynthToneModel.cpp src/model/PcmSynthToneModel.cpp
    src/model/SnAcousticToneModel.cpp src/model/SnAcousticInstCatalog.cpp
    src/model/WaveformCatalog.cpp src/undo/UndoController.cpp
    src/midi/SysexEngine.cpp src/midi/MidiDeviceModel.cpp
    src/midi/AddressMap.cpp src/midi/RolandChecksum.cpp
    src/platform/RolandFAPlatform.cpp
)
target_include_directories(FAEditorToneWorkflowTests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src ${rtmidi_SOURCE_DIR})
target_link_libraries(FAEditorToneWorkflowTests PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Test rtmidi)
if(APPLE)
    target_link_libraries(FAEditorToneWorkflowTests PRIVATE ${COREMIDI_LIBRARY} ${COREAUDIO_LIBRARY} ${COREFOUNDATION_LIBRARY})
endif()
add_test(NAME FAEditorToneWorkflowTests COMMAND FAEditorToneWorkflowTests)

qt_add_executable(FAEditorSvdImportTests
    tests/test_svd_import.cpp
    src/model/SvdImportModel.cpp src/model/StudioSetModel.cpp src/model/PartModel.cpp
    src/model/EffectsModel.cpp src/model/TemporarySysexStore.cpp
    src/model/MfxModel.cpp src/model/MfxParamCatalog.cpp
    src/model/MfxUiHelpers.cpp src/model/MfxTapDelayModel.cpp src/model/MfxParamListModel.cpp
    src/model/SnAcousticToneModel.cpp src/model/SnAcousticInstCatalog.cpp
    src/model/SnSynthToneModel.cpp src/model/WaveformCatalog.cpp
    src/undo/UndoController.cpp
)
target_include_directories(FAEditorSvdImportTests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src ${CMAKE_CURRENT_SOURCE_DIR}/tests)
target_link_libraries(FAEditorSvdImportTests PRIVATE Qt6::Core Qt6::Gui Qt6::Test)
add_test(NAME FAEditorSvdImportTests COMMAND FAEditorSvdImportTests)
