# Tools

This file documents small utility tools included in this repository, with usage and examples.

## tools/hexdump.py 🔧

Purpose: Export a binary file (or a portion of it) as a C-style byte array, useful for embedding firmware or generating test data.

Location: `tools/hexdump.py`

Basic usage:

  python3 tools/hexdump.py input_file [offset] [size] [options]

Arguments:

- input_file: Path to the input file (required)
- offset: Optional start offset (supports decimal or hex with 0x), default 0
- size: Optional number of bytes to read, default reads to end of file

Common options:

- -s, --size SIZE: Number of bytes to read (overrides positional size if provided)
- -o, --output FILE: Write output to FILE instead of stdout
- -n, --name NAME: If given, output as a C variable definition, e.g. `const uint8_t NAME[] = { ... };`
- -t, --type TYPE: C type to use (default `uint8_t`)
- -p, --per-line N: Number of bytes per line (default 12)
- -v, --var-length: When used with `-n`, also output `NAME_len` (size_t)

Examples:

- Print the file's byte list (12 bytes per line):

    python3 tools/hexdump.py firmware.bin

- Read 256 bytes from offset 0x1000 and output as a C array (8 bytes per line):

    python3 tools/hexdump.py firmware.bin 0x1000 256 -n firmware -p 8

- Read first 1024 bytes, output variable and its length:

    python3 tools/hexdump.py firmware.bin 0 1024 -n firmware -v

- Read 256 bytes and write the output to `firmware.c` as a C array:

    python3 tools/hexdump.py firmware.bin -s 256 -o firmware.c -n firmware -p 16

Notes:

- Output is written to stdout by default and can be redirected to a file.
- The script can be run with `python3 tools/hexdump.py` or made executable with `chmod +x tools/hexdump.py`.

---

If you'd like, I can also:
- make `tools/hexdump.py` executable, or
- add a short usage example to `README.md`, or
- add unit/integration tests for the tool

Tell me which you'd prefer and I'll proceed.