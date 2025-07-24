param(
    [string]$ExeToSign = "$PWD\_output\bootsmann-0.1-setup-64bit.exe",
    [string]$PfxPassword = "MyStrongPassword123!",
    [string]$PfxPath = "$PWD\MyTestCertSetup.pfx",
    [string]$SignedExe = "$PWD\_output\bootsmann-0.1-setup-64bit-signed.exe",
    [string]$TimestampUrl = "http://timestamp.digicert.com"
)

# Step 1: Create a self-signed code signing certificate
Write-Host "Creating self-signed certificate..."
$cert = New-SelfSignedCertificate -Type CodeSigningCert `
    -Subject "CN=Test Code Signing Cert" `
    -CertStoreLocation "Cert:\CurrentUser\My"

# Step 2: Export the certificate to .pfx
Write-Host "Exporting certificate to PFX..."
$securePwd = ConvertTo-SecureString -String $PfxPassword -Force -AsPlainText
Export-PfxCertificate -Cert $cert -FilePath $PfxPath -Password $securePwd

# Step 3: Sign the executable
if (-Not (Test-Path $ExeToSign)) {
    Write-Error "Executable to sign not found: $ExeToSign"
    exit 1
}

Write-Host "Signing executable..."
$Signtool = "C:\Program Files (x86)\Windows Kits\10\App Certification Kit\signtool.exe"
if (-Not (Test-Path $Signtool)) {
    $Signtool = "${env:ProgramFiles}\Windows Kits\10\bin\x64\signtool.exe"
}
if (-Not (Test-Path $Signtool)) {
    Write-Error "signtool.exe not found. Please install Windows SDK."
    exit 1
}

# Copy the exe to a new file to sign
Copy-Item $ExeToSign $SignedExe -Force

& "$Signtool" sign `
    /f "$PfxPath" `
    /p "$PfxPassword" `
    /fd sha256 `
    /td sha256 `
    /tr "$TimestampUrl" `
    "$SignedExe"

# Step 4: Verify signature
Write-Host "Verifying signature..."
& "$Signtool" verify /pa /v "$SignedExe"
