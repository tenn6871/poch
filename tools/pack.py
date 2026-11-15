"""Pack a Modtopia release and print its SHA-256."""
from hashlib import sha256
from pathlib import Path
import argparse
import tomllib
from zipfile import ZIP_DEFLATED, ZipFile


ROOT = Path(__file__).resolve().parent.parent
parser = argparse.ArgumentParser(description="Pack a platform-specific Modtopia release")
parser.add_argument("--platform", choices=("windows-x64", "linux-x64", "linux-arm64"), required=True)
args = parser.parse_args()
manifest = ROOT / "mod.toml"
library_name = "mod.dll" if args.platform == "windows-x64" else "mod.so"
library = ROOT / "build" / "bin" / args.platform / library_name
version = tomllib.loads(manifest.read_text(encoding="utf-8"))["version"]
if not library.is_file():
    raise SystemExit(f"Missing {library}. Run the platform build script first.")
out = ROOT / "dist" / f"personal_cheats-{version}-{args.platform}.pkmod"
out.parent.mkdir(exist_ok=True)
with ZipFile(out, "w", ZIP_DEFLATED) as archive:
    archive.write(manifest, "mod.toml")
    archive.write(library, f"bin/{args.platform}/{library_name}")
print(out)
print(f"SHA-256: {sha256(out.read_bytes()).hexdigest()}")
