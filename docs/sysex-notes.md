# FA Editor SysEx Notes

Primary reference: Roland FA-06/07/08 MIDI Implementation (local copy; not in this repo)

## Framing

| Field | Value |
|-------|-------|
| Manufacturer | `41` (Roland) |
| Device ID | `10`–`1F` (default `10`) |
| Model ID | `00 00 77` |
| RQ1 | `11` |
| DT1 | `12` |

RQ1: `F0 41 dev 00 00 77 11 aa bb cc dd ss tt uu vv sum F7`  
DT1: `F0 41 dev 00 00 77 12 aa bb cc dd data… sum F7`

Checksum over address+size (RQ1) or address+data (DT1):  
`(128 - (sum % 128)) % 128`

Example (Reverb Type = Room 2):  
`F0 41 10 00 00 77 12 18 00 02 01 02 63 F7`

## Temporary Studio Set `18 00 00 00`

| Offset | Block |
|--------|-------|
| `00 00 00` | Common (name at `00`–`0F`, Solo Part at `39`) |
| `00 01 00` | Chorus |
| `00 02 00` | Reverb (Type at `01`) |
| `00 05 00` | Master Comp |
| `00 20 00`–`00 2F 00` | Parts 1–16 (size `4C`) |
| `00 40 00`–`00 4F 00` | Zones 1–16 (key range / Keyboard Switch) |

Packets larger than 256 bytes must be split with ~20 ms spacing.

## Temporary vs User Studio Sets

| Action | What happens |
|--------|----------------|
| **Open** a Studio Set in the app | Setup recall (`01 00 00 04`) loads User/Preset into Temporary, then Pull |
| **Live edits** (tone, level, mute…) | Immediate DT1 to Temporary only |
| **Push Temp** | Rewrites Temporary part/zone/FX blocks from the editor |
| **Write on the FA** | Copies Temporary → a User Studio Set slot (not available via SysEx) |

There is no SysEx “store to User.” Push never replaces a User slot by itself.

## Connection

Use FA USB MIDI music ports. Ignore `DAW CTRL` / Mackie Control ports.

## Known hardware limitation

The following FA system audio-routing parameters are not exposed in the
documented FA-06/07/08 MIDI SysEx address map and cannot currently be
controlled by the editor:

- USB Audio Output Select
- USB Audio Input Destination
- USB Driver Mode

The UI may display instructions or workflow presets for these settings,
but the user must change them manually on the FA hardware.

### Where to set them on the FA

- **USB Driver Mode** — MENU → System → USB Driver (e.g. VENDER / GENERIC). Reboot after changing.
- **USB Audio Output Select** / **USB Audio Input Destination** — MENU → System → **[3] System Effects** → **USB Audio** tab. Use System Write on the FA if you want the choice to persist.
