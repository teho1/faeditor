# Roland SVD1 research notes

This document records reverse-engineering observations used to develop offline
Roland FA backup import. It is not an official Roland file-format specification.
All multi-byte integers observed in the FA sample are big-endian.

![Observed SVD1 container layout](svd-format.svg)

## Container layout

The observed file starts with a 16-byte fixed header:

| Offset | Size | Meaning |
| --- | ---: | --- |
| `0x00` | 2 | Length of the bytes following this field up to the first data area |
| `0x02` | 4 | ASCII magic `SVD1` |
| `0x06` | 10 | Zero/reserved bytes |

The first data area therefore starts at `2 + headerLength`. The number of
16-byte data-area descriptors is `(2 + headerLength - 16) / 16`. Each descriptor
contains an 8-byte area identifier, a 4-byte absolute file offset, and a 4-byte
area length.

In the tested SVD1/MI73 file, each data area starts with another 16-byte header:

| Offset | Size | Meaning |
| --- | ---: | --- |
| `0x00` | 4 | Number of entries |
| `0x04` | 4 | Packed length of each entry |
| `0x08` | 4 | Offset from the area start to the first entry (observed: 16) |
| `0x0c` | 4 | Zero/reserved |

The entries follow consecutively at `areaOffset + entryOffset`.

This inner area header is **not universal across SVD versions**. A tested
SVD0/XP50 file begins its area payload immediately at the descriptor's offset;
interpreting its first 16 payload bytes as the SVD1 entry header produces invalid
counts and lengths.

## Observed FA backup

The research fixture `Tony.SVD` has SHA-256
`efac55b8942fb92ba949219e347f37ab77381a7ddde6609d163f28f6bd03a439`.
The fixture itself is private test data and is not committed.

| Area | Entries | Packed bytes/entry | Current interpretation |
| --- | ---: | ---: | --- |
| `PRFbMI73` | 512 | 1460 | Studio Set |
| `RFPaMI73` | 256 | 590 | PCM Synth Tone |
| `RFRaMI73` | 32 | 10890 | PCM Drum Kit |
| `SHPaMI73` | 512 | 280 | SuperNATURAL Synth Tone |
| `SNTaMI73` | 128 | 138 | SuperNATURAL Acoustic Tone |
| `SDKaMI73` | 8 | 1006 | SuperNATURAL Drum Kit |
| `SYSaMI73` | 1 | 128 | System data |
| `SYNaMI73` | 1 | 21 | Unknown; name retained pending verification |
| `VCa\0MI73` | 1 | 17 | Unknown |
| `VISaMI73` | 100 | 11 | Unknown |
| `MCGaMI73` | 1 | 17 | Unknown |
| `MCCaMI73` | 16 | 26 | Unknown |
| `PRXaMI73` | 512 | 392 | Unknown |

## SVD0 comparison fixture

The private comparison file `DANCEKIT.SVD` has SHA-256
`d5369e19197cebb5037fc7dc4e1bbc77382d08e5297e8dcc3fe393a0c0412dca`
and identifies itself as `SVD0` / `XP50`. It confirms the same outer header and
16-byte descriptor table, with four areas:

| Area | Absolute offset | Area length |
| --- | ---: | ---: |
| `PRFaXP50` | `0x00050` | `0x02280` |
| `PATaXP50` | `0x022d0` | `0x0c580` |
| `RHYaXP50` | `0x0e850` | `0x01516` |
| `SYSaXP50` | `0x0fd66` | `0x000ff` |

Its payload layout differs from SVD1 and is outside the FA import scope. This
fixture is useful for ensuring that a parser validates the magic/model and never
blindly applies MI73 rules to another Roland product.

The tone-area interpretations agree with the independently reported Integra-7
`MI69` structures, but the FA `MI73` parameter layouts must still be verified
against the FA MIDI implementation before writing device data.

## Packed entries

Entries are a continuous, most-significant-bit-first bit stream. The first 12
fields of the tested FA tone entries are 7-bit ASCII characters. For example,
the first entries decode as:

- `SNTaMI73`: `Full Grand 1`
- `SHPaMI73`: `KidSawLdPoly`
- `RFPaMI73`: `My128voicPEQ`

The remaining parameters use varying bit widths and are not byte-aligned. The
reported logical composition is:

- PCM Synth: Common + Common MFX + PMT + 4 Partials + Common 2
- PCM Drum: Common + Common MFX + Common Comp/EQ + 88 Partials (Common 2 was
  reported missing in the Integra-7 research)
- SN Synth: Common + Common MFX + 3 Partials + an undocumented block
- SN Acoustic: Common + MFX
- SN Drum: Common + MFX + Common Comp/EQ + 62 Notes + an additional EQ block

These compositions are research leads, not yet a safe decoding schema. FAEditor
must reject a packed entry unless every field boundary, range, output block size,
and target engine has been validated. Import work must initially target Temporary
Tone memory only; permanent User writes remain an explicit operation on the FA.

## References

- [Audiofanzine discussion: Fichier SVD](https://fr.audiofanzine.com/workstation/roland/Fantom-X6/forums/t.107849,fichier-svd.html) — original container diagram and MI69 structure research by forum user Gerbilles.
- [JDTools](https://github.com/sagamusix/JDTools) — BSD-licensed C++ conversion code for newer Roland SVD5/SVZ/BIN formats.
- [JD08PatchManager](https://github.com/NilsKr/JD08PatchManager) — GPL-licensed Python tooling for copying JD-08/JX-08 SVD5 entries.
- [Roland FA-06/FA-07/FA-08 support and manuals](https://www.roland.com/global/products/fa-06/support/) — Reference Manual, Parameter Guide, Sound List, and MIDI Implementation.

The SVD5 projects are useful comparative references but do not specify the FA
SVD1/MI73 parameter layout. Their code must not be copied into FAEditor without
observing the respective licenses.
