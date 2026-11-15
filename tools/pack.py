"""Pack a Modtopia release and print its SHA-256."""
from hashlib import sha256
from pathlib import Path
import tomllib
from zipfile import ZIP_DEFLATED, ZipFile


ROOT = Path(__file__).resolve().parent.parent
manifest = ROOT / "mod.toml"
dll = ROOT / "build" / "bin" / "windows-x64" / "mod.dll"
version = tomllib.loads(manifest.read_text(encoding="utf-8"))["version"]
if not dll.is_file():
    raise SystemExit(f"Missing {dll}. Run build.cmd first.")
out = ROOT / "dist" / f"personal_cheats-{version}.pkmod"
out.parent.mkdir(exist_ok=True)
with ZipFile(out, "w", ZIP_DEFLATED) as archive:
    archive.write(manifest, "mod.toml")
    archive.write(dll, "bin/windows-x64/mod.dll")
print(out)
print(f"SHA-256: {sha256(out.read_bytes()).hexdigest()}")
