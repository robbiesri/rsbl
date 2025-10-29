#!/usr/bin/env python3
# Copyright Robert Srinivasiah 2025
# Licensed under the MIT License
"""
Git subtree setup script for rsbl.
Manages remote dependencies as git subtrees.
"""

import os
import sys
import subprocess
import json
from pathlib import Path
from typing import List, Dict, Optional


class GitSubtreeManager:
    """Manages git subtree operations for remote dependencies."""

    def __init__(self, repo_root: Path):
        self.repo_root = repo_root
        self.config_file = repo_root / "subtrees.json"

    def run_command(self, cmd: List[str], cwd: Optional[Path] = None) -> tuple[int, str, str]:
        """Run a command and return exit code, stdout, and stderr."""
        try:
            result = subprocess.run(
                cmd,
                cwd=cwd or self.repo_root,
                capture_output=True,
                text=True,
                encoding='utf-8'
            )
            return result.returncode, result.stdout, result.stderr
        except Exception as e:
            return 1, "", str(e)

    def check_git_repo(self) -> bool:
        """Check if the current directory is a git repository."""
        returncode, _, _ = self.run_command(["git", "rev-parse", "--git-dir"])
        return returncode == 0

    def load_config(self) -> Dict:
        """Load subtree configuration from JSON file."""
        if not self.config_file.exists():
            print(f"[WARNING] Configuration file not found: {self.config_file}")
            print("Creating example configuration file...")
            self.create_example_config()
            return self.load_config()

        try:
            with open(self.config_file, 'r') as f:
                return json.load(f)
        except json.JSONDecodeError as e:
            print(f"[ERROR] Invalid JSON in configuration file: {e}")
            sys.exit(1)

    def create_example_config(self):
        """Create an example subtrees.json configuration file."""
        example_config = {
            "subtrees": [
                {
                    "name": "example-lib",
                    "remote": "https://github.com/example/example-lib.git",
                    "prefix": "external/example-lib",
                    "branch": "main",
                    "enabled": False
                }
            ]
        }

        with open(self.config_file, 'w') as f:
            json.dump(example_config, f, indent=2)

        print(f"[OK] Created example configuration at: {self.config_file}")
        print("     Edit this file to add your dependencies.")

    def add_subtree(self, name: str, remote: str, prefix: str, branch: str = "main"):
        """Add a git subtree for a remote dependency."""
        print(f"\n--- Adding subtree: {name} ---")
        print(f"  Remote: {remote}")
        print(f"  Prefix: {prefix}")
        print(f"  Branch: {branch}")

        # Check if prefix already exists
        prefix_path = self.repo_root / prefix
        if prefix_path.exists():
            print(f"[WARNING] Directory already exists: {prefix}")
            print("          Skipping subtree add. Use update to refresh.")
            return False

        # Add remote if it doesn't exist
        print(f"\nAdding remote '{name}'...")
        returncode, stdout, stderr = self.run_command([
            "git", "remote", "add", name, remote
        ])

        if returncode != 0 and "already exists" not in stderr:
            print(f"[ERROR] Failed to add remote: {stderr}")
            return False

        # Fetch from remote
        print(f"Fetching from remote '{name}'...")
        returncode, stdout, stderr = self.run_command([
            "git", "fetch", name, branch
        ])

        if returncode != 0:
            print(f"[ERROR] Failed to fetch from remote: {stderr}")
            return False

        # Add subtree
        print(f"Adding subtree to '{prefix}'...")
        returncode, stdout, stderr = self.run_command([
            "git", "subtree", "add",
            "--prefix", prefix,
            name, branch,
            "--squash"
        ])

        if returncode != 0:
            print(f"[ERROR] Failed to add subtree: {stderr}")
            return False

        print(f"[OK] Successfully added subtree '{name}'")
        return True

    def update_subtree(self, name: str, prefix: str, branch: str = "main"):
        """Update an existing git subtree."""
        print(f"\n--- Updating subtree: {name} ---")
        print(f"  Prefix: {prefix}")
        print(f"  Branch: {branch}")

        # Check if prefix exists
        prefix_path = self.repo_root / prefix
        if not prefix_path.exists():
            print(f"[WARNING] Directory does not exist: {prefix}")
            print("          Use add to create the subtree first.")
            return False

        # Fetch from remote
        print(f"Fetching from remote '{name}'...")
        returncode, stdout, stderr = self.run_command([
            "git", "fetch", name, branch
        ])

        if returncode != 0:
            print(f"[ERROR] Failed to fetch from remote: {stderr}")
            return False

        # Pull subtree updates
        print(f"Pulling updates for subtree '{prefix}'...")
        returncode, stdout, stderr = self.run_command([
            "git", "subtree", "pull",
            "--prefix", prefix,
            name, branch,
            "--squash"
        ])

        if returncode != 0:
            print(f"[ERROR] Failed to update subtree: {stderr}")
            return False

        print(f"[OK] Successfully updated subtree '{name}'")
        return True

    def setup_all_subtrees(self, update: bool = False):
        """Set up all subtrees defined in the configuration."""
        config = self.load_config()
        subtrees = config.get("subtrees", [])

        if not subtrees:
            print("[WARNING] No subtrees defined in configuration")
            return

        enabled_subtrees = [s for s in subtrees if s.get("enabled", True)]

        if not enabled_subtrees:
            print("[INFO] No enabled subtrees in configuration")
            return

        print(f"\nFound {len(enabled_subtrees)} enabled subtree(s) to process")

        success_count = 0
        for subtree in enabled_subtrees:
            name = subtree.get("name")
            remote = subtree.get("remote")
            prefix = subtree.get("prefix")
            branch = subtree.get("branch", "main")

            if not all([name, remote, prefix]):
                print(f"[WARNING] Skipping invalid subtree entry: {subtree}")
                continue

            if update:
                if self.update_subtree(name, prefix, branch):
                    success_count += 1
            else:
                if self.add_subtree(name, remote, prefix, branch):
                    success_count += 1

        print(f"\n{'=' * 60}")
        print(f"Processed {success_count}/{len(enabled_subtrees)} subtree(s) successfully")
        print(f"{'=' * 60}")


def main():
    """Main entry point."""
    print("=" * 60)
    print("rsbl Git Subtree Setup")
    print("=" * 60)

    # Determine repository root
    script_dir = Path(__file__).parent
    repo_root = script_dir.parent

    manager = GitSubtreeManager(repo_root)

    # Check if we're in a git repository
    if not manager.check_git_repo():
        print("\n[ERROR] Not a git repository!")
        print("Initialize a git repository first with: git init")
        sys.exit(1)

    # Parse command line arguments
    update_mode = "--update" in sys.argv or "-u" in sys.argv

    if update_mode:
        print("\nMode: UPDATE")
        print("Will update existing subtrees")
    else:
        print("\nMode: ADD")
        print("Will add new subtrees")

    print()

    # Set up subtrees
    manager.setup_all_subtrees(update=update_mode)

    print()


if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        print("\n\n[CANCELLED] Operation interrupted by user")
        sys.exit(1)
    except Exception as e:
        print(f"\n[ERROR] Unexpected error: {e}")
        import traceback
        traceback.print_exc()
        sys.exit(1)
