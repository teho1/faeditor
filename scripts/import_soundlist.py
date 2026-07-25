#!/usr/bin/env python3
"""Regenerate resources/tones/soundlist.json from the official FA Sound List PDF text dump.

Source: https://static.roland.com/assets/media/pdf/FA-06_07_08_SoundList_multi01_W.pdf
Pass the extracted text path as argv[1], or set SOUNDLIST_TXT.
"""

from __future__ import annotations

import json
import os
import re
import sys
from collections import Counter
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
DEFAULT_TXT = Path.home() / ".cursor/projects/Users-teholapp-development-faeditor/agent-tools/46540030-71de-45bd-8b70-395f7e91b4e7.txt"
OUT = ROOT / "resources/tones/soundlist.json"

CATEGORIES = sorted(
    {
        "Synth Pad/Strings",
        "Ensemble Strings",
        "Ensemble Brass",
        "Plucked/Stroke",
        "Other Keyboards",
        "Bell/Mallet",
        "Synth PolyKey",
        "Synth Seq/Pop",
        "Synth Bellpad",
        "Synth Brass",
        "Synth Bass",
        "Synth Lead",
        "Synth FX",
        "Sound FX",
        "Beat&Groove",
        "Solo Strings",
        "Solo Brass",
        "Pipe Organ",
        "Pop Piano",
        "Ac.Piano",
        "Ac.Guitar",
        "Ac.Bass",
        "E.Piano1",
        "E.Piano2",
        "E.Piano",
        "E.Guitar",
        "E.Organ",
        "E.Bass",
        "Dist.Guitar",
        "Vox/Choir",
        "Pulsating",
        "Orchestral",
        "Percussion",
        "Harmonica",
        "Harpsichord",
        "Celesta",
        "Flute",
        "Sax",
        "Wind",
        "Hit",
        "Bell",
        "Mallet",
        "Clav",
        "Drums",
        "Scat",
        "FX",
        "Other",
        "Strings",
        "Brass",
        "Bass",
        "Organ",
        "Piano",
        "Guitar",
        "Pad",
        "Lead",
        "Ethnic",
        "Phrase",
        "Synth",
        "Reed",
        "Pipe",
        "Percussive",
        "SFX",
    },
    key=len,
    reverse=True,
)


def split_name_cat(mid: str) -> tuple[str, str]:
    for cat in CATEGORIES:
        if mid == cat:
            return mid, cat
        if mid.endswith(" " + cat):
            name = mid[: -(len(cat) + 1)].rstrip()
            if name:
                return name, cat
    parts = mid.rsplit(" ", 1)
    if len(parts) == 2:
        return parts[0], parts[1]
    return mid, "Other"


