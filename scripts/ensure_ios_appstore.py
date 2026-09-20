#!/usr/bin/env python3
"""Ensure the iOS bundle ID exists. App records must be created in App Store Connect."""

from __future__ import annotations

import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from asc_api import find_app, find_bundle_id, request  # noqa: E402

BUNDLE_ID = "com.righthere.faeditor"


def log(message: str) -> None:
    print(message, flush=True)


def main() -> None:
    bundle = find_bundle_id(BUNDLE_ID)
    if bundle is None:
        log(f"Creating bundle ID {BUNDLE_ID}")
        bundle = request(
            "POST",
            "/v1/bundleIds",
            {
                "data": {
                    "type": "bundleIds",
                    "attributes": {
                        "identifier": BUNDLE_ID,
                        "name": "FA Editor",
                        "platform": "IOS",
                    },
                }
            },
        )["data"]
    log(f"Bundle ID ready: {bundle['id']}")

    app = find_app(BUNDLE_ID)
    if app is None:
        log(
            f"No App Store Connect app for {BUNDLE_ID}. "
            "Add the iOS platform to the existing app in App Store Connect, then uploads can go to TestFlight. "
            "Continuing the archive."
        )
        return
    log(f"App already exists: {app['id']} {app['attributes'].get('name')}")


if __name__ == "__main__":
    main()
