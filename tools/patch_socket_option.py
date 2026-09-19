#!/usr/bin/env python3
"""Patches the per-level values of a socket option in the client data files.

The client reads ``Data/Local/<Language>/SocketItem_<Language>.bmd`` to *display* socket
options (tooltips); the actual effect is calculated by the server. Both have to be kept in
sync, so this script changes the display values of one option in all language files.

Each record is a ``SOCKET_OPTION_INFO_FILE`` structure which is XOR-"encrypted" with the key
``FC CF AB`` (``BuxConvert`` in Core/Globals/_crypt.h):

    int  m_iOptionID;              // offset 0
    int  m_iOptionCategory;        // offset 4
    char m_szOptionName[64];       // offset 8
    char m_bOptionType;            // offset 72 (+3 padding)
    int  m_iOptionValue[5];        // offset 76
    BYTE m_bySocketCheckInfo[6];   // offset 96 (+2 padding)  => 104 bytes per record

Example: set "Critical damage increase" (option id 31) of the socket item options table::

    python tools/patch_socket_option.py --option-id 31 --values 1000,2000,3000,4000,5000 \
        src/bin/Data/Local/Eng/SocketItem_Eng.bmd ...
"""

from __future__ import annotations

import argparse
import os
import struct
import sys

KEY = bytes([0xFC, 0xCF, 0xAB])  # BuxConvert key
RECORD_SIZE = 104
OPTIONS_PER_TABLE = 50
TABLE_SOCKET_ITEM_OPTIONS = 0
VALUE_OFFSET = 76
NAME_OFFSET = 8
NAME_LENGTH = 64


def transform(data: bytearray) -> None:
    """Applies BuxConvert to the record; the same operation reverses it."""
    for index in range(len(data)):
        data[index] ^= KEY[index % 3]


def record_offset(table: int, index: int) -> int:
    return (table * OPTIONS_PER_TABLE + index) * RECORD_SIZE


def read_record(content: bytes, option_id: int, table: int) -> bytearray:
    record = bytearray(content[record_offset(table, option_id):record_offset(table, option_id) + RECORD_SIZE])
    transform(record)
    return record


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("files", nargs="+", help="SocketItem_<lang>.bmd files to patch")
    parser.add_argument("--option-id", type=int, required=True, help="Option id inside the socket item options table")
    parser.add_argument("--values", required=True, help="Five comma separated values, level 1 to 5")
    parser.add_argument("--dry-run", action="store_true", help="Only print what would be changed")
    return parser.parse_args()


def main() -> None:
    args = parse_args()
    values = [int(value) for value in args.values.split(",")]
    if len(values) != 5:
        raise SystemExit("Exactly five values are expected, for example --values 1000,2000,3000,4000,5000")
    if args.option_id < 0 or args.option_id >= OPTIONS_PER_TABLE:
        raise SystemExit(f"The option id must be between 0 and {OPTIONS_PER_TABLE - 1}.")

    for file_path in args.files:
        if not os.path.isfile(file_path):
            raise SystemExit(f"File not found: {file_path}")

        with open(file_path, "rb") as file:
            content = file.read()

        expected_size = RECORD_SIZE * OPTIONS_PER_TABLE * 3
        if len(content) != expected_size:
            raise SystemExit(f"Unexpected file size of {file_path}: {len(content)} instead of {expected_size} bytes.")

        record = read_record(content, args.option_id, TABLE_SOCKET_ITEM_OPTIONS)
        name = record[NAME_OFFSET:NAME_OFFSET + NAME_LENGTH].split(b"\x00")[0].decode("utf-8", "replace")
        option_id = struct.unpack_from("<i", record, 0)[0]
        old_values = struct.unpack_from("<5i", record, VALUE_OFFSET)
        if option_id != args.option_id:
            raise SystemExit(f"The record {args.option_id} of {file_path} contains the option id {option_id}.")

        # The names of the other language files may contain characters which the console
        # encoding can not print, so they are reduced to ASCII here.
        printable_name = name.encode("ascii", "replace").decode("ascii")
        print(f"{file_path}: '{printable_name}' {old_values} -> {tuple(values)}")
        if args.dry_run:
            continue

        struct.pack_into("<5i", record, VALUE_OFFSET, *values)
        transform(record)  # XOR again to encrypt it
        with open(file_path, "r+b") as file:
            file.seek(record_offset(TABLE_SOCKET_ITEM_OPTIONS, args.option_id))
            file.write(record)

    if args.dry_run:
        print("Dry run: no file was changed.")
    else:
        print("Done. Keep the server configuration in sync with these values.")


if __name__ == "__main__":
    sys.exit(main())
