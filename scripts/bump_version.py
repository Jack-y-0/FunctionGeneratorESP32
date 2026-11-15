"""Bump the patch version once per merged PR and sync firmware identifiers."""

import argparse
from pathlib import Path
import re
import subprocess


ROOT = Path(__file__).resolve().parents[1]


def commit_subject(pr):
    return f"chore: bump version after PR #{pr}"


def bump(root, pr):
    if pr < 1:
        raise ValueError("PR number must be positive")
    subject = commit_subject(pr)
    history = subprocess.check_output(
        ["git", "log", "--first-parent", "--format=%s", "--fixed-strings",
         f"--grep={subject}"], cwd=root, text=True
    ).splitlines()
    if subject in history:
        print(f"PR #{pr} already has a version bump")
        return None

    version_path = root / "VERSION"
    current = version_path.read_text().strip()
    if not re.fullmatch(r"(0|[1-9][0-9]*)\.(0|[1-9][0-9]*)\.(0|[1-9][0-9]*)", current):
        raise ValueError(f"Invalid VERSION: {current!r}")
    major, minor, patch = map(int, current.split("."))
    version = f"{major}.{minor}.{patch + 1}"
    replacements = [
        ("functiongenerator_esp32/functiongenerator_esp32.ino",
         r'(?m)^(#define VERSION )"[^"\n]*"', f'\\g<1>" V {version} "'),
        ("functiongenerator_web/src/main.cpp",
         r'(?m)^(static constexpr const char \*FIRMWARE_BUILD_ID = )"[^"\n]*";',
         f'\\g<1>"{version}";'),
    ]
    updates = [(version_path, version + "\n")]
    # Validate every target before modifying any file.
    for name, pattern, replacement in replacements:
        path = root / name
        updated, count = re.subn(pattern, replacement, path.read_text())
        if count != 1:
            raise ValueError(f"Expected one version definition in {name}, found {count}")
        updates.append((path, updated))
    for path, content in updates:
        path.write_text(content)
    print(version)
    return version


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--pr", type=int, required=True)
    args = parser.parse_args()
    bump(ROOT, args.pr)
