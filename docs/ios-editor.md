# iOS editor development build

The full FA editor now has an iPhone/iPad entry point (`MobileMain.qml`) sharing
its C++ controllers, models, library and Roland protocol adapters with macOS.
The small connection tester remains available with `FAEDITOR_CONNECTION_PROBE=ON`.

## Build

```sh
/Volumes/Datastore/Qt/6.11.1/ios/bin/qt-cmake -S . -B build-ios-editor -DFAEDITOR_CONNECTION_PROBE=OFF
xcodebuild -project build-ios-editor/FAEditor.xcodeproj -scheme FAEditor -configuration Debug -destination 'generic/platform=iOS' -allowProvisioningUpdates build
```

Open that Xcode project and select your iPhone or **My Mac (Designed for iPad)**
to run. iOS uses the same bundle ID as macOS, `com.righthere.faeditor`. The
connection tester stays on `com.righthere.faeditor.connectiontest`. Signing
team defaults to the existing project team and can be overridden with
`FAEDITOR_IOS_TEAM`.

For rapid desktop layout checks, build the normal macOS target and launch its
executable with `--mobile-ui`. This uses the mobile QML with Fusion controls;
it does not reproduce UIKit, iOS file pickers, or iOS suspension behavior.

## Current mobile behavior

- Connect explicitly, then pull the current Studio Set. FA USB driver must be
  GENERIC (MIDI only), saved and applied with a keyboard restart.
- Sets: side-by-side parts and tones at tablet widths; a part picker on phones.
- Mixer: wider channel strips and touch targets; part controls in a drawer.
- Effects/Tone: shared editors, horizontally scrollable on narrow displays.
- Library: separate tab, using the app's sandbox storage.
- The action menu provides local save, temporary push, SVD import and exports.
- Lock/sleep saves local state and closes MIDI (iOS suspends USB MIDI). When
  you unlock, the app retries auto-connect and pull for a few seconds. Use
  Disconnect in the menu if you do not want that. Background MIDI is not
  enabled.

This is an initial device-test build. Full transfer operations still use the
existing synchronous model APIs and may pause interaction during a pull/push.
Native document import/export and a physical iPad still need device validation.
The full phone layout is secondary to the iPad layout; dense editors can scroll.

## Hardware acceptance checks

1. Connect FA-08 in GENERIC mode; verify the current set and all 16 part names.
2. Change a part level/pan and confirm the keyboard follows; pull to verify it.
3. Select a tone for one part and confirm the change on the keyboard.
4. Open Effects and Tone, verify their parameters match the selected part.
5. Save/load a local copy; test imported/exported documents through Files.
6. Disconnect/reconnect USB. Lock and unlock the phone: MIDI should come back
   without tapping Connect.
7. Repeat at iPad portrait and landscape sizes. Mac testing cannot verify touch
   accuracy, keyboard avoidance or physical iPad USB behavior.
