[CmdletBinding()]
param([string]$Cli='arduino-cli',[string]$ArduinoConfig='')
$ErrorActionPreference='Stop'
function Invoke-Arduino([string[]]$Arguments) {
  if ($ArduinoConfig) { $Arguments += @('--config-file',$ArduinoConfig) }
  & $Cli @Arguments
  if ($LASTEXITCODE -ne 0) { throw 'Arduino CLI setup failed.' }
}
$packageUrl='https://espressif.github.io/arduino-esp32/package_esp32_index.json'
Invoke-Arduino -Arguments @('core','update-index','--additional-urls',$packageUrl)
Invoke-Arduino -Arguments @('core','install','esp32:esp32@3.3.11','--additional-urls',$packageUrl)
Invoke-Arduino -Arguments @('lib','install','OOCSI@1.6.0','ArduinoJson@7.3.2')
Write-Host 'Dependencies installed. Configure config/wifi.json, then run scripts/build.ps1.'
