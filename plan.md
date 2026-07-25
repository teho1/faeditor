# Roland FA-06 / FA-07 / FA-08 Studio Set Editor

## Goal

Create a cross-platform desktop application (Qt 6 / C++) for editing Roland FA Studio Sets over USB MIDI (SysEx).

The application is intended to replace Roland's obsolete AIR-based Studio Set Editor.

MVP only edits Studio Sets.

No Tone editing in the first version.

---

# Technology

- Qt 6
- C++20
- CMake
- RtMidi (preferred) or native platform MIDI APIs
- JSON project storage
- Optional YAML export

Supported OS:

- Windows
- macOS
- Linux

---

# Architecture

```
GUI
 │
 │ Qt Signals
 ▼
StudioSetModel
 │
 ▼
Roland SysEx Layer
 │
 ▼
RtMidi
 │
 ▼
Roland FA USB MIDI
```

---

# MVP Features

## MIDI

- Enumerate MIDI devices
- Connect
- Disconnect
- Detect FA automatically

---

## Studio Set

Read complete Studio Set from keyboard.

Write complete Studio Set.

Read individual parameter.

Write individual parameter.

Support realtime editing.

---

## Parts

Display all 16 Parts simultaneously.

Columns:

- Part Number
- Name
- Tone
- Bank
- Program
- MIDI Channel
- Volume
- Pan
- Reverb Send
- Chorus Send
- Octave Shift
- Transpose
- Keyboard Switch
- Receive Switch
- Output Assign

---

## Keyboard View

Visual keyboard.

Editable:

- Key Range Low
- Key Range High
- Velocity Low
- Velocity High

Drag handles.

---

## Mixer

16-channel mixer.

Realtime sliders:

- Volume
- Pan
- Mute
- Solo

---

## Effects

Studio Set Common:

- Chorus
- Reverb
- Master EQ
- Master Compressor

Only expose common parameters in MVP.

---

## Tone Browser

Search box.

Filters:

- Piano
- EP
- Organ
- Strings
- Brass
- Synth
- Bass
- Guitar
- Pad
- Drum

Preview immediately.

---

## Favorites

Store favorite tones.

JSON file.

---

## Studio Set Library

Save

Load

Duplicate

Rename

Delete

Compare

Export

Import

---

## Undo / Redo

Unlimited.

---

## Auto Save

Every edit.

Crash recovery.

---

# UI

Main Window

```
--------------------------------------------------------
Menu
Toolbar
--------------------------------------------------------

Studio Set

--------------------------------------------------------

Parts

1 Piano
2 Strings
3 Bass
...

--------------------------------------------------------

Selected Part

Tone

Volume

Pan

Transpose

Octave

Keyboard Range

Velocity Range

--------------------------------------------------------

Mixer

--------------------------------------------------------

Effects

--------------------------------------------------------
```

---

# SysEx Layer

Provide generic API.

```
read(address,size)

write(address,data)

checksum(data)

buildPacket()

parsePacket()
```

No GUI logic inside MIDI layer.

---

# Project Format

```
project.json

{
    studioSet : ...
    favorites : ...
    comments :
    created :
    modified :
}
```

---

# Stretch Goals

## Tone Editor

Not in MVP.

Future:

- Partial editor
- Filter
- Envelope
- LFO
- TVA
- TVF
- Matrix

---

## Live Mode

Instant scene switching.

---

## Compare Mode

Highlight changed parameters.

---

## Randomizer

Randomize:

- Pads
- Strings
- Synths

---

## Patch Search

Global search.

---

## Drag & Drop

Copy Part between Studio Sets.

---

## Logic Integration

Store Studio Set together with Logic project.

---

## Documentation Generator

Export Studio Set as Markdown or PDF.

---

# Important

Do NOT reverse engineer Roland binary files.

Only communicate using documented Roland MIDI SysEx messages.

Use address-based reads/writes.

Everything should work in realtime.

Never require reboot.

---

# Nice to Have

Dark Mode

Keyboard shortcuts

Resizable mixer

Dockable panels

Multiple Studio Sets open simultaneously

Recent Projects

Autoscan MIDI devices

```

# Official documentation

## Roland support

https://www.roland.com/global/support/by_product/fa-08/owners_manuals/

Contains:

- Reference Manual
- Parameter Guide
- MIDI Implementation
- Sound List
- DAW Workflow Guide

## MIDI Implementation

https://static.roland.com/assets/media/pdf/FA-06_07_08_MIDI_Imple_eng01_W.pdf

Primary reference.

Contains:

- SysEx protocol
- Address map
- Studio Set structure
- Checksums
- Read / Write messages

## Parameter Guide

Describes every Studio Set parameter.

## Reference Manual

Describes user workflow and Studio Set behaviour.

---

# Development order

1. MIDI connection
2. Read Studio Set
3. Write Studio Set
4. 16-part mixer
5. Tone browser
6. Keyboard ranges
7. Effects
8. Save/Load
9. Undo
10. Polish

