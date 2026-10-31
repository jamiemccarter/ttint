#!/usr/bin/env python3
"""Verify each ```cpp block in README.md matches the example file named by its fence.

The fence names its file in the info string: ```cpp example=NAME.cpp
Each example also needs a screenshot at .github/images/NAME.png, referenced from the README.
Exits non-zero if a block differs from examples/NAME.cpp, has no example= tag, or lacks its image.
"""

import pathlib
import re
import sys

ROOT = pathlib.Path(__file__).resolve().parent.parent
BLOCK = re.compile(r"```cpp example=(\S+)\n(.*?)```", re.S)


def main() -> int:
    readme = (ROOT / "README.md").read_text()
    status = 0
    marked = 0
    for name, body in BLOCK.findall(readme):
        marked += 1
        path = ROOT / "examples" / name
        if not path.exists() or path.read_text() != body:
            print(f"README example out of sync: {name}")
            status = 1
        image = f".github/images/{pathlib.Path(name).stem}.png"
        if not (ROOT / image).exists():
            print(f"missing screenshot: {image}")
            status = 1
        elif image not in readme:
            print(f"README does not reference {image}")
            status = 1
    unmarked = len(re.findall(r"```cpp\b", readme)) - marked
    if unmarked:
        print(f"{unmarked} README cpp block(s) lack an example=NAME.cpp tag")
        status = 1
    return status


if __name__ == "__main__":
    sys.exit(main())
