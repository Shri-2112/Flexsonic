import serial
import joblib
import numpy as np

# === Load trained model + scaler ===
kmeans = joblib.load("../models/kmeans.pkl")
scaler = joblib.load("../models/scaler.pkl")

# === Serial connection to ESP32 ===
# ⚠️ Change COM port if needed
ser = serial.Serial("COM3", 115200, timeout=1)
print("✅ System ready. Listening for gesture data...")

# === Cluster → gesture → audio mapping ===
cluster_to_label = {
    5: "thumb_bent",
    1: "index_bent",
    4: "middle_bent",
    0: "ring_bent",
    3: "pinky_bent",
    2: "all_bent"
}

cluster_to_audio = {
    5: 1,  # thumb_bent → 0001.mp3
    1: 2,  # index_bent → 0002.mp3
    4: 3,  # middle_bent → 0003.mp3
    0: 4,  # ring_bent → 0004.mp3
    3: 5,  # pinky_bent → 0005.mp3
    2: 6   # all_bent → 0006.mp3
}

while True:
    try:
        # === Read line from ESP32 ===
        line = ser.readline().decode("utf-8").strip()
        if not line:
            continue

        # Expected: "Thumb,Index,Middle,Ring,Pinky,GyroX,GyroY,GyroZ"
        values = [float(x) for x in line.split(",")]
        if len(values) < 5:  # basic safety check
            continue

        # Only use the 5 flex values for clustering
        X_new = np.array(values[:5]).reshape(1, -1)

        # Scale before prediction
        X_scaled = scaler.transform(X_new)

        # Predict cluster
        cluster = int(kmeans.predict(X_scaled)[0])
        gesture = cluster_to_label.get(cluster, "unknown")

        print(f" Detected: Cluster {cluster} = {gesture}")

        # Play mapped audio
        if cluster in cluster_to_audio:
            audio_index = cluster_to_audio[cluster]
            cmd = f"PLAY:{audio_index}\n"  # ESP32 listens for this
            ser.write(cmd.encode("utf-8"))
            print(f" Playing {audio_index} ({gesture})")

    except Exception as e:
        print(" Error:", e)
