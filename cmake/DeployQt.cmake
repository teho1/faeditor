# Invoked as a POST_BUILD step for FAEditor (Xcode / Ninja).
# Bundles Qt frameworks so App Store / TestFlight builds are self-contained.

if(NOT DEFINED APP OR NOT DEFINED MACDEPLOYQT OR NOT DEFINED QMLDIR)
    message(FATAL_ERROR "DeployQt.cmake: APP, MACDEPLOYQT, and QMLDIR are required")
endif()

# Strip accidental quotes from -DAPP="..." / shell expansion.
string(REPLACE "\"" "" APP "${APP}")
string(REPLACE "\"" "" MACDEPLOYQT "${MACDEPLOYQT}")
string(REPLACE "\"" "" QMLDIR "${QMLDIR}")

if(NOT DEFINED CONFIG)
    set(CONFIG "Release")
endif()
string(REPLACE "\"" "" CONFIG "${CONFIG}")

# Only ship Qt for distribution configs (Archive uses Release).
if(NOT CONFIG STREQUAL "Release" AND NOT CONFIG STREQUAL "RelWithDebInfo" AND NOT CONFIG STREQUAL "MinSizeRel")
    message(STATUS "DeployQt: skip for CONFIG=${CONFIG}")
    return()
endif()

if(NOT EXISTS "${APP}")
    message(FATAL_ERROR "DeployQt: app bundle not found: ${APP}")
endif()

if(NOT EXISTS "${MACDEPLOYQT}")
    message(FATAL_ERROR "DeployQt: macdeployqt not found: ${MACDEPLOYQT}")
endif()

message(STATUS "DeployQt: macdeployqt → ${APP} (CONFIG=${CONFIG})")
execute_process(
    COMMAND "${MACDEPLOYQT}"
        "${APP}"
        "-qmldir=${QMLDIR}"
        -appstore-compliant
        -verbose=1
    RESULT_VARIABLE _deploy_rc
)
if(NOT _deploy_rc EQUAL 0)
    message(FATAL_ERROR "DeployQt: macdeployqt failed with code ${_deploy_rc}")
endif()

if(NOT EXISTS "${APP}/Contents/Frameworks")
    message(FATAL_ERROR "DeployQt: Contents/Frameworks missing after macdeployqt")
endif()

message(STATUS "DeployQt: done")
