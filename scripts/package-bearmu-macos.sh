#!/usr/bin/env bash
set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
source_app="$repo_root/out/build/macos-arm64-apple/src/Release/Main.app"
archive="$repo_root/BearMU-macos-arm64.zip"
work_dir="$(mktemp -d)"
staged_app="$work_dir/BearMU.app"
trap 'rm -rf "$work_dir"' EXIT

required_paths=(
    "Contents/MacOS/Main"
    "Contents/MacOS/MUnique.Client.Library.dylib"
    "Contents/MacOS/config.ini"
    "Contents/MacOS/shaders"
    "Contents/MacOS/Data"
    "Contents/MacOS/fonts"
)

for path in "${required_paths[@]}"; do
    if [[ ! -e "$source_app/$path" ]]; then
        echo "Missing required bundle path: $source_app/$path" >&2
        exit 1
    fi
done

ditto "$source_app" "$staged_app"
/usr/libexec/PlistBuddy -c "Set :CFBundleName BearMU" "$staged_app/Contents/Info.plist"

rm -f "$archive"
ditto -c -k --sequesterRsrc --keepParent "$staged_app" "$archive"
echo "Created $archive"
