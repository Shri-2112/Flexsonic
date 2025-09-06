# graph.py
import re
import matplotlib.pyplot as plt

# === Read file ===
with open("data.txt", "r") as f:   # adjust path if needed
    lines = f.readlines()

# === Extract data ===
thumb, index, middle, ring, pinky = [], [], [], [], []
gyro_x, gyro_y, gyro_z = [], [], []

pattern = re.compile(
    r"Thumb:(\d+) \| Index:(\d+) \| Middle:(\d+) \| Ring:(\d+) \| Pinky:(\d+).*Gyro X:([-]?\d+) Y:([-]?\d+) Z:([-]?\d+)"
)

for line in lines:
    match = pattern.search(line)
    if match:
        t, i, m, r, p, gx, gy, gz = map(int, match.groups())
        thumb.append(t)
        index.append(i)
        middle.append(m)
        ring.append(r)
        pinky.append(p)
        gyro_x.append(gx)
        gyro_y.append(gy)
        gyro_z.append(gz)

print(f"Parsed {len(thumb)} samples.")

if len(thumb) == 0:
    print("⚠️ No data matched the pattern. Check your data.txt format.")
    exit()

# === Plot ===
plt.figure(figsize=(12, 6))

# Flex sensor values
plt.subplot(2, 1, 1)
plt.plot(thumb, label="Thumb")
plt.plot(index, label="Index")
plt.plot(middle, label="Middle")
plt.plot(ring, label="Ring")
plt.plot(pinky, label="Pinky")
plt.title("Flex Sensor Values")
plt.ylabel("ADC Value")
plt.legend()

# Gyroscope values
plt.subplot(2, 1, 2)
plt.plot(gyro_x, label="Gyro X")
plt.plot(gyro_y, label="Gyro Y")
plt.plot(gyro_z, label="Gyro Z")
plt.title("Gyroscope Data")
plt.ylabel("Angular Velocity")
plt.xlabel("Time (samples)")
plt.legend()

plt.tight_layout()
plt.show()
