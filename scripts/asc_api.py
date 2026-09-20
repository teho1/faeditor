#!/usr/bin/env python3
"""Shared App Store Connect JWT helpers for FA Editor CI."""

from __future__ import annotations

import base64
import json
import os
import subprocess
import tempfile
import time
import urllib.error
import urllib.request
from pathlib import Path
from urllib.parse import quote


API = "https://api.appstoreconnect.apple.com"


def _b64url(data: bytes) -> str:
    return base64.urlsafe_b64encode(data).rstrip(b"=").decode("ascii")


def _der_to_p1363(der: bytes) -> bytes:
    # Convert OpenSSL ECDSA DER signature to r||s (64 bytes for P-256).
    if der[0] != 0x30:
        raise ValueError("unexpected ECDSA signature")
    idx = 2 if der[1] < 0x80 else 3
    if der[idx] != 0x02:
        raise ValueError("missing r")
    r_len = der[idx + 1]
    r = der[idx + 2 : idx + 2 + r_len]
    idx = idx + 2 + r_len
    if der[idx] != 0x02:
        raise ValueError("missing s")
    s_len = der[idx + 1]
    s = der[idx + 2 : idx + 2 + s_len]
    r = r.lstrip(b"\x00").rjust(32, b"\x00")
    s = s.lstrip(b"\x00").rjust(32, b"\x00")
    return r + s


def jwt_token() -> str:
    key_id = os.environ["APPSTORE_API_KEY_ID"]
    issuer = os.environ["APPSTORE_ISSUER_ID"]
    pem = os.environ["APPSTORE_API_PRIVATE_KEY"]
    now = int(time.time())
    header = _b64url(json.dumps({"alg": "ES256", "kid": key_id, "typ": "JWT"}).encode())
    payload = _b64url(
        json.dumps(
            {
                "iss": issuer,
                "iat": now - 10,
                "exp": now + 19 * 60,
                "aud": "appstoreconnect-v1",
            }
        ).encode()
    )
    message = f"{header}.{payload}".encode()
    with tempfile.NamedTemporaryFile("w", suffix=".p8", delete=False) as handle:
        handle.write(pem if pem.endswith("\n") else pem + "\n")
        key_path = handle.name
    try:
        der = subprocess.check_output(
            ["openssl", "dgst", "-sha256", "-sign", key_path],
            input=message,
        )
    finally:
        Path(key_path).unlink(missing_ok=True)
    return f"{header}.{payload}.{_b64url(_der_to_p1363(der))}"


def request(method: str, path: str, body: dict | None = None):
    data = None if body is None else json.dumps(body).encode()
    req = urllib.request.Request(
        API + path,
        data=data,
        method=method,
        headers={
            "Authorization": f"Bearer {jwt_token()}",
            "Content-Type": "application/json",
            "Accept": "application/json",
        },
    )
    try:
        with urllib.request.urlopen(req) as response:
            raw = response.read()
            return json.loads(raw) if raw else {}
    except urllib.error.HTTPError as error:
        detail = error.read().decode()
        raise SystemExit(f"App Store Connect {method} {path} failed ({error.code}): {detail}") from error


def find_app(bundle_id: str) -> dict | None:
    payload = request("GET", f"/v1/apps?filter[bundleId]={bundle_id}&limit=10")
    items = payload.get("data") or []
    return items[0] if items else None


def find_bundle_id(identifier: str, platform: str = "IOS") -> dict | None:
    payload = request(
        "GET",
        f"/v1/bundleIds?filter[identifier]={quote(identifier, safe='')}&limit=50",
    )
    for item in payload.get("data") or []:
        attributes = item.get("attributes") or {}
        if attributes.get("identifier") == identifier and attributes.get("platform") == platform:
            return item
    return None
