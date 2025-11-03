#!/usr/bin/env python3
# Copyright 2025 Robert Srinivasiah
# Licensed under the MIT License, see the LICENSE file for more info

"""
Download external dependencies defined in external_deps.toml.

Downloads release archives and extracts them to external/{name}/ directory.
"""

import os
import sys
import urllib.request
import shutil
import zipfile
import tarfile
from pathlib import Path

# Try to import tomllib (Python 3.11+) or tomli as fallback
try:
    import tomllib
except ModuleNotFoundError:
    try:
        import tomli as tomllib
    except ModuleNotFoundError:
        print("ERROR: Neither tomllib (Python 3.11+) nor tomli package found.")
        print("Please upgrade to Python 3.11+ or install tomli: pip install tomli")
        sys.exit(1)


# ANSI color codes
class Colors:
    CYAN = '\033[0;36m'
    GREEN = '\033[0;32m'
    YELLOW = '\033[0;33m'
    RED = '\033[0;31m'
    GRAY = '\033[0;90m'
    NC = '\033[0m'  # No Color


def print_header(text):
    print(f"{Colors.CYAN}{'=' * 60}{Colors.NC}")
    print(f"{Colors.CYAN}{text}{Colors.NC}")
    print(f"{Colors.CYAN}{'=' * 60}{Colors.NC}")


def print_success(text):
    print(f"{Colors.GREEN}[OK] {text}{Colors.NC}")


def print_info(text):
    print(f"{Colors.YELLOW}{text}{Colors.NC}")


def print_error(text):
    print(f"{Colors.RED}[ERROR] {text}{Colors.NC}")


def print_gray(text):
    print(f"{Colors.GRAY}  {text}{Colors.NC}")


def download_file(url, destination):
    """Download a file from URL to destination."""
    try:
        print_info(f"Downloading from: {url}")

        urllib.request.urlretrieve(url, destination)

        file_size = os.path.getsize(destination)
        size_mb = file_size / (1024 * 1024)
        print_success(f"Downloaded ({size_mb:.2f} MB)")
        return True
    except Exception as e:
        print_error(f"Failed to download: {e}")
        return False


def extract_archive(archive_path, dest_dir, strip_components=0):
    """Extract an archive (zip or tar.gz) to destination directory."""
    try:
        archive_path = Path(archive_path)
        dest_dir = Path(dest_dir)

        print_info(f"Extracting archive...")

        # Determine archive type
        if archive_path.suffix == '.zip' or archive_path.name.endswith('.zip'):
            return extract_zip(archive_path, dest_dir, strip_components)
        elif archive_path.name.endswith('.tar.gz') or archive_path.name.endswith('.tgz'):
            return extract_tar(archive_path, dest_dir, strip_components)
        else:
            print_error(f"Unsupported archive format: {archive_path.suffix}")
            return False

    except Exception as e:
        print_error(f"Failed to extract archive: {e}")
        return False


def extract_zip(archive_path, dest_dir, strip_components=0):
    """Extract a zip archive."""
    with zipfile.ZipFile(archive_path, 'r') as zip_ref:
        members = zip_ref.namelist()

        for member in members:
            # Split path and remove specified number of components
            parts = member.split('/')
            if len(parts) <= strip_components:
                continue  # Skip if not enough path components

            new_path = '/'.join(parts[strip_components:])
            if not new_path:  # Skip if path becomes empty
                continue

            # Create target path
            target_path = dest_dir / new_path

            # Extract directory or file
            if member.endswith('/'):
                target_path.mkdir(parents=True, exist_ok=True)
            else:
                target_path.parent.mkdir(parents=True, exist_ok=True)
                with zip_ref.open(member) as source, open(target_path, 'wb') as target:
                    shutil.copyfileobj(source, target)

    print_success(f"Extracted to: {dest_dir}")
    return True


