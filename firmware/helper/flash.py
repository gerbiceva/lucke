"""Build the firmware and flash it, over the air or over USB.

    python3 helper/flash.py                      # OTA to 192.168.0.140
    python3 helper/flash.py 192.168.0.140 192.168.0.141
    python3 helper/flash.py --usb                # serial upload, first port pio finds
    python3 helper/flash.py --build-only

OTA: opens the fixture's update window ({"request":"ota"} on TCP 8888),
uploads with the lolin_c3_mini_ota env, waits for the fixture to come back
with the new version, then waits for "new firmware confirmed" (~30 s after
boot; an unconfirmed image rolls back on the next reboot).
"""
import argparse
import json
import os
import re
import shutil
import socket
import subprocess
import sys
import time
from pathlib import Path

FIRMWARE = Path(__file__).resolve().parent.parent
COMMAND_PORT = 8888
DEFAULT_HOST = "192.168.0.140"


def pio() -> str:
    found = shutil.which("pio")
    if found:
        return found
    fallback = Path.home() / ".platformio/penv/bin/pio"
    if fallback.exists():
        return str(fallback)
    sys.exit("pio not found (not on PATH, not in ~/.platformio/penv/bin)")


def source_version() -> str:
    text = (FIRMWARE / "src/Core/Config/EngineConfig.h").read_text()
    m = re.search(r'ENGINE_VERSION\s*=\s*"([^"]+)"', text)
    return m.group(1) if m else "?"


def request(host: str, req: dict, timeout: float = 5) -> dict:
    """Sends one command, returns the parsed reply."""
    with socket.create_connection((host, COMMAND_PORT), timeout=timeout) as s:
        f = s.makefile("rwb")
        f.write(json.dumps(req).encode() + b"\n")
        f.flush()
        line = f.readline()
    if not line:
        raise ConnectionError("connection closed without a reply")
    return json.loads(line)


def run_pio(env: str, target: str | None, upload_port: str | None = None) -> None:
    cmd = [pio(), "run", "-e", env]
    if target:
        cmd += ["-t", target]
    if upload_port:
        cmd += ["--upload-port", upload_port]
    print(f"$ {' '.join(cmd)}", flush=True)
    if subprocess.run(cmd, cwd=FIRMWARE).returncode != 0:
        sys.exit(f"pio failed ({env}{', ' + target if target else ''})")


def wait_for_reboot(host: str, timeout: float) -> bool:
    """True once the fixture stops answering, i.e. it's restarting into the new image."""
    deadline = time.monotonic() + timeout
    while time.monotonic() < deadline:
        try:
            request(host, {"request": "describe"}, timeout=1)
        except (OSError, ValueError):
            return True
        time.sleep(0.3)
    return False


def wait_for_version(host: str, version: str, timeout: float) -> bool:
    deadline = time.monotonic() + timeout
    while time.monotonic() < deadline:
        try:
            if request(host, {"request": "describe"}, timeout=2).get("version") == version:
                return True
        except (OSError, ValueError):
            pass  # still rebooting
        time.sleep(2)
    return False


def wait_for_confirm(host: str, timeout: float) -> bool:
    deadline = time.monotonic() + timeout
    while time.monotonic() < deadline:
        try:
            if "new firmware confirmed" in json.dumps(request(host, {"request": "logs"})):
                return True
        except (OSError, ValueError):
            pass
        time.sleep(3)
    return False


def flash_ota(host: str, env: str, args) -> bool:
    version = source_version()
    try:
        before = request(host, {"request": "describe"}).get("version", "?")
        reply = request(host, {"request": "ota", "minutes": args.minutes})
    except (OSError, ValueError) as e:
        print(f"[{host}] not reachable: {e}")
        return False
    if reply.get("status") != "ota open":
        print(f"[{host}] couldn't open the OTA window: {reply}")
        return False
    print(f"[{host}] {before} -> {version}, OTA window open")

    run_pio(env, "upload", host)

    # Without this, an unchanged version would "match" before the reboot.
    if not wait_for_reboot(host, timeout=15):
        print(f"[{host}] never saw it go down; checking the version anyway")
    if not wait_for_version(host, version, timeout=90):
        print(f"[{host}] didn't come back as {version} within 90 s")
        return False
    print(f"[{host}] running {version}")

    if args.no_confirm:
        return True
    print(f"[{host}] waiting for the image to be confirmed...", flush=True)
    if not wait_for_confirm(host, timeout=60):
        print(f"[{host}] not confirmed within 60 s; it rolls back on the next reboot")
        return False
    print(f"[{host}] confirmed")
    return True


def main():
    p = argparse.ArgumentParser(description="Build and flash the lucke firmware.")
    p.add_argument("hosts", nargs="*", default=[DEFAULT_HOST], help=f"fixture IPs for OTA (default {DEFAULT_HOST})")
    p.add_argument("--usb", action="store_true", help="upload over serial instead of OTA")
    p.add_argument("--port", help="serial port for --usb (default: pio picks)")
    p.add_argument("--env", help="pio env (default: lolin_c3_mini_ota, or lolin_c3_mini with --usb)")
    p.add_argument("--build-only", action="store_true", help="just build")
    p.add_argument("--minutes", type=int, default=5, help="OTA window length, 1..30")
    p.add_argument("--no-confirm", action="store_true", help="don't wait for the image to be confirmed")
    args = p.parse_args()

    os.environ.setdefault("PYTHONUNBUFFERED", "1")
    print(f"firmware {source_version()}")

    if args.build_only:
        run_pio(args.env or "lolin_c3_mini", None)
        return

    if args.usb:
        run_pio(args.env or "lolin_c3_mini", "upload", args.port)
        return

    env = args.env or "lolin_c3_mini_ota"
    run_pio(env, None)  # build once; a failure stops here before any OTA window opens
    failed = [h for h in args.hosts if not flash_ota(h, env, args)]
    if failed:
        sys.exit(f"failed: {', '.join(failed)}")
    print("all done")


if __name__ == "__main__":
    main()
