# iPhone / iPad MIDI connection test

This standalone app reuses the editor's RtMidi/CoreMIDI transport and Roland
FA/FANTOM-0 protocol adapters. It does not launch the full editor or write any
instrument parameters. Each test opens the selected ports, requests identity,
reads the temporary set/scene name, and closes the ports. MIDI waits run off the
UI thread. Keep the app in the foreground while testing.

## Build and install

Requires Qt 6.11 for iOS, Xcode, and development signing for your device.

```sh
/Volumes/Datastore/Qt/6.11.1/ios/bin/qt-cmake -S . -B build-ios-probe -DFAEDITOR_CONNECTION_PROBE=ON
cmake --build build-ios-probe --config Debug --target FAConnectionTest -- -allowProvisioningUpdates
```

Open `build-ios-probe/FAEditor.xcodeproj`, choose FAConnectionTest and your iPhone,
then Run. Override `FAEDITOR_IOS_TEAM` at configure time for another signing team.
The bundle identifier is `com.righthere.faeditor.connectiontest`, separate from
the shipping editor. Normal macOS builds are unchanged.

## On the instrument and phone

1. On FA-06/07/08, choose System → USB Driver → GENERIC (MIDI only), save the
   system setting, and restart the instrument. Roland's instructions:
   https://support.roland.com/hc/en-us/articles/115004414746
2. Connect the instrument's USB COMPUTER port to the phone using a data-capable
   cable/appropriate USB adapter. For an iPhone 15, use its USB-C connection.
3. Open FA Connection Test and tap Refresh. Select the music MIDI input and
   output, avoiding DAW CTRL. Manual selection also supports MIDI interfaces
   whose names do not contain “Roland”.
4. Tap Test connection. A successful result reports Identity OK, PASS, and the
   current set/scene name. Port discovery alone does not prove communication.
5. Change the current set on the instrument and repeat: the returned name should
   follow it. Unplug/reconnect, Refresh, and repeat. Also try leaving the app and
   returning, then starting a new test.
6. If anything fails, Copy diagnostic log and send the text back with your
   instrument model and cable/adapter details.

The FA-specific USB setup above is not a verified FANTOM-0 setup guide. This
prototype can identify FANTOM-0 and read its scene name using the existing adapter.
