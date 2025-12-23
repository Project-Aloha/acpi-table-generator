#!/usr/bin/env python3
"""hexdump.py

Generate a C-style hex byte array, for example:
  0xFF, 0x00, 0x12, ...

Usage:
  hexdump.py input_file [offset] [size]

Arguments:
  input_file   Path to the input file
  offset       Optional start offset (supports hex with 0x prefix), default 0
  size         Optional number of bytes to read, default to end of file

Options:
  -n, --name NAME     If provided, output a C definition: "const uint8_t NAME[] = { ... };"
  -t, --type TYPE     C data type to use (default "uint8_t")
  -p, --per-line N    Number of bytes per line (default 12)
  -v, --var-length    When used with -n, also print NAME_len (size_t)

Examples:
  python3 tools/hexdump.py firmware.bin 0x1000 256 -n firmware -p 8
  python3 tools/hexdump.py firmware.bin -s 256 -o firmware.c -n firmware
"""

from __future__ import annotations
import argparse
import os
import sys
from typing import Optional


def parse_int(s: str) -> int:
    try:
        return int(s, 0)
    except Exception as e:
        raise argparse.ArgumentTypeError(f"Invalid integer: {s}") from e


def format_bytes(data: bytes, per_line: int = 12) -> str:
    parts = [f"0x{b:02X}" for b in data]
    lines = []
    for i in range(0, len(parts), per_line):
        lines.append("  " + ", ".join(parts[i:i+per_line]))
    return ",\n".join(lines)


def main() -> int:
    parser = argparse.ArgumentParser(description="Generate a C-style array of hex bytes")
    parser.add_argument("input_file", help="Input file path")
    parser.add_argument("offset", nargs="?", type=parse_int, default=0, help="Start offset (supports 0x hex)")
    parser.add_argument("size", nargs="?", type=parse_int, default=None, help="Number of bytes to read")
    parser.add_argument("-s", "--size", dest="size_opt", type=parse_int, default=None, help="Number of bytes to read (overrides positional size if provided)")
    parser.add_argument("-o", "--output", dest="output", default=None, help="Write output to a file (default stdout)")
    parser.add_argument("-n", "--name", dest="name", default=None, help="If provided, output as C variable name")
    parser.add_argument("-t", "--type", dest="ctype", default="uint8_t", help="C data type to use (default uint8_t)")
    parser.add_argument("-p", "--per-line", dest="per_line", type=int, default=12, help="Bytes per line (default 12)")
    parser.add_argument("-v", "--var-length", dest="var_length", action="store_true", help="With -n, also output variable length (NAME_len)")

    args = parser.parse_args()

    if not os.path.isfile(args.input_file):
        print(f"Error: file not found: {args.input_file}", file=sys.stderr)
        return 2

    try:
        file_size = os.path.getsize(args.input_file)
        if args.offset < 0:
            raise ValueError("offset must be >= 0")
        if args.offset > file_size:
            print(f"Warning: offset beyond end of file (size {file_size}), no data to read", file=sys.stderr)
            data = b""
        else:
            max_read = file_size - args.offset
            # Prefer --size/-s if provided, otherwise positional size, otherwise read to EOF
            read_size = args.size_opt if getattr(args, 'size_opt', None) is not None else (args.size if args.size is not None else max_read)
            if read_size < 0:
                raise ValueError("size must be >= 0")
            read_size = min(read_size, max_read)
            with open(args.input_file, "rb") as f:
                f.seek(args.offset)
                data = f.read(read_size)
    except Exception as e:
        print(f"Failed to read file: {e}", file=sys.stderr)
        return 3

    if len(data) == 0:
        # Output empty array text
        if args.name:
            out = f"const {args.ctype} {args.name}[] = {{}};"
            if args.var_length:
                out += f"\nsize_t {args.name}_len = 0;"
        else:
            out = ""
        # write to file or stdout
        if args.output:
            try:
                with open(args.output, "w", encoding="utf-8") as outf:
                    outf.write(out + ("\n" if out and not out.endswith("\n") else ""))
            except Exception as e:
                print(f"Failed to write output file: {e}", file=sys.stderr)
                return 4
        else:
            print(out)
        return 0

    body = format_bytes(data, per_line=args.per_line)

    if args.name:
        out_lines = [f"const {args.ctype} {args.name}[] = {{", body + ",", "};"]
        if args.var_length:
            out_lines.append(f"size_t {args.name}_len = sizeof({args.name});")
        out = "\n".join(out_lines)
    else:
        # Only output comma-separated bytes suitable for pasting into existing code
        out = body + ("," if body and not body.endswith(",") else "")

    if args.output:
        try:
            with open(args.output, "w", encoding="utf-8") as outf:
                outf.write(out + ("\n" if out and not out.endswith("\n") else ""))
        except Exception as e:
            print(f"Failed to write output file: {e}", file=sys.stderr)
            return 4
    else:
        print(out)

    return 0


if __name__ == "__main__":
    sys.exit(main())
