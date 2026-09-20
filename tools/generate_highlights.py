#!/usr/bin/env python3
"""Creates the short, Vietnamese "what's new" list which the client launcher shows.

The list is built from the most recent commits of this repository:

* commits which only touch the build, the CI or tooling are skipped
* an explicit ``Highlight:`` line of the commit body wins, so new commits can provide their
  own player facing text, for example::

      git commit -m "Raise the fire socket options" -m "Highlight: Tăng sát thương socket Fire"

* otherwise the subject is translated with the rules below
* if nothing matches, the subject is used as it is, so nothing gets lost

Usage::

    python tools/generate_highlights.py --count 4 --output highlights.json
"""

from __future__ import annotations

import argparse
import json
import re
import subprocess
import sys

DEFAULT_COUNT = 4
SCAN_LIMIT = 80
END_MARKER = "---END---"

# Commits which are not interesting for the players: build, CI and tooling changes.
SKIP_PATTERN = re.compile(
    r"(?i)(^(ci|chore|test|tests|build|docs|style|lint|format|refactor|release)(\([^)]*\))?\s*:)"
    r"|(cppcheck|clang-format|client manifest|manifest release|runtime archive|"
    r"mirror build artifacts|wait for the data archive|refresh the client manifest)"
)

# Ordered rules: the first matching pattern wins.
# Groups like (?P<element>...) are inserted into the Vietnamese text.
TRANSLATION_RULES: list[tuple[str, str]] = [
    (r"(?i)client audio as a separate package", "Client có đầy đủ âm thanh (Sound và Music)"),
    (r"(?i)lightning socket option.*critical damage", "Tăng sát thương chí mạng của socket Lightning"),
    (r"(?i)raise the remaining socket option values", "Tăng chỉ số các socket còn lại (Ice, Wind, Earth, Defence)"),
    (r"(?i)raise the fire socket option values", "Tăng sát thương socket Fire"),
    (r"(?i)trade with a right click", "Thêm chuột phải để đưa item vào cửa sổ trade (ví dụ hộp Kundun)"),
    (r"(?i)server player list window opened with (?P<key>\w+)", "Thêm cửa sổ danh sách người chơi trong server (phím %(key)s)"),
    (r"(?i)map position in the server player list", "Hiện vị trí trên map trong danh sách người chơi"),
    (r"(?i)correct class in the server player list", "Hiện đúng class trong danh sách người chơi"),
    (r"(?i)server player list request as a custom packet", "Cải thiện yêu cầu danh sách người chơi"),
    (r"(?i)master skill data cover the rage fighter", "Cập nhật master skill cho Rage Fighter"),
    (r"(?i)item options when a special entry has no text", "Giữ option của item khi dòng mô tả đặc biệt không có chữ"),
    (r"(?i)fist master open the master skill tree", "Cho Fist Master mở được cây master skill"),
    (r"(?i)master skill buff upgrades with the buff they replace", "Xoá đúng buff master skill khi bị thay thế"),
    (r"(?i)(auto attack|mu ?helper)", "Sửa lỗi auto attack (MU Helper)"),
    (r"(?i)seed sphere item data for level 4 and 5", "Cập nhật dữ liệu Seed Sphere level 4 và 5"),
    (r"(?i)sphere \(4\) and sphere \(5\)", "Thêm Sphere (4) và Sphere (5) vào dữ liệu item"),
    (r"(?i)why a crafting was rejected", "Hiển thị lý do mix đồ bị từ chối"),
    (r"(?i)keep mixing after mix", "Cho phép mix tiếp sau khi mix xong"),
    (r"(?i)auto move item after success mix", "Tự động chuyển item sau khi mix thành công"),
    (r"(?i)confirm add master skill", "Bỏ bước xác nhận khi cộng master skill"),
    (r"(?i)additional ops of item 380", "Hiển thị thêm option của item 380"),
    (r"(?i)inventory|vault", "Cập nhật túi đồ và thùng đồ"),
    (r"(?i)rage fighter", "Cập nhật class Rage Fighter"),
    (r"(?i)craft(ing)?|mix", "Cập nhật hệ thống mix đồ"),
    (r"(?i)socket", "Cập nhật hệ thống socket"),
    (r"(?i)master skill|master level", "Cập nhật master skill"),
    (r"(?i)item (data|option)", "Cập nhật dữ liệu item"),
]

COMPILED_RULES = [(re.compile(pattern), replacement) for pattern, replacement in TRANSLATION_RULES]


def read_commits() -> list[tuple[str, str]]:
    """Returns the (subject, body) pairs of the most recent commits."""
    output = subprocess.run(
        ["git", "log", f"-{SCAN_LIMIT}", "--no-merges", f"--pretty=%s%n%b{END_MARKER}"],
        capture_output=True,
        text=True,
        check=True,
    ).stdout

    commits: list[tuple[str, str]] = []
    for block in output.split(END_MARKER):
        block = block.strip()
        if not block:
            continue

        lines = block.splitlines()
        subject = lines[0].strip()
        body = "\n".join(lines[1:]).strip()
        commits.append((subject, body))

    return commits


def get_highlight(subject: str, body: str) -> str | None:
    """Returns the player facing text of a commit, or None if it should be skipped."""
    if SKIP_PATTERN.search(subject):
        return None

    explicit = re.search(r"(?im)^\s*(highlight|hiển thị|hien thi)\s*:\s*(.+)$", body)
    if explicit:
        return explicit.group(2).strip()

    for pattern, replacement in COMPILED_RULES:
        match = pattern.search(subject)
        if match:
            text = replacement % match.groupdict() if "%(" in replacement else replacement
            return text.strip()

    # Nothing matched: keep the original subject so that nothing is lost.
    return subject.strip()


def build_highlights(count: int) -> list[str]:
    highlights: list[str] = []
    for subject, body in read_commits():
        text = get_highlight(subject, body)
        if text is None or text in highlights:
            continue

        highlights.append(text)
        if len(highlights) >= count:
            break

    return highlights


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--count", type=int, default=DEFAULT_COUNT, help="Number of entries to produce")
    parser.add_argument("--output", help="Write the JSON array into this file instead of stdout")
    return parser.parse_args()


def main() -> None:
    if hasattr(sys.stdout, "reconfigure"):
        # The Vietnamese text otherwise breaks the console encoding of Windows.
        sys.stdout.reconfigure(encoding="utf-8")

    args = parse_args()
    highlights = build_highlights(max(1, args.count))
    content = json.dumps(highlights, ensure_ascii=False, indent=2)

    if args.output:
        with open(args.output, "w", encoding="utf-8", newline="\n") as file:
            file.write(content + "\n")
        print(f"Wrote {len(highlights)} entries to {args.output}.")
    else:
        print(content)


if __name__ == "__main__":
    sys.exit(main())
