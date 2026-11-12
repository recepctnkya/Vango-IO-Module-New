import json
import time

import serial
import paho.mqtt.client as mqtt

try:
    c = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2)
except Exception:
    c = mqtt.Client()

c.connect("185.33.234.10", 1883, 20)
c.loop_start()
time.sleep(0.5)

ser = serial.Serial("COM14", 115200, timeout=0.2)
ser.reset_input_buffer()

# 1) relay pulse test
relay_cmd = json.dumps(
    {"cmd": "relay_set", "args": {"relay": 0, "state": 1}, "request_id": "fw-test-r1"}
)
print("TX relay_set", relay_cmd)
c.publish("hexnet/v1/2/14/command", relay_cmd, qos=1)

args = {
    "relayEnableMask": 4095,
    "dimEnableMask": 255,
    "sensorEnableMask": 15,
    "tankEnableMask": 7,
    "motorEnabled": True,
    "rgbEnabled": True,
    "channels": {
        "relays": [
            {
                "id": 0,
                "enabled": True,
                "name": "SoT Test Role",
                "icon": "tavan_aydinlatma",
            }
        ]
    },
}
cfg_cmd = json.dumps(
    {"cmd": "io_config_set", "args": args, "request_id": "fw-test-cfg1"}
)
print("TX io_config_set", cfg_cmd[:160])
c.publish("hexnet/v1/2/14/command", cfg_cmd, qos=1)

buf = b""
end = time.time() + 18
while time.time() < end:
    chunk = ser.read(4096)
    if chunk:
        buf += chunk
    time.sleep(0.05)

# turn relay off
c.publish(
    "hexnet/v1/2/14/command",
    json.dumps(
        {"cmd": "relay_set", "args": {"relay": 0, "state": 0}, "request_id": "fw-test-r0"}
    ),
    qos=1,
)
time.sleep(1.0)
chunk = ser.read(8192)
if chunk:
    buf += chunk

ser.close()
c.loop_stop()
c.disconnect()

text = buf.decode("utf-8", "replace")
checks = {
    "MQTT connected": "MQTT connected" in text or "CONNECTED" in text,
    "relay/cmd": ("relay_set" in text) or ("mask=" in text) or ("set_outputs" in text),
    "io_config_set": "io_config_set" in text,
    "labels": ("IO_LABELS" in text) or ("labels" in text),
    "panel changed": "panel changed" in text or "panel_seq" in text,
    "SoT Test": "SoT Test" in text,
}
print("CHECKS", checks)
for line in text.splitlines():
    if any(
        x in line
        for x in (
            "relay",
            "io_config",
            "labels",
            "IO_LABELS",
            "panel",
            "Profil",
            "MQTT",
            "DATA_FROM",
            "command",
        )
    ):
        print(line.strip()[:240])
