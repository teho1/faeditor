include(FetchContent)
# Upstream RtMidi 6 supports iOS, but its CMake target requires macOS CoreServices.
FetchContent_Declare(rtmidi
    GIT_REPOSITORY https://github.com/thestk/rtmidi.git GIT_TAG 6.0.0 GIT_SHALLOW TRUE
    SOURCE_SUBDIR faeditor-unused)
FetchContent_MakeAvailable(rtmidi)
add_library(rtmidi STATIC ${rtmidi_SOURCE_DIR}/RtMidi.cpp)
target_include_directories(rtmidi PUBLIC ${rtmidi_SOURCE_DIR})
target_compile_definitions(rtmidi PRIVATE __MACOSX_CORE__)
target_link_libraries(rtmidi PUBLIC "-framework CoreMIDI" "-framework CoreAudio" "-framework CoreFoundation")
if(NOT IOS)
    target_link_libraries(rtmidi PUBLIC "-framework CoreServices")
endif()
