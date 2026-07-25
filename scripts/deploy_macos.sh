#!/usr/bin/env bash
# Bundle Qt frameworks/plugins into FAEditor.app for App Store / TestFlight.
# Usage:
#   ./scripts/deploy_macos.sh [path/to/FAEditor.app]
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
QT_PREFIX="${CMAKE_PREFIX_PATH:-/Volumes/Datastore/Qt/6.11.1/macos}"
MACDEPLOYQT="${QT_PREFIX}/bin/macdeployqt"
APP="${1:-}"

if [[ -z "${APP}" ]]; then
  if [[ -d "${ROOT}/build-xcode/Release/FAEditor.app" ]]; then
    APP="${ROOT}/build-xcode/Release/FAEditor.app"
  elif [[ -d "${ROOT}/build/FAEditor.app" ]]; then
    APP="${ROOT}/build/FAEditor.app"
  else
    echo "FAEditor.app not found. Build Release first, or pass the .app path." >&2
    exit 1
  fi
fi

if [[ ! -x "${MACDEPLOYQT}" ]]; then
  echo "macdeployqt not found at ${MACDEPLOYQT}" >&2
  echo "Set CMAKE_PREFIX_PATH to your Qt macos kit." >&2
  exit 1
fi

echo "Deploying into: ${APP}"
echo "Before: $(du -sh "${APP}" | awk '{print $1}')"

"${MACDEPLOYQT}" "${APP}" \
  -qmldir="${ROOT}/qml" \
  -qmlimport="${QT_PREFIX}/qml" \
  -appstore-compliant \
  -verbose=1

echo "After:  $(du -sh "${APP}" | awk '{print $1}')"
if [[ ! -d "${APP}/Contents/Frameworks" ]]; then
  echo "ERROR: Contents/Frameworks missing after macdeployqt" >&2
  exit 1
fi
echo "Frameworks present. Re-sign/archive in Xcode (Product → Archive) and upload that build."
