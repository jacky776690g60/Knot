param (
    [switch]$Force = $false
)

$BUILD_DIR = "build"
$SOURCE_DIR = Get-Location

try {
    if ($Force) {
        if (Test-Path $BUILD_DIR) {
            Write-Host "Removing $BUILD_DIR/ first ..."
            Remove-Item -Recurse -Force $BUILD_DIR
        }
    }

    if (-not (Test-Path $BUILD_DIR)) {
        Write-Host "Creating $BUILD_DIR directory ..."
        New-Item -ItemType Directory -Force -Path $BUILD_DIR | Out-Null
    }

    Write-Host "Changing to $BUILD_DIR directory ..."
    Push-Location $BUILD_DIR

    Write-Host "Running CMake ..."
    cmake -S $SOURCE_DIR -B .

    if ($LASTEXITCODE -ne 0) {
        throw "CMake configuration failed."
    }

    Write-Host "Building project ..."
    # Assuming you're using Visual Studio's MSBuild
    msbuild ALL_BUILD.vcxproj

    if ($LASTEXITCODE -ne 0) {
        throw "Build failed."
    }

    Write-Host "Build complete."
    Write-Host "Executables can be found in $BUILD_DIR\dist\"
}
catch {
    Write-Host "An error occurred: $_"
    exit 1
}
finally {
    # Always change back to the original directory
    Pop-Location
    Write-Host "Returned to original directory: $(Get-Location)"
}