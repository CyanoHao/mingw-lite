#Requires -Version 7.3

param(
    [string]$Distro = 'mlenv',
    [string]$Root = './env',
    [int]$Version = 2
)

$ErrorActionPreference = 'Stop'
$PSNativeCommandArgumentPassing = 'Standard'
$PSNativeCommandUseErrorActionPreference = $true

$OsArch = [System.Runtime.InteropServices.RuntimeInformation]::OSArchitecture
$WslImage = switch ($OsArch) {
  'X64' {
    @{
      Name = 'ubuntu-24.04.4-wsl-amd64.wsl'
      UrlBase = 'https://releases.ubuntu.com/24.04'
    }
  }
  'Arm64' {
    @{
      Name = 'ubuntu-24.04.4-wsl-arm64.wsl'
      UrlBase = 'https://cdimages.ubuntu.com/ubuntu/releases/24.04/release'
    }
  }
  default {
    throw "Unsupported OS architecture: $OsArch"
  }
}
$WslImageFile = "assets/$($WslImage.Name)"
$WslImageUrl = "$($WslImage.UrlBase)/$($WslImage.Name)"

if (Test-Path $Root) {
  if ((Get-ChildItem -LiteralPath $Root).Count -gt 0) {
    throw "Root directory '$Root' is not empty"
  }
} else {
  New-Item -ItemType Directory -Path $Root -Force
}

if (-not (Test-Path $WslImageFile)) {
  New-Item -ItemType Directory -Path 'assets' -Force
  Invoke-WebRequest -Uri $WslImageUrl -OutFile $WslImageFile
}

wsl --import $Distro $Root $WslImageFile --version $Version

wsl --distribution $Distro --exec apt update
# don't update -- systemd will fail on WSL 1
wsl --distribution $Distro --exec env DEBIAN_FRONTEND=noninteractive `
  apt-get install --no-install-recommends -y `
    build-essential cmake flex gettext libtool llvm-19 m4 ninja-build texinfo `
    7zip ca-certificates curl file gawk libarchive-tools python3 python3-packaging zstd
foreach ($t in 'llvm-config', 'llvm-ar', 'llvm-ranlib', 'llvm-dlltool') {
  wsl --distribution $Distro --exec update-alternatives --install "/usr/bin/$t" $t "/usr/bin/$t-19" 100
  wsl --distribution $Distro --exec update-alternatives --set $t "/usr/bin/$t-19"
}
wsl --distribution $Distro --exec sync
