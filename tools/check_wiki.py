"""Verify docs/wiki/ against the plugin it documents.

Checks that every [[WikiLink]] has a page, every HP claim matches the panel
SVG, every module in plugin.json has a page, and (with --network) that every
image URL resolves.

Usage:
    python tools/check_wiki.py            # offline checks
    python tools/check_wiki.py --network  # also fetch every image URL
"""
from __future__ import annotations

import argparse
import json
import re
import sys
from pathlib import Path

PX_PER_HP = 15

WIKILINK = re.compile(r"\[\[([^\]|]+?)\]\]")
IMAGE = re.compile(r"!\[[^\]]*\]\((https://raw\.githubusercontent\.com/[^)]+)\)")
# The HP need not end the heading — Mass Driver trails its "(AF-01)" designation.
HP_HEADING = re.compile(r"^#\s+.+?—\s*(\d+)\s*HP\b", re.MULTILINE)
# The "Full parameter spec" link every module page carries into docs/modules/.
# Matched on the blob URL rather than the link text, because the text is the
# path and would match itself if someone wrote it without a link at all.
SPEC_LINK = re.compile(r"/blob/[^/]+/docs/modules/([A-Za-z0-9_-]+)\.md")
VIEWBOX = re.compile(r'viewBox="\s*[\d.]+\s+[\d.]+\s+([\d.]+)\s+[\d.]+\s*"')

# Pages that exist for structure, not content, and are never link targets.
PARTIALS = {"_Sidebar", "_Footer"}


def hp_from_svg(svg_path: Path) -> int:
    """Panel width in HP, from the SVG viewBox (15 px per HP)."""
    match = VIEWBOX.search(svg_path.read_text(encoding="utf-8"))
    if not match:
        raise ValueError(f"no viewBox in {svg_path}")
    return round(float(match.group(1)) / PX_PER_HP)


def page_name_for(module_name: str) -> str:
    """Wiki page name for a plugin.json module name."""
    return module_name.replace(" ", "-")


def find_wikilinks(text: str) -> list[str]:
    return WIKILINK.findall(text)


def find_image_urls(text: str) -> list[str]:
    return IMAGE.findall(text)


def hp_claim(text: str) -> int | None:
    """HP asserted by the page's first heading, or None if it makes no claim."""
    match = HP_HEADING.search(text)
    return int(match.group(1)) if match else None


def _modules(plugin_json: Path) -> list[dict]:
    return json.loads(plugin_json.read_text(encoding="utf-8"))["modules"]


def _pages(wiki_dir: Path) -> dict[str, Path]:
    return {p.stem: p for p in sorted(wiki_dir.glob("*.md"))}


def check_wikilinks(wiki_dir: Path) -> list[str]:
    pages = _pages(wiki_dir)
    errors = []
    for name, path in pages.items():
        for target in find_wikilinks(path.read_text(encoding="utf-8")):
            if target not in pages:
                errors.append(f"{path.name}: [[{target}]] has no page")
    return errors


def check_hp(wiki_dir: Path, res_dir: Path, plugin_json: Path) -> list[str]:
    errors = []
    for module in _modules(plugin_json):
        page = wiki_dir / f"{page_name_for(module['name'])}.md"
        svg = res_dir / f"{module['slug']}.svg"
        if not page.exists() or not svg.exists():
            continue  # absent pages are check_module_pages' business
        claimed = hp_claim(page.read_text(encoding="utf-8"))
        actual = hp_from_svg(svg)
        if claimed is None:
            errors.append(f"{page.name}: heading states no HP (expected {actual} HP)")
        elif claimed != actual:
            errors.append(
                f"{page.name}: heading says {claimed} HP, {svg.name} is {actual} HP"
            )
    return errors


