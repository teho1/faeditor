#!/usr/bin/env python3
"""Regenerate resources/waves/waveforms.json from the official FA Sound List PDF text dump.

Source: https://static.roland.com/assets/media/pdf/FA-06_07_08_SoundList_multi01_W.pdf
Pass the extracted text path as argv[1], or set SOUNDLIST_TXT.

Extracts:
  - SuperNATURAL Synth PCM Waveform (1–450)
  - PCM Synth Waveform INT-A (~1–1083)
  - PCM Synth Waveform INT-B (~1–790)

SN-S PCM and PCM Synth INT lists are kept separate (different ROM tables).
"""

from __future__ import annotations

import json
import os
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
DEFAULT_TXT = (
    Path.home()
    / ".cursor/projects/Users-teholapp-development-faeditor/agent-tools"
    / "46540030-71de-45bd-8b70-395f7e91b4e7.txt"
)
OUT = ROOT / "resources/waves/waveforms.json"

ENTRY_RE = re.compile(r"(?m)^(\d+)\s+(.+?)\s*$")


def extract_section(text: str, start_marker: str, end_markers: list[str]) -> str:
    i = text.find(start_marker)
    if i < 0:
        return ""
    body = text[i + len(start_marker) :]
    end = len(body)
    for marker in end_markers:
        j = body.find(marker)
        if j >= 0:
            end = min(end, j)
    return body[:end]


def parse_waves(blob: str) -> dict[str, str]:
    waves: dict[int, str] = {}
    for m in ENTRY_RE.finditer(blob):
        n = int(m.group(1))
        name = m.group(2).strip()
        if not name or name == "No Wave Name" or name.startswith("#"):
            continue
        if re.fullmatch(r"\d+", name):
            continue
        name = re.sub(r"\s+", " ", name).strip(" .")
        if not name or len(name) > 40:
            continue
        waves[n] = name
    return {str(k): waves[k] for k in sorted(waves)}


def main() -> int:
    txt_path = Path(
        sys.argv[1] if len(sys.argv) > 1 else os.environ.get("SOUNDLIST_TXT", DEFAULT_TXT)
    )
    if not txt_path.exists():
        print(f"missing sound list text: {txt_path}", file=sys.stderr)
        return 1

    text = txt_path.read_text(encoding="utf-8", errors="replace")
    text = (
        text.replace(" .", ".")
        .replace("‘", "'")
        .replace("’", "'")
        .replace("–", "-")
    )

    sn_blob = extract_section(text, "### PCM Waveform", ["### PCM Synth Waveform"])
    int_a_blob = extract_section(text, "#### (INT-A)", ["#### (INT-B)"])
    int_b_blob = extract_section(text, "#### (INT-B)", ["## ", "-----"])

    sn = parse_waves(sn_blob)
    int_a = parse_waves(int_a_blob)
    int_b = parse_waves(int_b_blob)

    doc = {
        "source": "FA-06/07/08 Sound List (Roland)",
        "sourceUrl": "https://static.roland.com/assets/media/pdf/FA-06_07_08_SoundList_multi01_W.pdf",
        "note": "MIDI wave number 0 = OFF; names are keyed from 1. SN-S PCM ≠ PCM Synth INT-A/B.",
        "snSynthPcm": sn,
        "pcmIntA": int_a,
        "pcmIntB": int_b,
    }

    OUT.parent.mkdir(parents=True, exist_ok=True)
    OUT.write_text(json.dumps(doc, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")

    print(f"wrote SN-S PCM={len(sn)} INT-A={len(int_a)} INT-B={len(int_b)} -> {OUT}")

    checks = [
        ("snSynthPcm", "1", "JP-8 Saw"),
        ("snSynthPcm", "450", "SynthFx 2"),
        ("pcmIntA", "1", "StGrand pA L"),
        ("pcmIntA", "1083", "REV Metro"),
        ("pcmIntB", "1", "Jazz Doo L"),
        ("pcmIntB", "790", "Reverse Cym"),
    ]
    ok = True
    for table, key, expect in checks:
        got = doc[table].get(key)
        good = got == expect
        ok = ok and good
        print(("OK" if good else "BAD"), table, key, got, "expected", expect)

    if len(sn) != 450 or len(int_a) != 1083 or len(int_b) != 790:
        print(
            f"BAD counts: expected 450/1083/790 got {len(sn)}/{len(int_a)}/{len(int_b)}",
            file=sys.stderr,
        )
        ok = False
    return 0 if ok else 2


if __name__ == "__main__":
    raise SystemExit(main())
