#!/usr/bin/env python3
"""Generates the client update manifest which is consumed by the OpenMU client launcher.

The layout is described in docs/ClientLauncherDesign.md of the OpenMU repository.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import os
from datetime import datetime, timezone

SCHEMA_VERSION = 1
ARCHIVE_FORMAT = "tar.gz"


def sha256_of_file(path: str) -> str:
    digest = hashlib.sha256()
    with open(path, "rb") as file:
        for block in iter(lambda: file.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


def describe_archive(path: str, url: str) -> dict:
    archive_format = "zip" if path.lower().endswith(".zip") else ARCHIVE_FORMAT
    return {
        "url": url,
        "size": os.path.getsize(path),
        "sha256": sha256_of_file(path),
        "format": archive_format,
    }


def release_base(repository: str) -> str:
    return f"https://github.com/{repository}/releases/download"


def build_manifest(args: argparse.Namespace) -> dict:
    runtime_name = os.path.basename(args.runtime_archive)
    data_name = os.path.basename(args.data_archive)
    runtime_tag = args.runtime_tag or f"v{args.runtime_version}"
    data_tag = f"data-{args.data_id}"
    # --base-url is used for local test runs, where a simple http server serves the archives.
    base_url = args.base_url.rstrip("/") + "/" if args.base_url else None
    runtime_url = args.runtime_url or (
        base_url + runtime_name if base_url else f"{release_base(args.repository)}/{runtime_tag}/{runtime_name}"
    )
    data_url = args.data_url or (
        base_url + data_name if base_url else f"{release_base(args.repository)}/{data_tag}/{data_name}"
    )

    manifest = {
        "schemaVersion": SCHEMA_VERSION,
        "channel": args.channel,
        "generatedAtUtc": datetime.now(timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ"),
        "runtime": {
            "version": args.runtime_version,
            "tag": runtime_tag,
            "archive": describe_archive(
                args.runtime_archive,
                runtime_url,
            ),
        },
        "data": {
            "id": args.data_id,
            "tag": data_tag,
            "archive": describe_archive(
                args.data_archive,
                data_url,
            ),
        },
        "server": {
            "host": args.server_host,
            "hostName": args.server_host_name,
            "port": args.server_port,
        },
        "preserve": ["config.ini"],
    }

    if args.commits_file and os.path.isfile(args.commits_file):
        with open(args.commits_file, encoding="utf-8") as file:
            manifest["commits"] = json.load(file)

    if args.audio_id and args.audio_url and args.audio_sha256 and args.audio_size:
        manifest["audio"] = {
            "id": args.audio_id,
            "tag": args.audio_tag or args.runtime_tag or args.repository,
            "archive": {
                "url": args.audio_url,
                "size": args.audio_size,
                "sha256": args.audio_sha256,
                "format": "tar.gz",
            },
        }

    if args.launcher_version and args.launcher_archive:
        launcher_name = os.path.basename(args.launcher_archive)
        launcher_tag = f"launcher-v{args.launcher_version}"
        launcher_url = args.launcher_url or (
            base_url + launcher_name if base_url else f"{release_base(args.launcher_repository)}/{launcher_tag}/{launcher_name}"
        )
        launcher_archive = describe_archive(args.launcher_archive, launcher_url)
        manifest["launcher"] = {
            "version": args.launcher_version,
            "url": launcher_archive["url"],
            "size": launcher_archive["size"],
            "sha256": launcher_archive["sha256"],
        }

    return manifest


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--runtime-version", required=True, help="Version of the client runtime, e.g. 1.4.2")
    parser.add_argument("--runtime-archive", required=True, help="Path of the runtime tar.gz archive")
    parser.add_argument("--data-id", required=True, help="Content id of the data release")
    parser.add_argument("--data-archive", required=True, help="Path of the data tar.gz archive")
    parser.add_argument("--repository", default="x9999mu/MuMain", help="Repository which hosts the archives")
    parser.add_argument("--base-url", help="Overrides the archive urls, e.g. http://127.0.0.1:8080 for local test runs")
    parser.add_argument("--runtime-url", help="Explicit url of the runtime archive; overrides --base-url and the release url")
    parser.add_argument("--runtime-tag", help="Release tag which hosts the runtime archive, e.g. client-latest")
    parser.add_argument("--data-url", help="Explicit url of the data archive; overrides --base-url and the release url")
    parser.add_argument("--launcher-url", help="Explicit url of the launcher executable")
    parser.add_argument("--audio-id", help="Content id of the audio package (Data/Sound and Data/Music)")
    parser.add_argument("--audio-url", help="Url of the audio archive")
    parser.add_argument("--audio-sha256", help="sha256 of the audio archive")
    parser.add_argument("--audio-size", type=int, help="Size of the audio archive in bytes")
    parser.add_argument("--audio-tag", help="Release tag which hosts the audio archive")
    parser.add_argument("--commits-file", help="JSON file with the short change list for the launcher")
    parser.add_argument("--channel", default="stable", help="Release channel")
    parser.add_argument("--server-host", default="100.108.169.118", help="Connect server address")
    parser.add_argument("--server-host-name", default="", help="Optional connect server host name")
    parser.add_argument("--server-port", type=int, default=44405, help="Connect server port")
    parser.add_argument("--launcher-version", help="Version of the published launcher")
    parser.add_argument("--launcher-archive", help="Path of the published launcher executable")
    parser.add_argument("--launcher-repository", default="x9999mu/OpenMU", help="Repository which hosts the launcher")
    parser.add_argument("--output", required=True, help="Path of the manifest to write")
    return parser.parse_args()


def main() -> None:
    args = parse_args()
    manifest = build_manifest(args)
    with open(args.output, "w", encoding="utf-8", newline="\n") as file:
        json.dump(manifest, file, indent=2)
        file.write("\n")
    print(f"Wrote {args.output}: runtime {args.runtime_version}, data {args.data_id}")


if __name__ == "__main__":
    main()