def main() -> int:
    txt_path = Path(sys.argv[1] if len(sys.argv) > 1 else os.environ.get("SOUNDLIST_TXT", DEFAULT_TXT))
    if not txt_path.exists():
        print(f"missing sound list text: {txt_path}", file=sys.stderr)
        return 1

    text = txt_path.read_text(encoding="utf-8", errors="replace")
    end = text.find("## Waveforms")
    body = text[: end if end > 0 else len(text)]
    start = body.find("### SuperNATURAL Acoustic Tone")
    body = body[start if start >= 0 else 0 :]
    body = (
        body.replace(" .", ".")
        .replace("‘", "'")
        .replace("’", "'")
        .replace("–", "-")
    )

    tones: list[dict] = []
    seen: set[tuple[int, int, int]] = set()

    def add(name: str, cat: str, msb: int, lsb: int, pc1: int) -> bool:
        name = name.strip(" .")
        cat = cat.strip()
        if not name or not (1 <= pc1 <= 128):
            return False
        key = (msb, lsb, pc1)
        if key in seen:
            return False
        seen.add(key)
        tones.append(
            {
                "name": name,
                "category": cat,
                "bankMsb": msb,
                "bankLsb": lsb,
                "program": pc1 - 1,
            }
        )
        return True

    gm2_pos = body.find("### GM2 Tone")
    main = body[: gm2_pos if gm2_pos > 0 else len(body)]

    headers = list(re.finditer(r"No (?:Tone|Kit) Name Category MSB LSB PC", main))
    blobs: list[str] = []
    for i, h in enumerate(headers):
        chunk = main[h.end() : headers[i + 1].start() if i + 1 < len(headers) else len(main)]
        for stop in ["* SuperNATURAL", "* PCM", "* When shipped", "### ", "## "]:
            j = chunk.find(stop)
            if j >= 0:
                chunk = chunk[:j]
        chunk = re.sub(r"#\s*", " ", chunk)
        blobs.append(chunk)
    raw = re.sub(r"\s+", " ", " ".join(blobs)).strip()

    entry_re = re.compile(
        r"(\d+)\s+(.+?)\s+(89|95|88|87|86)\s+(\d{1,3})\s+(\d{1,3})"
        r"(?=\s+\d+\s+[A-Za-z0-9'\"(]|\s*$|\s+\*)"
    )
    for m in entry_re.finditer(raw):
        name, cat = split_name_cat(m.group(2).strip())
        if len(name) > 40:
            continue
        add(name, cat, int(m.group(3)), int(m.group(4)), int(m.group(5)))

    if gm2_pos > 0:
        gmd_pos = body.find("### GM2 Drum Kit")
        gm2 = body[gm2_pos : gmd_pos if gmd_pos > 0 else len(body)]
        gm2 = re.sub(r"\s+", " ", gm2)
        gm2 = re.sub(r"^.*?No\.?\s*Tone Name Category MSB LSB PC\s*", "", gm2)
        gm2_re = re.compile(
            r"(\d{3})\s+(.+?)\s+(121)\s+(\d{1,3})(?:\s+(\d{1,3}))?(?=\s+\d{3}\s+|\s*$)"
        )
        last_pc = 1
        for m in gm2_re.finditer(gm2):
            name, cat = split_name_cat(m.group(2).strip())
            if m.group(5):
                last_pc = int(m.group(5))
            add(name, cat, 121, int(m.group(4)), last_pc)

        if gmd_pos > 0:
            gmd = body[gmd_pos:]
            for stop in ["#### SuperNATURAL", "## Waveforms", "-----"]:
                j = gmd.find(stop)
                if j > 0:
                    gmd = gmd[:j]
            gmd = re.sub(r"\s+", " ", gmd)
            gmd_re = re.compile(
                r"(\d+)\s+(GM2\s+\S+)\s+(Drums)\s+(120)\s+(\d{1,3})\s+(\d{1,3})"
            )
            for m in gmd_re.finditer(gmd):
                add(m.group(2), m.group(3), int(m.group(4)), int(m.group(5)), int(m.group(6)))

    tones.sort(key=lambda t: (t["bankMsb"], t["bankLsb"], t["program"], t["name"]))
    OUT.parent.mkdir(parents=True, exist_ok=True)
    OUT.write_text(json.dumps(tones, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")

    print(f"wrote {len(tones)} tones -> {OUT}")
    print("by MSB:", dict(Counter(t["bankMsb"] for t in tones)))
    checks = [
        (87, 64, 0, "128voicePno"),
        (89, 64, 0, "Full Grand 1"),
        (95, 64, 0, "JP8 Strings1"),
        (88, 64, 0, "PowerSession"),
        (87, 65, 12, "Finger Bass"),
        (86, 64, 0, "LD Std Kit 1"),
        (121, 0, 0, "Piano 1"),
        (120, 0, 0, "GM2 STANDARD"),
    ]
    ok = True
    for msb, lsb, pc0, expect in checks:
        hit = next(
            (t for t in tones if t["bankMsb"] == msb and t["bankLsb"] == lsb and t["program"] == pc0),
            None,
        )
        name = hit["name"] if hit else None
        good = name == expect
        ok = ok and good
        print(("OK" if good else "BAD"), f"{msb}:{lsb}:PC{pc0 + 1}", name, "expected", expect)
    return 0 if ok else 2


if __name__ == "__main__":
    raise SystemExit(main())
