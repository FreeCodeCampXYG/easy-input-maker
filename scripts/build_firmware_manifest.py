#!/usr/bin/env python3
"""生成桌面烧录器使用的公开固件清单。"""

import argparse
import hashlib
import json
from pathlib import Path


EXPECTED_FILES = {
    "bootloader.bin": ("build/bootloader/bootloader.bin", "0x0"),
    "partition-table.bin": ("build/partition_table/partition-table.bin", "0x8000"),
    "easy_input_keyboard.bin": ("build/easy_input_keyboard.bin", "0x10000"),
}


def file_hash(path: Path) -> str:
    """按二进制计算 SHA-256，确保清单绑定本轮构建产物。"""
    digest = hashlib.sha256()
    with path.open("rb") as handle:
        for chunk in iter(lambda: handle.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--tag", required=True)
    parser.add_argument("--commit", required=True)
    parser.add_argument("--output", required=True, type=Path)
    args = parser.parse_args()

    files = []
    for name, (relative_path, offset) in EXPECTED_FILES.items():
        path = Path(relative_path)
        if not path.is_file():
            raise SystemExit(f"missing firmware artifact: {path}")
        files.append(
            {
                "name": name,
                "offset": offset,
                "sha256": file_hash(path),
                "size": path.stat().st_size,
            }
        )

    manifest = {
        "schemaVersion": 1,
        "product": "easyinput-firmware",
        "board": "easyinput-v2",
        "chip": "esp32s3",
        "tag": args.tag,
        "commit": args.commit,
        "idfVersion": "5.5.5",
        "releaseNotes": "GitHub Actions built firmware bundle for EasyInput V2.0.",
        "files": files,
    }
    args.output.write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")


if __name__ == "__main__":
    main()
