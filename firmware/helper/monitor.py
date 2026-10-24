import argparse
import sys
import threading
import time

import serial  # pyserial; PlatformIO's venv has it: ~/.platformio/penv/bin/python

# Log lines start with a level letter: "W   12.345 [CMD] ...".
COLORS = {"D": "\033[2m", "W": "\033[33m", "E": "\033[31m"}
RESET = "\033[0m"


def open_port(port: str, baud: int) -> serial.Serial:
    """Opens without touching DTR/RTS: toggling them resets the ESP32-C3."""
    ser = serial.Serial()
    ser.port = port
    ser.baudrate = baud
    ser.timeout = 0.2
    ser.dtr = False
    ser.rts = False
    ser.open()
    return ser


def show(line: str, color: bool):
    c = COLORS.get(line[:1]) if color and line[1:2] == " " else None
    print(f"{c}{line}{RESET}" if c else line, flush=True)


def read_loop(state: dict, args):
    """Prints every line; reopens the port when the board resets or the
    cable comes back."""
    while True:
        try:
            ser = open_port(args.port, args.baud)
        except serial.SerialException:
            time.sleep(0.5)
            continue
        state["ser"] = ser
        print(f"-- connected to {args.port} --", file=sys.stderr, flush=True)
        try:
            buf = b""
            while True:
                buf += ser.read(ser.in_waiting or 1)
                while b"\n" in buf:
                    raw, buf = buf.split(b"\n", 1)
                    show(raw.rstrip(b"\r").decode(errors="replace"), args.color)
        except (serial.SerialException, OSError):
            state["ser"] = None
            ser.close()
            print(f"-- {args.port} disconnected, waiting --", file=sys.stderr, flush=True)


def main():
    p = argparse.ArgumentParser(description="Serial monitor: prints logs, sends typed lines (e.g. JSON commands).")
    p.add_argument("port", nargs="?", default="/dev/ttyACM0")
    p.add_argument("-b", "--baud", type=int, default=115200, help="ignored by USB serial, matters for UART")
    p.add_argument("--no-color", dest="color", action="store_false")
    args = p.parse_args()
    args.color = args.color and sys.stdout.isatty()

    state = {"ser": None}
    threading.Thread(target=read_loop, args=(state, args), daemon=True).start()

    # Every typed line goes to the device as is.
    try:
        for line in sys.stdin:
            ser = state["ser"]
            if ser is None:
                print("-- not connected --", file=sys.stderr)
                continue
            try:
                ser.write(line.rstrip("\n").encode() + b"\n")
            except serial.SerialException as e:
                print(f"-- write failed: {e} --", file=sys.stderr)
    except KeyboardInterrupt:
        pass


if __name__ == "__main__":
    main()
