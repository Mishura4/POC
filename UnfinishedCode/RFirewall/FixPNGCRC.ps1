# Script to fix PNG iCCP CRC errors using ImageMagick
# This strips the corrupted ICC color profile chunks from PNG files

param(
    [string]$ResourcePath = ".\Solution\resource",
    [switch]$Backup = $true
)

# Check if magick is available
try {
    $magickVersion = & magick -version 2>$null
    if (-not $magickVersion) {
        Write-Host "ImageMagick not found. Install it from: https://imagemagick.org/script/download.php#windows" -ForegroundColor Red
        Write-Host "Or use: choco install imagemagick" -ForegroundColor Yellow
        exit 1
    }
} catch {
    Write-Host "ImageMagick not found in PATH" -ForegroundColor Red
    exit 1
}

Write-Host "ImageMagick found: $($magickVersion.Split([Environment]::NewLine)[0])" -ForegroundColor Green

# Find all PNG files
$pngFiles = Get-ChildItem -Path $ResourcePath -Filter "*.png" -Recurse
Write-Host "Found $($pngFiles.Count) PNG files to process" -ForegroundColor Cyan

$successCount = 0
$errorCount = 0

foreach ($file in $pngFiles) {
    $filePath = $file.FullName
    Write-Host "Processing: $($file.Name)..." -NoNewline
    
    if ($Backup) {
        $backupPath = "$filePath.bak"
        Copy-Item -Path $filePath -Destination $backupPath -Force
    }
    
    try {
        # Remove the iCCP chunk by converting and re-saving
        # This strips all profiles and re-encodes without them
        & magick $filePath -strip $filePath 2>$null
        Write-Host " [OK]" -ForegroundColor Green
        $successCount++
    } catch {
        Write-Host " [FAILED]" -ForegroundColor Red
        Write-Host "  Error: $_"
        $errorCount++
    }
}

Write-Host "`nSummary:" -ForegroundColor Cyan
Write-Host "  Processed: $($pngFiles.Count)" -ForegroundColor White
Write-Host "  Success: $successCount" -ForegroundColor Green
Write-Host "  Failed: $errorCount" -ForegroundColor $(if ($errorCount -eq 0) { "Green" } else { "Red" })

if ($Backup) {
    Write-Host "`nBackup files created with .bak extension" -ForegroundColor Yellow
}