def check_module_pages(wiki_dir: Path, plugin_json: Path) -> list[str]:
    pages = _pages(wiki_dir)
    errors = []
    for module in _modules(plugin_json):
        expected = page_name_for(module["name"])
        if expected not in pages:
            errors.append(f"module {module['slug']}: no wiki page {expected}.md")
    return errors


def check_module_refs(modules_dir: Path, plugin_json: Path) -> list[str]:
    """Every module has a parameter reference, and none is left behind.

    docs/modules/ is the second half of the manual - the wiki page is the
    narrative, this is the parameter spec it links out to - and until now
    nothing checked it, though CONTRIBUTING said otherwise. That is how a
    reference page went on describing a mode the module no longer had.
    """
    errors = []
    if not modules_dir.is_dir():
        return [f"{modules_dir.name}/: directory missing entirely"]

    present = {p.stem: p for p in sorted(modules_dir.glob("*.md"))}
    expected = {m["slug"] for m in _modules(plugin_json)}

    for slug in sorted(expected - set(present)):
        errors.append(f"module {slug}: no parameter reference docs/modules/{slug}.md")
    for stem in sorted(set(present) - expected):
        errors.append(
            f"docs/modules/{stem}.md: no module of that slug — renamed or removed?")
    return errors


def check_spec_links(wiki_dir: Path, modules_dir: Path) -> list[str]:
    """Each wiki page's "Full parameter spec" link must resolve.

    The link is a github.com blob URL, so a broken one is invisible offline and
    still renders as a link: the page looks complete and the destination 404s.
    """
    # Compared against the real directory listing, not Path.exists(). This is
    # developed on Windows, where the filesystem is case-insensitive, so
    # exists() says yes to docs/modules/Dronecore.md when the file is
    # DroneCore.md. github.com is case-sensitive: the link 404s for every
    # reader while passing the check on the machine that wrote it.
    actual = {p.stem for p in modules_dir.glob("*.md")}
    errors = []
    for path in sorted(wiki_dir.glob("*.md")):
        for target in SPEC_LINK.findall(path.read_text(encoding="utf-8")):
            if target not in actual:
                errors.append(
                    f"{path.name}: parameter spec link points at "
                    f"docs/modules/{target}.md, which does not exist")
    return errors


def check_image_urls(wiki_dir: Path) -> list[str]:
    import requests

    errors = []
    seen: dict[str, int] = {}
    for path in sorted(wiki_dir.glob("*.md")):
        for url in find_image_urls(path.read_text(encoding="utf-8")):
            if url not in seen:
                seen[url] = requests.head(url, timeout=20, allow_redirects=True).status_code
            if seen[url] != 200:
                errors.append(f"{path.name}: {url} returned {seen[url]}")
    return errors


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--network", action="store_true", help="also fetch image URLs")
    parser.add_argument(
        "--repo",
        type=Path,
        default=Path(__file__).resolve().parent.parent,
        help="plugin repo root (default: parent of tools/)",
    )
    args = parser.parse_args(argv)

    wiki_dir = args.repo / "docs" / "wiki"
    modules_dir = args.repo / "docs" / "modules"
    res_dir = args.repo / "res"
    plugin_json = args.repo / "plugin.json"

    errors = []
    errors += check_module_pages(wiki_dir, plugin_json)
    errors += check_hp(wiki_dir, res_dir, plugin_json)
    errors += check_wikilinks(wiki_dir)
    errors += check_module_refs(modules_dir, plugin_json)
    errors += check_spec_links(wiki_dir, modules_dir)
    if args.network:
        errors += check_image_urls(wiki_dir)

    for error in errors:
        print(f"FAIL  {error}")
    if errors:
        print(f"\n{len(errors)} problem(s).")
        return 1
    pages = len(list(wiki_dir.glob("*.md")))
    refs = len(list(modules_dir.glob("*.md")))
    modules = len(_modules(plugin_json))
    print(f"OK  {pages} pages, {refs} parameter refs, {modules} modules, "
          f"HP and links consistent.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
