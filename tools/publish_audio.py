#!/usr/bin/env python3
"""Creates the audio package of the client and uploads it as a release asset.

The sound and music files are not part of the git repository because of their size (about
440 MB). They are published as ``MuMain-audio-<id>.tar.gz`` on the manifest release, where
the client manifest and the launcher pick them up. Run this script whenever the audio
changes, from a machine which has the files below ``src/bin/Data``::

    python tools/publish_audio.py --upload

The archive is created deterministically, so unchanged files always result in the same id
and the launcher does not download the audio again.
"""

from __future__ import annotations

import argparse
import gzip
import hashlib
import os
import subprocess
import sys
import tarfile

# 2024-01-01T00:00:00Z; a fixed timestamp keeps the archive deterministic.
FIXED_MTIME = 1704067200
ARCHIVE_PREFIX = "MuMain-audio-"
DIRECTORIES = ("Sound", "Music")


def collect_entries(data_directory: str) -> list[tuple[str, str]]:
    entries: list[tuple[str, str]] = []
    for directory_name in DIRECTORIES:
        base_directory = os.path.join(data_directory, directory_name)
        if not os.path.isdir(base_directory):
            raise SystemExit(f"Missing directory: {base_directory}")

        for current_directory, _, files in os.walk(base_directory):
            for file_name in files:
                file_path = os.path.join(current_directory, file_name)
                relative_path = os.path.relpath(file_path, data_directory).replace(os.sep, "/")
                entries.append((file_path, f"Data/{relative_path}"))

    if not entries:
        raise SystemExit(f"No audio files found below {data_directory}.")

    entries.sort(key=lambda entry: entry[1])
    return entries


def build_archive(data_directory: str, archive_path: str) -> None:
    entries = collect_entries(data_directory)
    with open(archive_path, "wb") as raw_file:
        # mtime=0 keeps the gzip header deterministic.
        with gzip.GzipFile(filename="", mode="wb", fileobj=raw_file, mtime=0, compresslevel=6) as gzip_file:
            with tarfile.open(fileobj=gzip_file, mode="w", format=tarfile.PAX_FORMAT) as tar_file:
                for file_path, archive_name in entries:
                    info = tar_file.gettarinfo(file_path, archive_name)
                    info.mtime = FIXED_MTIME
                    info.uid = 0
                    info.gid = 0
                    info.uname = ""
                    info.gname = ""
                    info.mode = 0o644
                    with open(file_path, "rb") as entry_file:
                        tar_file.addfile(info, entry_file)

    print(f"Added {len(entries)} files to {archive_path}.")


def sha256_of_file(file_path: str) -> str:
    digest = hashlib.sha256()
    with open(file_path, "rb") as file:
        for block in iter(lambda: file.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


def ensure_release(repository: str, release_tag: str) -> None:
    result = subprocess.run(
        ["gh", "release", "view", release_tag, "-R", repository],
        capture_output=True,
        text=True,
        check=False,
    )
    if result.returncode == 0:
        return

    print(f"Creating the release {release_tag} in {repository}.")
    subprocess.run(
        [
            "gh",
            "release",
            "create",
            release_tag,
            "-R",
            repository,
            "--title",
            "Client manifest",
            "--notes",
            "Manifest, mirrored runtime archives and the audio package, used by the client launcher. Do not delete.",
            "--latest=false",
        ],
        check=True,
    )


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--data-dir", default=os.path.join("src", "bin", "Data"), help="Directory which contains Sound and Music")
    parser.add_argument("--output-dir", default=".", help="Directory into which the archive is written")
    parser.add_argument("--release", default="client-latest", help="Release tag which stores the audio package")
    parser.add_argument("--repository", default="x9999mu/MuMain", help="Repository which stores the audio package")
    parser.add_argument("--upload", action="store_true", help="Upload the archive with the GitHub CLI")
    return parser.parse_args()


def main() -> None:
    args = parse_args()
    os.makedirs(args.output_dir, exist_ok=True)

    temporary_archive = os.path.join(args.output_dir, "MuMain-audio.tmp.tar.gz")
    build_archive(args.data_dir, temporary_archive)

    digest = sha256_of_file(temporary_archive)
    size = os.path.getsize(temporary_archive)
    audio_id = digest[:12]
    archive_name = f"{ARCHIVE_PREFIX}{audio_id}.tar.gz"
    archive_path = os.path.join(args.output_dir, archive_name)
    os.replace(temporary_archive, archive_path)

    print(f"Audio id: {audio_id}")
    print(f"Size: {size} bytes")
    print(f"sha256: {digest}")
    print(f"Archive: {archive_path}")

    if not args.upload:
        print("Run again with --upload to publish it as a release asset.")
        return

    ensure_release(args.repository, args.release)
    subprocess.run(
        ["gh", "release", "upload", args.release, archive_path, "--clobber", "-R", args.repository],
        check=True,
    )
    print(
        "Uploaded. The manifest is refreshed automatically with the next push to main/develop, "
        "or by running the 'Client manifest' workflow."
    )


if __name__ == "__main__":
    sys.exit(main())
