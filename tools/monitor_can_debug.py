"""Capture COM serial for CAN debug analysis."""
import re
import sys
import time
from pathlib import Path

import serial

PORT = sys.argv[1] if len(sys.argv) > 1 else "COM19"
SECONDS = int(sys.argv[2]) if len(sys.argv) > 2 else 65
OUT = Path(__file__).resolve().parent.parent / "build" / "monitor_com19_capture.txt"

PATTERNS = re.compile(
    r"HEXNET_CAN_RX|CAN_BUS|TWAI|Guru|panic|GECERSIZ|SUPHELI|BILINMEYEN|"
    r"TX_ECHO|ozet 30s|Setting output|relay_set|Platform komut",
    re.I,
)


def main() -> int:
    buf = bytearray()
    try:
        ser = serial.Serial(PORT, 115200, timeout=0.3)
    except serial.SerialException as exc:
        print(f"ERROR: cannot open {PORT}: {exc}")
        return 1

    ser.dtr = False
    ser.rts = True
    time.sleep(0.08)
    ser.rts = False
    time.sleep(0.08)

    end = time.time() + SECONDS
    while time.time() < end:
        chunk = ser.read(4096)
        if chunk:
            buf.extend(chunk)
        else:
            time.sleep(0.05)
    ser.close()

    text = buf.decode("utf-8", errors="replace")
    OUT.parent.mkdir(parents=True, exist_ok=True)
    OUT.write_text(text, encoding="utf-8")

    lines = text.splitlines()
    filtered = [ln for ln in lines if PATTERNS.search(ln)]

    print(f"=== CAPTURE {PORT} {SECONDS}s -> {OUT} ===")
    print(f"total_lines={len(lines)} bytes={len(buf)}")
    print("=== FILTERED ===")
    for ln in filtered[-120:]:
        print(ln)

    print("=== COUNTS ===")
    for key in (
        "HEXNET_CAN_RX",
        "GECERSIZ",
        "SUPHELI",
        "BILINMEYEN",
        "TX_ECHO",
        "ozet 30s",
        "Setting output",
        "Guru Meditation",
        "panic",
    ):
        n = sum(1 for ln in lines if key.lower() in ln.lower())
        print(f"{key}={n}")

    ozet = [ln for ln in lines if "ozet 30s" in ln]
    if ozet:
        print("=== LAST OZET ===")
        print(ozet[-1])
    else:
        print("=== LAST OZET === (yok — CAN task henuz baslamamis olabilir)")

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
