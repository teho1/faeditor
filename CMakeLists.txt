cmake_minimum_required(VERSION 3.21)
project(FAEditor VERSION 1.0.0 LANGUAGES CXX)

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

qt_add_executable(FAEditor
    src/main.cpp
    src/app/AppController.cpp
    src/midi/MidiDeviceModel.cpp
    src/midi/SysexEngine.cpp
    src/midi/RolandChecksum.cpp
    src/midi/AddressMap.cpp
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
    src/model/TemporaryToneModel.cpp
    src/model/MfxPresetStore.cpp
    src/project/ProjectStore.cpp
    src/undo/UndoController.cpp
)

set_source_files_properties(qml/theme/LogicTheme.qml PROPERTIES QT_QML_SINGLETON_TYPE TRUE)
set_source_files_properties(qml/theme/FaIcons.qml PROPERTIES QT_QML_SINGLETON_TYPE TRUE)

qt_add_qml_module(FAEditor
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

target_include_directories(FAEditor PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${rtmidi_SOURCE_DIR}
)

target_link_libraries(FAEditor PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::Svg
    Qt6::Concurrent
    rtmidi
)

if(APPLE)
    if(NOT CMAKE_OSX_DEPLOYMENT_TARGET)
        set(CMAKE_OSX_DEPLOYMENT_TARGET "12.0" CACHE STRING "Minimum macOS version" FORCE)
    endif()

    set(FAEDITOR_ICNS "${CMAKE_CURRENT_SOURCE_DIR}/resources/icons/AppIcon.icns")
    set_source_files_properties(${FAEDITOR_ICNS} PROPERTIES
        MACOSX_PACKAGE_LOCATION Resources
    )
    target_sources(FAEditor PRIVATE ${FAEDITOR_ICNS})

    set(FAEDITOR_ENTITLEMENTS "${CMAKE_CURRENT_SOURCE_DIR}/cmake/FAEditor.entitlements")
    set_target_properties(FAEditor PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_BUNDLE_NAME "FA Editor"
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.righthere.faeditor"
        MACOSX_BUNDLE_BUNDLE_VERSION 3
        MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION}
        MACOSX_BUNDLE_ICON_FILE "AppIcon"
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
    target_link_libraries(FAEditor PRIVATE
        ${COREMIDI_LIBRARY}
        ${COREAUDIO_LIBRARY}
        ${COREFOUNDATION_LIBRARY}
    )

    # Bundle Qt into the .app on every Release build / Xcode Archive (required for TestFlight).
    find_program(MACDEPLOYQT_EXECUTABLE macdeployqt
        HINTS "${Qt6_DIR}/../../../bin" "${CMAKE_PREFIX_PATH}/bin"
    )
    if(MACDEPLOYQT_EXECUTABLE)
        add_custom_command(TARGET FAEditor POST_BUILD
            COMMAND "${CMAKE_COMMAND}"
                "-DAPP=$<TARGET_BUNDLE_DIR:FAEditor>"
                "-DCONFIG=$<CONFIG>"
                "-DMACDEPLOYQT=${MACDEPLOYQT_EXECUTABLE}"
                "-DQMLDIR=${CMAKE_CURRENT_SOURCE_DIR}/qml"
                -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/DeployQt.cmake"
            COMMENT "macdeployqt (bundle Qt for App Store / TestFlight)"
            VERBATIM
        )
        # Keep a manual target as well.
        add_custom_target(FAEditor_deploy
            COMMAND "${CMAKE_COMMAND}"
                "-DAPP=$<TARGET_BUNDLE_DIR:FAEditor>"
                "-DCONFIG=Release"
                "-DMACDEPLOYQT=${MACDEPLOYQT_EXECUTABLE}"
                "-DQMLDIR=${CMAKE_CURRENT_SOURCE_DIR}/qml"
                -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/DeployQt.cmake"
            DEPENDS FAEditor
            COMMENT "Running macdeployqt"
            VERBATIM
        )
    else()
        message(WARNING "macdeployqt not found; Release/Archive builds will be incomplete for TestFlight")
    endif()
endif()

enable_testing()
qt_add_executable(FAEditorTests
    tests/test_roland.cpp
    src/midi/RolandChecksum.cpp
    src/midi/AddressMap.cpp
)
target_include_directories(FAEditorTests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
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
