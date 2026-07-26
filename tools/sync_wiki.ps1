<#
.SYNOPSIS
    Publish docs/wiki/ to the GitHub wiki. One-way: the wiki is a mirror.
.DESCRIPTION
    Clones the wiki repo to a temp directory, replaces its markdown with
    docs/wiki/, commits and pushes. Anything edited directly on github.com is
    overwritten — that is the point.
.EXAMPLE
    .\tools\sync_wiki.ps1 -DryRun
    .\tools\sync_wiki.ps1 -Message "Fourteen modules, new screenshots"
#>
[CmdletBinding()]
param(
    [string]$Message = "Sync wiki from docs/wiki",
    [switch]$DryRun
)

$ErrorActionPreference = 'Stop'

$repo    = Split-Path -Parent $PSScriptRoot
$source  = Join-Path $repo 'docs\wiki'
$wikiUrl = 'https://github.com/dboles99/amplified-futures-vcv.wiki.git'
$work    = Join-Path $env:TEMP "af-wiki-sync-$(Get-Date -Format yyyyMMddHHmmss)"

if (-not (Test-Path $source)) { throw "No wiki source at $source" }

$pages = @(Get-ChildItem "$source\*.md")
if ($pages.Count -eq 0) { throw "No pages in $source - refusing to publish an empty wiki" }

# A stale PAT in the user environment outranks the keyring account.
$env:GH_TOKEN = $null
git clone --quiet $wikiUrl $work
if ($LASTEXITCODE -ne 0) { throw "Clone failed: $wikiUrl" }

Get-ChildItem "$work\*.md" | Remove-Item -Force
Copy-Item "$source\*.md" $work

Push-Location $work
try {
    git add -A
    if (-not (git status --porcelain)) {
        Write-Host 'No changes - wiki already matches docs/wiki.'
        return
    }

    git --no-pager diff --cached --stat

    if ($DryRun) {
        Write-Host "`nDry run - nothing pushed. $($pages.Count) page(s) would publish."
        return
    }

    git -c user.name='Daniel Boles' -c user.email='daniel.boles@gmail.com' commit --quiet -m $Message
    git push --quiet origin master
    if ($LASTEXITCODE -ne 0) { throw 'Push failed' }
    Write-Host "Published $($pages.Count) page(s) to the wiki."
}
finally {
    Pop-Location
    Remove-Item $work -Recurse -Force -ErrorAction SilentlyContinue
}
