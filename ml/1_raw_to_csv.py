import re
import csv

# Absolute paths
input_file = r"E:\Janhavi\SRA\flexsonic\data collection\data.txt"
output_file = r"E:\Janhavi\SRA\flexsonic\data processed\gesture_parsed.csv"

# Regex patterns
flex_pattern = re.compile(
    r"Thumb:(\d+)\s*\|\s*Index:(\d+)\s*\|\s*Middle:(\d+)\s*\|\s*Ring:(\d+)\s*\|\s*Pinky:(\d+)"
)
gyro_pattern = re.compile(r"Gyro X:(-?\d+)\s*Y:(-?\d+)\s*Z:(-?\d+)")

with open(input_file, "r") as infile, open(output_file, "w", newline="") as csvfile:
    writer = csv.writer(csvfile)
    writer.writerow(["Thumb", "Index", "Middle", "Ring", "Pinky", "Gyro_X", "Gyro_Y", "Gyro_Z"])
    for line in infile:
        flex_match = flex_pattern.search(line)
        gyro_match = gyro_pattern.search(line)
        if flex_match and gyro_match:
            writer.writerow(list(flex_match.groups()) + list(gyro_match.groups()))

print(f"[✔] Parsed data saved to {output_file}")
