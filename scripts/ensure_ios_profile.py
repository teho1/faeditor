#!/usr/bin/env python3
"""Install an iOS App Store provisioning profile for manual CI signing."""

from __future__ import annotations

import base64
import os
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from asc_api import find_bundle_id, request  # noqa: E402

BUNDLE_ID = "com.righthere.faeditor"
PROFILE_NAME = "FA Editor iOS App Store CI"


def log(message: str) -> None:
    print(message, flush=True)


def included_by_id(payload: dict) -> dict:
    return {item["id"]: item for item in payload.get("included") or []}


def profile_bundle_id(profile: dict, included: dict) -> str | None:
    rel = ((profile.get("relationships") or {}).get("bundleId") or {}).get("data") or {}
    item = included.get(rel.get("id"))
    if not item:
        return None
    return (item.get("attributes") or {}).get("identifier")


def find_existing_profile() -> dict | None:
    payload = request(
        "GET",
        "/v1/profiles?filter[profileType]=IOS_APP_STORE&include=bundleId&limit=200",
    )
    included = included_by_id(payload)
    for profile in payload.get("data") or []:
        if profile_bundle_id(profile, included) == BUNDLE_ID:
            return profile
        name = (profile.get("attributes") or {}).get("name")
        if name == PROFILE_NAME:
            return profile
    return None


def find_distribution_certificates() -> list[dict]:
    certs: list[dict] = []
    for cert_type in ("DISTRIBUTION", "IOS_DISTRIBUTION"):
        payload = request(
            "GET",
            f"/v1/certificates?filter[certificateType]={cert_type}&limit=50",
            fatal=False,
        )
        if payload.get("_http_status"):
            continue
        certs.extend(payload.get("data") or [])
    return certs


def create_profile(bundle_id: str, certificate_ids: list[str]) -> dict:
    payload = request(
        "POST",
        "/v1/profiles",
        {
            "data": {
                "type": "profiles",
                "attributes": {
                    "name": PROFILE_NAME,
                    "profileType": "IOS_APP_STORE",
                },
                "relationships": {
                    "bundleId": {"data": {"type": "bundleIds", "id": bundle_id}},
                    "certificates": {
                        "data": [
                            {"type": "certificates", "id": cert_id}
                            for cert_id in certificate_ids
                        ]
                    },
                },
            }
        },
        fatal=False,
    )
    if payload.get("_http_status"):
        existing = find_existing_profile()
        if existing is None:
            raise SystemExit("Could not create or find an iOS App Store profile")
        return existing
    return payload["data"]


def install_profile(profile: dict) -> str:
    attributes = profile.get("attributes") or {}
    name = attributes.get("name") or PROFILE_NAME
    content = attributes.get("profileContent")
    uuid = attributes.get("uuid")
    if not content or not uuid:
        profile = request("GET", f"/v1/profiles/{profile['id']}")["data"]
        attributes = profile.get("attributes") or {}
        content = attributes.get("profileContent")
        uuid = attributes.get("uuid")
        name = attributes.get("name") or name
    if not content or not uuid:
        raise SystemExit("Provisioning profile is missing profileContent/uuid")
    raw = base64.b64decode(content)
    destinations = [
        Path.home() / "Library/MobileDevice/Provisioning Profiles",
        Path.home() / "Library/Developer/Xcode/UserData/Provisioning Profiles",
    ]
    for directory in destinations:
        directory.mkdir(parents=True, exist_ok=True)
        path = directory / f"{uuid}.mobileprovision"
        path.write_bytes(raw)
        log(f"Installed profile {name} -> {path}")
    return name


def write_github_env(name: str) -> None:
    github_env = os.environ.get("GITHUB_ENV")
    if not github_env:
        return
    with open(github_env, "a", encoding="utf-8") as handle:
        handle.write(f"APPLE_PROFILE_NAME={name}\n")


def main() -> None:
    bundle = find_bundle_id(BUNDLE_ID)
    if bundle is None:
        raise SystemExit(f"No Developer bundle ID for {BUNDLE_ID}")
    log(f"Bundle {BUNDLE_ID} id={bundle['id']} platform={(bundle.get('attributes') or {}).get('platform')}")

    profile = find_existing_profile()
    if profile is None:
        certs = find_distribution_certificates()
        if not certs:
            raise SystemExit("No Apple Distribution certificate in App Store Connect")
        cert_ids = [item["id"] for item in certs]
        log(f"Creating {PROFILE_NAME} with {len(cert_ids)} certificate(s)")
        profile = create_profile(bundle["id"], cert_ids)
    else:
        log(f"Reusing profile {(profile.get('attributes') or {}).get('name')}")

    name = install_profile(profile)
    write_github_env(name)
    log(f"APPLE_PROFILE_NAME={name}")


if __name__ == "__main__":
    main()
