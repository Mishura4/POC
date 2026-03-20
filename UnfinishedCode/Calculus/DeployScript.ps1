$ErrorActionPreference = 'Stop'
$WS     = $(Get-Location).Path
$OUT    = $(Get-ChildItem Build\Windows\Release\Release\bin\*exe).FullName
${BIN_DIR} = Split-Path -Path $OUT -Parent

# Deploy directly into the executable's bin directory so the app
# can run in-place without copying out of bin.

& "windeployqt.exe" `
  --release `
  --compiler-runtime `
  --force `
  --dir "$BIN_DIR" `
  --qmldir "$WS\Solution\view" `
  --qmldir "$WS\Solution" `
  --verbose 2 `
  "$OUT"