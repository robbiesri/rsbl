#!/usr/bin/env python3
# Copyright 2025 Robert Srinivasiah
# Licensed under the MIT License, see the LICENSE file for more info

"""
Download assets defined in asset_listing.toml to sample_assets/ directory.

Supports two types of downloads:
- glb: Single binary file download
- gltf: Directory download with all associated files (textures, etc.)
"""

import os
import sys
import urllib.request
import urllib.parse
from pathlib import Path
import re

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
    """Download a single file from URL to destination."""
    try:
        print_info(f"Downloading: {os.path.basename(destination)}")
        print_gray(f"From: {url}")

        urllib.request.urlretrieve(url, destination)

        file_size = os.path.getsize(destination)
        size_kb = file_size / 1024
        print_success(f"Downloaded {os.path.basename(destination)} ({size_kb:.1f} KB)")
        return True
    except Exception as e:
        print_error(f"Failed to download {url}: {e}")
        return False


def get_github_raw_url(tree_url):
    """Convert GitHub tree URL to raw content base URL."""
    # Example: https://github.com/user/repo/tree/branch/path
    # To: https://raw.githubusercontent.com/user/repo/branch/path

    match = re.match(r'https://github\.com/([^/]+)/([^/]+)/tree/([^/]+)/(.+)', tree_url)
    if match:
        user, repo, branch, path = match.groups()
        return f"https://raw.githubusercontent.com/{user}/{repo}/{branch}/{path}"
    return None


def get_github_api_url(tree_url):
    """Convert GitHub tree URL to API URL for listing directory contents."""
    # Example: https://github.com/user/repo/tree/branch/path
    # To: https://api.github.com/repos/user/repo/contents/path?ref=branch

    match = re.match(r'https://github\.com/([^/]+)/([^/]+)/tree/([^/]+)/(.+)', tree_url)
    if match:
        user, repo, branch, path = match.groups()
        return f"https://api.github.com/repos/{user}/{repo}/contents/{path}?ref={branch}"
    return None


def download_gltf_directory(tree_url, destination_dir):
    """Download entire glTF directory from GitHub."""
    import json

    api_url = get_github_api_url(tree_url)
    raw_base_url = get_github_raw_url(tree_url)

    if not api_url or not raw_base_url:
        print_error(f"Invalid GitHub URL format: {tree_url}")
        return False

    try:
        print_info(f"Fetching directory listing from GitHub...")
        print_gray(f"API URL: {api_url}")

        # Get directory listing from GitHub API
        with urllib.request.urlopen(api_url) as response:
            files = json.loads(response.read().decode())

        if not isinstance(files, list):
            print_error("Unexpected API response format")
            return False

        success = True
        for file_info in files:
            if file_info['type'] == 'file':
                file_name = file_info['name']
                file_url = f"{raw_base_url}/{file_name}"
                file_dest = os.path.join(destination_dir, file_name)

                if not download_file(file_url, file_dest):
                    success = False

        return success

    except Exception as e:
        print_error(f"Failed to download directory: {e}")
        return False


def download_assets():
    """Main function to download all assets from asset_listing.toml."""

    print_header("rsbl Asset Downloader")
    print()

    # Get paths
    script_dir = Path(__file__).parent
    repo_root = script_dir.parent
    toml_file = repo_root / "asset_listing.toml"
    assets_dir = repo_root / "sample_assets"

    # Check if TOML file exists
    if not toml_file.exists():
        print_error(f"asset_listing.toml not found at: {toml_file}")
        return 1

    # Load TOML
    print_info(f"Loading asset listing from: {toml_file}")
    try:
        with open(toml_file, 'rb') as f:
            config = tomllib.load(f)
    except Exception as e:
        print_error(f"Failed to parse TOML: {e}")
        return 1

    assets = config.get('assets', [])
    if not assets:
        print_info("No assets defined in asset_listing.toml")
        return 0

    print_success(f"Found {len(assets)} asset(s) to download")
    print()

    # Create assets directory
    assets_dir.mkdir(exist_ok=True)

    # Download each asset
    failed_assets = []
    for i, asset in enumerate(assets, 1):
        name = asset.get('name', f'asset_{i}')
        url = asset.get('url')
        asset_type = asset.get('type', 'glb')
        category = asset.get('category', 'uncategorized')
        description = asset.get('description', '')

        print(f"{Colors.CYAN}--- Asset {i}/{len(assets)}: {name} ---{Colors.NC}")
        if description:
            print_gray(f"Description: {description}")
        print_gray(f"Type: {asset_type}")
        print_gray(f"Category: {category}")
        print()

        if not url:
            print_error(f"No URL specified for asset: {name}")
            failed_assets.append(name)
            print()
            continue

        # Create category subdirectory
        asset_dest_dir = assets_dir / category / name
        asset_dest_dir.mkdir(parents=True, exist_ok=True)

        success = False
        if asset_type == 'glb':
            # Single file download
            file_name = f"{name}.glb"
            dest_file = asset_dest_dir / file_name
            success = download_file(url, dest_file)
        elif asset_type == 'gltf':
            # Directory download
            success = download_gltf_directory(url, asset_dest_dir)
        else:
            print_error(f"Unknown asset type: {asset_type}")
            failed_assets.append(name)

        if not success:
            failed_assets.append(name)

        print()

    # Summary
    print_header("Download Summary")
    print()

    total = len(assets)
    succeeded = total - len(failed_assets)

    print(f"Total assets: {total}")
    print(f"{Colors.GREEN}Succeeded: {succeeded}{Colors.NC}")
    if failed_assets:
        print(f"{Colors.RED}Failed: {len(failed_assets)}{Colors.NC}")
        print_gray(f"Failed assets: {', '.join(failed_assets)}")

    print()
    print_success(f"Assets downloaded to: {assets_dir}")
    print()

    return 0 if not failed_assets else 1


if __name__ == '__main__':
    sys.exit(download_assets())
