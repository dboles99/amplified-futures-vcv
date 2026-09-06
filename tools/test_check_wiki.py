"""Tests for check_wiki.py."""
import json
from pathlib import Path

import pytest

from check_wiki import (
    check_hp,
    check_module_pages,
    check_module_refs,
    check_spec_links,
    check_wikilinks,
    find_image_urls,
    find_wikilinks,
    hp_claim,
    hp_from_svg,
    page_name_for,
)

SVG = '<svg width="120mm" viewBox="0 0 120 380" id="svg1"></svg>'


def write(path: Path, text: str) -> Path:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(text, encoding="utf-8")
    return path


@pytest.fixture
def plugin_json(tmp_path: Path) -> Path:
    data = {
        "slug": "amplified-futures",
        "version": "2.1.0",
        "modules": [
            {"slug": "DroneCore", "name": "DroneCore"},
            {"slug": "StringMassCore", "name": "String Mass Core"},
        ],
    }
    return write(tmp_path / "plugin.json", json.dumps(data))


def test_hp_from_svg_divides_viewbox_width_by_fifteen(tmp_path):
    svg = write(tmp_path / "DroneCore.svg", SVG)
    assert hp_from_svg(svg) == 8


def test_hp_from_svg_handles_wide_panels(tmp_path):
    svg = write(tmp_path / "SitarGrid.svg", '<svg viewBox="0 0 630 380"></svg>')
    assert hp_from_svg(svg) == 42


def test_page_name_for_hyphenates_spaces():
    assert page_name_for("String Mass Core") == "String-Mass-Core"
    assert page_name_for("DroneCore") == "DroneCore"
    assert page_name_for("Swarm Core") == "Swarm-Core"


def test_find_wikilinks_extracts_targets():
    text = "See [[DroneCore]] and [[String-Mass-Core]] for detail."
    assert find_wikilinks(text) == ["DroneCore", "String-Mass-Core"]


def test_find_image_urls_extracts_raw_github_urls():
    text = "![x](https://raw.githubusercontent.com/a/b/master/docs/panels/X.png)"
    assert find_image_urls(text) == [
        "https://raw.githubusercontent.com/a/b/master/docs/panels/X.png"
    ]


def test_find_image_urls_ignores_shields_badges():
    text = "[![Build](https://img.shields.io/badge/x-y-orange)](https://example.com)"
    assert find_image_urls(text) == []


@pytest.mark.parametrize(
    "heading,expected",
    [
        ("# DroneCore — 8HP", 8),
        ("# Swarm Core — 18 HP", 18),
        ("# Mass Driver (AF-01) — 32HP", 32),
        ("# Mass Driver — 32HP (AF-01)", 32),  # designation may trail the HP
        ("# Playbooks", None),
        ("# Music Theory — the harmonic series", None),
    ],
)
def test_hp_claim_reads_the_first_heading(heading, expected):
    assert hp_claim(heading + "\n\nbody text\n") == expected


def test_check_wikilinks_flags_a_missing_target(tmp_path):
    write(tmp_path / "Home.md", "Go to [[DroneCore]] then [[Nowhere]].")
    write(tmp_path / "DroneCore.md", "# DroneCore — 8HP")
    errors = check_wikilinks(tmp_path)
    assert len(errors) == 1
    assert "Nowhere" in errors[0]
    assert "Home.md" in errors[0]


def test_check_wikilinks_passes_when_all_targets_exist(tmp_path):
    write(tmp_path / "Home.md", "Go to [[DroneCore]].")
    write(tmp_path / "DroneCore.md", "# DroneCore — 8HP")
    assert check_wikilinks(tmp_path) == []


def test_check_hp_flags_a_wrong_claim(tmp_path, plugin_json):
    res = tmp_path / "res"
    write(res / "DroneCore.svg", SVG)
    write(res / "StringMassCore.svg", '<svg viewBox="0 0 240 380"></svg>')
    wiki = tmp_path / "wiki"
    write(wiki / "DroneCore.md", "# DroneCore — 14HP\n")
    write(wiki / "String-Mass-Core.md", "# String Mass Core — 16HP\n")
    errors = check_hp(wiki, res, plugin_json)
    assert len(errors) == 1
    assert "DroneCore" in errors[0]
    assert "14" in errors[0] and "8" in errors[0]


