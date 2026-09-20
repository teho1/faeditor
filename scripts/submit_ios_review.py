#!/usr/bin/env python3
"""Submit the latest processed iOS build for App Store review."""

from __future__ import annotations

import os
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from asc_api import find_app, request  # noqa: E402

BUNDLE_ID = "com.righthere.faeditor.ios"


def main() -> None:
    version = os.environ.get("FAEDITOR_IOS_VERSION", "").strip()
    app = find_app(BUNDLE_ID)
    if app is None:
        raise SystemExit(f"No App Store Connect app for {BUNDLE_ID}")

    builds = request(
        "GET",
        f"/v1/builds?filter[app]={app['id']}&filter[processingState]=VALID&sort=-uploadedDate&limit=20",
    ).get("data") or []
    if not builds:
        raise SystemExit("No processed iOS builds available for review")

    build = builds[0]
    if version:
        matched = [
            item
            for item in builds
            if (item.get("attributes") or {}).get("version") == version
            or version in str((item.get("attributes") or {}).get("version", ""))
        ]
        # CFBundleShortVersionString is on preReleaseVersion, not always on build.version
        if matched:
            build = matched[0]

    print(f"Using build {build['id']} attributes={build.get('attributes')}")

    submission = request(
        "POST",
        "/v1/reviewSubmissions",
        {
            "data": {
                "type": "reviewSubmissions",
                "attributes": {"platform": "IOS"},
                "relationships": {
                    "app": {"data": {"type": "apps", "id": app["id"]}},
                },
            }
        },
    )["data"]
    request(
        "POST",
        "/v1/reviewSubmissionItems",
        {
            "data": {
                "type": "reviewSubmissionItems",
                "relationships": {
                    "reviewSubmission": {
                        "data": {"type": "reviewSubmissions", "id": submission["id"]}
                    },
                    "build": {"data": {"type": "builds", "id": build["id"]}},
                },
            }
        },
    )
    submitted = request(
        "PATCH",
        f"/v1/reviewSubmissions/{submission['id']}",
        {
            "data": {
                "type": "reviewSubmissions",
                "id": submission["id"],
                "attributes": {"submitted": True},
            }
        },
    )
    state = (submitted.get("data") or submitted).get("attributes", {})
    print(f"Review submission {submission['id']} state={state}")


if __name__ == "__main__":
    main()
