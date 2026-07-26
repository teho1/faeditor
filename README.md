# FA Editor

macOS Studio Set editor for Roland FA-06 / FA-07 / FA-08.

Qt 6.11 QML UI + C++ models/controllers over documented Roland SysEx.

## Requirements

- macOS
- CMake ≥ 3.21
- Qt 6.11.1 at `/Volumes/Datastore/Qt/6.11.1/macos` (override with `CMAKE_PREFIX_PATH`)
- Xcode command-line tools
- Network once (Fetches [RtMidi](https://github.com/thestk/rtmidi))

## Build

```bash
cmake -S . -B build -DCMAKE_PREFIX_PATH=/Volumes/Datastore/Qt/6.11.1/macos
cmake --build build -j
ctest --test-dir build --output-on-failure
open build/FAEditor.app
```

## Usage

1. On the FA: System → USB Driver → **Vender (MIDI+Audio)**; install Roland USB driver if needed.
2. In the app: **MIDI → Auto-connect FA** (or pick ports — skip DAW CTRL).
3. Tab **1. Sets & Tones**: pick a User/Preset slot from the **FA Set** dropdown (top bar) to recall it, select a part (left), click a tone to assign (right; icon previews). Optional **Scan** for User names. Local projects are in the **Library** tab.
4. **Push Temp** if you want a full Temporary rewrite. To keep a **User** slot permanently, use **Write** on the FA itself (SysEx only edits Temporary).

## Layout

- `src/` — C++ MIDI, models, project store, undo
- `qml/` — Logic-inspired dark UI
- `resources/tones/soundlist.json` — FA-06/07/08 preset tone catalog from Roland Sound List (`scripts/import_soundlist.py`)
- `roland_specs/` — official PDFs (reference only)

## Notes

- Studio Sets only — no tone editor in this version.
- SysEx only — no proprietary binary reverse engineering.
- FA exposes Studio Set *data* only as Temporary (`18 00 00 00`); User/Preset slots are recalled via Setup Bank Select (`01 00 00 04`).

## Privacy

See [`PRIVACY.md`](PRIVACY.md). The app does not collect personal data.

## License

FA Editor source code is licensed under the **MIT License** — see [`LICENSE`](LICENSE).

Third-party components (Qt, RtMidi, Font Awesome Free, Material Icons, and
Roland reference documentation) are attributed in
[`THIRD_PARTY_NOTICES.md`](THIRD_PARTY_NOTICES.md).
