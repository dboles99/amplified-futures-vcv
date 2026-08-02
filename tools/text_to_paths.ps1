# text_to_paths.ps1 — convert every <text> element in a panel to bezier outlines.
#
# nanosvg, which Rack uses to render panels, supports neither text nor fonts.
# A panel shipped with <text> in it renders with those labels missing entirely —
# which is how ten of fourteen panels shipped unreadable in July.
#
# make_panel.py emits <text> because that is the only sane way to author a
# label. This is the pass that makes it renderable.
#
# Usage:
#   .\tools\text_to_paths.ps1                  # every panel in res/
#   .\tools\text_to_paths.ps1 -Module Drift
#   .\tools\text_to_paths.ps1 -Check           # report only, convert nothing
#
# Copyright (c) 2026 Daniel Boles. MIT.

[CmdletBinding()]
param(
    [string[]]$Module,
    [switch]$Check,
    [string]$Inkscape = "C:\Program Files\Inkscape\bin\inkscape.exe"
)

$ErrorActionPreference = "Stop"
$res = Join-Path $PSScriptRoot "..\res" | Resolve-Path

$panels = if ($Module) {
    $Module | ForEach-Object { Join-Path $res "$_.svg" }
} else {
    Get-ChildItem $res -Filter *.svg | Select-Object -ExpandProperty FullName
}

if ($Check) {
    $dirty = @()
    foreach ($p in $panels) {
        if (Select-String -Path $p -Pattern "<text" -Quiet) {
            $dirty += (Split-Path $p -Leaf)
        }
    }
    if ($dirty) {
        Write-Host "$($dirty.Count) panel(s) still carry <text>:" -ForegroundColor Yellow
        $dirty | ForEach-Object { Write-Host "    $_" }
        exit 1
    }
    Write-Host "All $($panels.Count) panel(s) are text-free." -ForegroundColor Green
    exit 0
}

if (-not (Test-Path $Inkscape)) {
    throw "Inkscape not found at $Inkscape. Pass -Inkscape <path>."
}

$converted = 0
$skipped = 0

foreach ($p in $panels) {
    $name = Split-Path $p -Leaf

    if (-not (Select-String -Path $p -Pattern "<text" -Quiet)) {
        $skipped++
        continue
    }

    # Inkscape 1.x: select everything, convert to path, save as plain SVG.
    # Writing to a temp file first means a failed run cannot leave a
    # half-converted panel in res/.
    $tmp = [System.IO.Path]::GetTempFileName() + ".svg"
    & $Inkscape --export-type=svg --export-plain-svg `
                --actions="select-all:all;object-to-path" `
                --export-filename="$tmp" "$p" 2>&1 | Out-Null

    if (-not (Test-Path $tmp) -or (Get-Item $tmp).Length -eq 0) {
        Write-Warning "$name — Inkscape produced nothing; left unchanged"
        continue
    }
    if (Select-String -Path $tmp -Pattern "<text" -Quiet) {
        Write-Warning "$name — still has <text> after conversion; left unchanged"
        Remove-Item $tmp -Force
        continue
    }

    Move-Item $tmp $p -Force
    Write-Host "converted  $name"
    $converted++
}

Write-Host ""
Write-Host "$converted converted, $skipped already clean."