def extract_tar(archive_path, dest_dir, strip_components=0):
    """Extract a tar archive."""
    with tarfile.open(archive_path, 'r:*') as tar_ref:
        members = tar_ref.getmembers()

        for member in members:
            # Split path and remove specified number of components
            parts = member.name.split('/')
            if len(parts) <= strip_components:
                continue

            new_path = '/'.join(parts[strip_components:])
            if not new_path:
                continue

            # Update member name
            member.name = new_path
            tar_ref.extract(member, dest_dir)

    print_success(f"Extracted to: {dest_dir}")
    return True


def download_dependency(name, url, version, dest_dir, strip_components=0):
    """Download and extract a dependency."""
    dest_dir = Path(dest_dir)

    # Check if already exists
    if dest_dir.exists() and any(dest_dir.iterdir()):
        print_success(f"{name} already exists at: {dest_dir}")
        return True

    # Create temporary download directory
    temp_dir = dest_dir.parent / f".{name}_download"
    temp_dir.mkdir(parents=True, exist_ok=True)

    try:
        # Determine archive filename from URL
        archive_name = url.split('/')[-1]
        if not archive_name:
            archive_name = f"{name}.zip"
        archive_path = temp_dir / archive_name

        # Download
        if not download_file(url, archive_path):
            return False

        # Create destination directory
        dest_dir.mkdir(parents=True, exist_ok=True)

        # Extract
        if not extract_archive(archive_path, dest_dir, strip_components):
            return False

        return True

    finally:
        # Clean up temp directory
        if temp_dir.exists():
            shutil.rmtree(temp_dir)


def download_dependencies():
    """Main function to download all dependencies from external_deps.toml."""

    print_header("rsbl External Dependencies Downloader")
    print()

    # Get paths
    script_dir = Path(__file__).parent
    repo_root = script_dir.parent
    toml_file = repo_root / "external_deps.toml"
    external_dir = repo_root / "external"

    # Check if TOML file exists
    if not toml_file.exists():
        print_error(f"external_deps.toml not found at: {toml_file}")
        return 1

    # Load TOML
    print_info(f"Loading dependencies from: {toml_file}")
    try:
        with open(toml_file, 'rb') as f:
            config = tomllib.load(f)
    except Exception as e:
        print_error(f"Failed to parse TOML: {e}")
        return 1

    dependencies = config.get('dependencies', [])
    if not dependencies:
        print_info("No dependencies defined in external_deps.toml")
        return 0

    # Filter enabled dependencies
    enabled_deps = [d for d in dependencies if d.get('enabled', True)]

    if not enabled_deps:
        print_info("No enabled dependencies in external_deps.toml")
        return 0

    print_success(f"Found {len(enabled_deps)} enabled dependenc{'y' if len(enabled_deps) == 1 else 'ies'}")
    print()

    # Download each dependency
    failed_deps = []
    for i, dep in enumerate(enabled_deps, 1):
        name = dep.get('name', f'dep_{i}')
        url = dep.get('url')
        version = dep.get('version', 'unknown')
        strip_components = dep.get('strip_components', 0)

        print(f"{Colors.CYAN}--- Dependency {i}/{len(enabled_deps)}: {name} ---{Colors.NC}")
        print_gray(f"Version: {version}")
        print()

        if not url:
            print_error(f"No URL specified for dependency: {name}")
            failed_deps.append(name)
            print()
            continue

        dest_dir = external_dir / name

        if not download_dependency(name, url, version, dest_dir, strip_components):
            failed_deps.append(name)

        print()

    # Summary
    print_header("Download Summary")
    print()

    total = len(enabled_deps)
    succeeded = total - len(failed_deps)

    print(f"Total dependencies: {total}")
    print(f"{Colors.GREEN}Succeeded: {succeeded}{Colors.NC}")
    if failed_deps:
        print(f"{Colors.RED}Failed: {len(failed_deps)}{Colors.NC}")
        print_gray(f"Failed dependencies: {', '.join(failed_deps)}")

    print()
    print_success(f"Dependencies extracted to: {external_dir}")
    print()

    return 0 if not failed_deps else 1


if __name__ == '__main__':
    sys.exit(download_dependencies())
