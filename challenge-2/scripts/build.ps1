[CmdletBinding()]
param(
  [ValidateSet('all','camera','skidule','morse','laser','vault-direct','vault-bridge','radar-test','uno')]
  [string]$Module = 'all',
  [string]$Cli = 'arduino-cli',
  [string]$ArduinoConfig = '',
  [string]$Libraries = '',
  [switch]$AllowPlaceholderWifi
)
$ErrorActionPreference = 'Stop'
$repoRoot = Split-Path -Parent $PSScriptRoot
$configPath = Join-Path $repoRoot 'config\wifi.json'
$targets = Get-Content -LiteralPath (Join-Path $repoRoot 'targets.json') -Raw | ConvertFrom-Json
$selected = @($targets | Where-Object { ($Module -eq 'all' -and $_.name -ne 'uno') -or $_.name -eq $Module })
$needsWifi = @($selected | Where-Object { $_.wifi }).Count -gt 0
$utf8 = New-Object System.Text.UTF8Encoding($false)

if (!(Test-Path -LiteralPath $configPath)) {
  Copy-Item -LiteralPath (Join-Path $repoRoot 'config\wifi.example.json') -Destination $configPath
  if ($needsWifi -and !$AllowPlaceholderWifi) { throw 'Created config/wifi.json. Fill in SSID/password once, then build again.' }
}
$wifi = Get-Content -LiteralPath $configPath -Raw -Encoding UTF8 | ConvertFrom-Json
foreach ($key in @('ssid','password','oocsi_server','team_channel')) {
  if ($wifi.PSObject.Properties.Name -notcontains $key -or $wifi.$key -isnot [string]) { throw "config/wifi.json: '$key' must be a string." }
  if ($wifi.$key -match '[\x00-\x1f\x7f]') { throw "config/wifi.json: '$key' contains a control character." }
}
if ($utf8.GetByteCount($wifi.ssid) -lt 1 -or $utf8.GetByteCount($wifi.ssid) -gt 32) { throw 'SSID must contain 1-32 UTF-8 bytes.' }
$passwordBytes = $utf8.GetByteCount($wifi.password)
if ($passwordBytes -ne 0 -and ($passwordBytes -lt 8 -or $passwordBytes -gt 64 -or ($passwordBytes -eq 64 -and $wifi.password -notmatch '^[0-9a-fA-F]{64}$'))) { throw 'Password must be empty for open Wi-Fi, 8-63 UTF-8 bytes, or a 64-digit hex key.' }
if ($wifi.team_channel -notmatch '^[A-Za-z0-9_/-]{1,90}$') { throw 'Invalid OOCSI team channel.' }
if ($wifi.oocsi_server -notmatch '^[A-Za-z0-9.-]{1,253}$') { throw 'OOCSI server must be a hostname, without a protocol or path.' }
$configured = $wifi.ssid -ne 'YOUR_WIFI_NAME' -and $wifi.password -ne 'YOUR_WIFI_PASSWORD'
if ($needsWifi -and !$configured -and !$AllowPlaceholderWifi) { throw 'Set real Wi-Fi details in config/wifi.json. Use -AllowPlaceholderWifi only to compile a non-connecting test build.' }

# Generate one library on EVERY invocation. Every ESP includes this exact header.
# Hex-encode each UTF-8 byte so quotes, backslashes and non-ASCII names are safe.
function ConvertTo-CppString([string]$Value) {
  return '"' + (($utf8.GetBytes($Value) | ForEach-Object { '\x{0:X2}' -f $_ }) -join '') + '"'
}
$libraryRoot = Join-Path $repoRoot 'build\generated\Team2Config'
$librarySrc = Join-Path $libraryRoot 'src'
New-Item -ItemType Directory -Path $librarySrc -Force | Out-Null
$header = "#pragma once`nnamespace team2 {`n"
foreach ($entry in @(@('WIFI_SSID','ssid'),@('WIFI_PASSWORD','password'),@('OOCSI_SERVER','oocsi_server'),@('TEAM_CHANNEL','team_channel'))) {
  $header += 'constexpr char ' + $entry[0] + '[] = ' + (ConvertTo-CppString $wifi.($entry[1])) + ";`n"
}
$header += 'constexpr bool WIFI_CONFIGURED = ' + $configured.ToString().ToLowerInvariant() + ";`n}`n"
[IO.File]::WriteAllText((Join-Path $librarySrc 'Team2Config.h'), $header, $utf8)
[IO.File]::WriteAllText((Join-Path $libraryRoot 'library.properties'), "name=Team2Config`nversion=1.0.0`nauthor=Team 2`nmaintainer=Team 2`nsentence=Generated local heist configuration.`nparagraph=Generated from config/wifi.json; never commit the generated header.`ncategory=Communication`narchitectures=esp32`n", $utf8)

$results = @()
foreach ($target in $selected) {
  $sketchPath = Join-Path $repoRoot ('firmware\' + $target.sketch)
  $buildPath = Join-Path $repoRoot ('build\' + $target.name)
  $cliArgs = @('compile','--fqbn',$target.fqbn,'--build-path',$buildPath)
  if ($target.name -ne 'uno') { $cliArgs += @('--library',$libraryRoot) }
  if ($ArduinoConfig) { $cliArgs += @('--config-file',$ArduinoConfig) }
  if ($Libraries) { $cliArgs += @('--libraries',$Libraries) }
  $cliArgs += $sketchPath
  Write-Host ("Building {0} ({1})" -f $target.name,$target.fqbn)
  & $Cli @cliArgs
  if ($LASTEXITCODE -ne 0) { throw "Build failed: $($target.name). No upload was attempted." }
  $results += [ordered]@{module=$target.name;fqbn=$target.fqbn;build_path=$buildPath;compiled=$true}
}
$summary = [ordered]@{built_at=(Get-Date).ToUniversalTime().ToString('o');wifi_configured=$configured;modules=$results}
$summary | ConvertTo-Json -Depth 5 | Set-Content -LiteralPath (Join-Path $repoRoot 'build\last-build.json') -Encoding UTF8
Write-Host 'Build complete. No hardware was uploaded or moved. Build outputs contain Wi-Fi credentials; keep build/ private.'