def test_check_hp_passes_when_claims_match_svgs(tmp_path, plugin_json):
    res = tmp_path / "res"
    write(res / "DroneCore.svg", SVG)
    write(res / "StringMassCore.svg", '<svg viewBox="0 0 240 380"></svg>')
    wiki = tmp_path / "wiki"
    write(wiki / "DroneCore.md", "# DroneCore — 8 HP\n")
    write(wiki / "String-Mass-Core.md", "# String Mass Core — 16 HP\n")
    assert check_hp(wiki, res, plugin_json) == []


def test_check_module_pages_flags_a_module_with_no_page(tmp_path, plugin_json):
    wiki = tmp_path / "wiki"
    write(wiki / "DroneCore.md", "# DroneCore — 8 HP\n")
    errors = check_module_pages(wiki, plugin_json)
    assert len(errors) == 1
    assert "String-Mass-Core" in errors[0]


def test_check_module_pages_passes_with_a_page_per_module(tmp_path, plugin_json):
    wiki = tmp_path / "wiki"
    write(wiki / "DroneCore.md", "# DroneCore — 8 HP\n")
    write(wiki / "String-Mass-Core.md", "# String Mass Core — 16 HP\n")
    assert check_module_pages(wiki, plugin_json) == []


BLOB = "https://github.com/dboles99/amplified-futures-vcv/blob/master/docs/modules"


def test_check_module_refs_flags_a_module_with_no_reference(tmp_path, plugin_json):
    mods = tmp_path / "modules"
    write(mods / "DroneCore.md", "# DroneCore\n")
    errors = check_module_refs(mods, plugin_json)
    assert len(errors) == 1
    assert "StringMassCore" in errors[0]


def test_check_module_refs_flags_an_orphan_reference(tmp_path, plugin_json):
    """A reference for a module that no longer exists - how MICRO went stale."""
    mods = tmp_path / "modules"
    write(mods / "DroneCore.md", "# DroneCore\n")
    write(mods / "StringMassCore.md", "# String Mass Core\n")
    write(mods / "Micro.md", "# Micro\n")
    errors = check_module_refs(mods, plugin_json)
    assert len(errors) == 1
    assert "Micro.md" in errors[0]


def test_check_module_refs_flags_the_directory_going_missing(tmp_path, plugin_json):
    errors = check_module_refs(tmp_path / "modules", plugin_json)
    assert len(errors) == 1
    assert "missing entirely" in errors[0]


def test_check_module_refs_passes_with_a_reference_per_module(tmp_path, plugin_json):
    mods = tmp_path / "modules"
    write(mods / "DroneCore.md", "# DroneCore\n")
    write(mods / "StringMassCore.md", "# String Mass Core\n")
    assert check_module_refs(mods, plugin_json) == []


def test_check_spec_links_flags_a_link_to_nothing(tmp_path):
    wiki, mods = tmp_path / "wiki", tmp_path / "modules"
    write(mods / "DroneCore.md", "# DroneCore\n")
    write(wiki / "DroneCore.md", f"**Full parameter spec:** [x]({BLOB}/Dronecore.md)\n")
    errors = check_spec_links(wiki, mods)
    assert len(errors) == 1
    assert "Dronecore.md" in errors[0]


def test_check_spec_links_passes_when_the_target_exists(tmp_path):
    wiki, mods = tmp_path / "wiki", tmp_path / "modules"
    write(mods / "DroneCore.md", "# DroneCore\n")
    write(wiki / "DroneCore.md", f"**Full parameter spec:** [x]({BLOB}/DroneCore.md)\n")
    assert check_spec_links(wiki, mods) == []


def test_check_spec_links_ignores_a_page_with_no_spec_link(tmp_path):
    wiki, mods = tmp_path / "wiki", tmp_path / "modules"
    write(wiki / "Home.md", "# Home\n\nNo parameter spec here.\n")
    assert check_spec_links(wiki, mods) == []
