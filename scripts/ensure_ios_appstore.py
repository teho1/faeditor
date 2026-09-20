#!/usr/bin/env python3
"""Create the iOS App Store Connect app and bundle ID when missing."""

from __future__ import annotations

import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from asc_api import find_app, find_bundle_id, request  # noqa: E402

BUNDLE_ID = "com.righthere.faeditor.ios"
APP_NAME = "Editor for Roland FA"
SKU = "faeditor-ios"
PRIVACY_URL = "https://github.com/teho1/faeditor/blob/main/PRIVACY.md"


def main() -> None:
    bundle = find_bundle_id(BUNDLE_ID, "IOS")
    if bundle is None:
        print(f"Creating bundle ID {BUNDLE_ID}")
        bundle = request(
            "POST",
            "/v1/bundleIds",
            {
                "data": {
                    "type": "bundleIds",
                    "attributes": {
                        "identifier": BUNDLE_ID,
                        "name": "FA Editor iOS",
                        "platform": "IOS",
                    },
                }
            },
        )["data"]
    else:
        print(f"Bundle ID already exists: {bundle['id']}")

    app = find_app(BUNDLE_ID)
    if app is None:
        print(f"Creating App Store Connect app {APP_NAME}")
        app = request(
            "POST",
            "/v1/apps",
            {
                "data": {
                    "type": "apps",
                    "attributes": {
                        "bundleId": BUNDLE_ID,
                        "name": APP_NAME,
                        "primaryLocale": "en-US",
                        "sku": SKU,
                    },
                }
            },
        )["data"]
    else:
        print(f"App already exists: {app['id']} {app['attributes'].get('name')}")

    print(f"iOS app id={app['id']} privacy={PRIVACY_URL}")


if __name__ == "__main__":
    main()
