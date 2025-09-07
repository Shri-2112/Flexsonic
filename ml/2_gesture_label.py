import pandas as pd
import os

# Define paths
base_path = r"E:\Janhavi\SRA\flexsonic\data processed"
input_file = os.path.join(base_path, "data_with_clusters.csv")
output_file = os.path.join(base_path, "gesture_labeled.csv")

# Load clustered data
df = pd.read_csv(input_file)

# Map cluster IDs to gesture labels
label_map = {
    5: "thumb_bent",
    1: "index_bent",
    4: "middle_bent",
    0: "ring_bent",
    3: "pinky_bent",
    2: "all_bent"
}

# Apply mapping
df["gesture"] = df["cluster"].map(label_map)

# Save labeled dataset
df.to_csv(output_file, index=False)

print(f"✅ gesture_labeled.csv regenerated at: {output_file}")
