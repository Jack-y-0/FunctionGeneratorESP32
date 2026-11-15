"""Exercise merge versioning in disposable Git repositories."""

import importlib.util
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest


ROOT = Path(__file__).resolve().parents[1]
SPEC = importlib.util.spec_from_file_location("bump_version", ROOT / "scripts/bump_version.py")
VERSIONING = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(VERSIONING)
FILES = ["VERSION", "functiongenerator_esp32/functiongenerator_esp32.ino",
         "functiongenerator_web/src/main.cpp"]


class VersioningTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        for name in FILES:
            target = self.root / name
            target.parent.mkdir(parents=True, exist_ok=True)
            shutil.copyfile(ROOT / name, target)
        self.git("init", "-q")
        self.git("config", "user.name", "Version test")
        self.git("config", "user.email", "version@example.invalid")
        self.commit("Initial version")

    def git(self, *args):
        return subprocess.check_output(["git", *args], cwd=self.root, text=True)

    def commit(self, subject):
        self.git("add", ".")
        self.git("commit", "-qm", subject)

    def contents(self):
        return {name: (self.root / name).read_text() for name in FILES}

    def test_patch_bump_updates_both_firmware_identifiers(self):
        current = (self.root / "VERSION").read_text().strip()
        major, minor, patch = map(int, current.split("."))
        expected = f"{major}.{minor}.{patch + 1}"
        self.assertEqual(VERSIONING.bump(self.root, 1), expected)
        data = self.contents()
        self.assertEqual(data["VERSION"], expected + "\n")
        self.assertIn(f'#define VERSION " V {expected} "', data[FILES[1]])
        self.assertIn(f'FIRMWARE_BUILD_ID = "{expected}";', data[FILES[2]])

    def test_rerun_does_not_bump_twice(self):
        VERSIONING.bump(self.root, 1)
        self.commit(VERSIONING.commit_subject(1))
        before = self.contents()
        self.assertIsNone(VERSIONING.bump(self.root, 1))
        self.assertEqual(self.contents(), before)
        self.assertEqual(self.git("status", "--porcelain"), "")

    def test_different_prs_each_increment_even_with_shared_number_prefix(self):
        first = VERSIONING.bump(self.root, 10)
        self.commit(VERSIONING.commit_subject(10))
        second = VERSIONING.bump(self.root, 1)
        self.assertEqual(int(second.split(".")[2]), int(first.split(".")[2]) + 1)

    def test_missing_definition_fails_without_partial_updates(self):
        (self.root / FILES[2]).write_text("// Version definition removed\n")
        before = self.contents()
        with self.assertRaisesRegex(ValueError, "Expected one version definition"):
            VERSIONING.bump(self.root, 1)
        self.assertEqual(self.contents(), before)

    def test_invalid_version_fails_without_partial_updates(self):
        (self.root / "VERSION").write_text("not-a-version\n")
        before = self.contents()
        with self.assertRaisesRegex(ValueError, "Invalid VERSION"):
            VERSIONING.bump(self.root, 1)
        self.assertEqual(self.contents(), before)


if __name__ == "__main__":
    unittest.main()
