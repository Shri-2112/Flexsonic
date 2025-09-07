import serial
import csv
import time
import re

# === CONFIG ===
port = "COM6"
baud = 115200

filename = input("Enter filename (without .csv): ") + ".csv"

print(f"Connecting to {port} at {baud} baud...")
ser = serial.Serial(port, baud)
time.sleep(2)

with open(filename, "w", newline="") as f:
    writer = csv.writer(f)
    writer.writerow(["flex1","flex2","flex3","flex4","flex5","gyroX","gyroY","gyroZ"])

    print(f" Logging started: {filename}")
    print("Press Ctrl+C to stop...\n")

    try:
        while True:
            line = ser.readline().decode(errors="ignore").strip()

            # Extract numbers (handles negative values too)
            nums = re.findall(r"-?\d+", line)

            # Sensor lines have at least 9 numbers: [timestamp, flex1..5, gyroX..Z]
            if len(nums) >= 9:
                values = nums[1:9]  # skip timestamp
                writer.writerow(values)
                print(values)

    except KeyboardInterrupt:
        print("\n Logging stopped. File saved:", filename)
