import argparse, os
import pandas as pd
import numpy as np
from sklearn.preprocessing import StandardScaler
from sklearn.cluster import KMeans
from sklearn.decomposition import PCA
import matplotlib.pyplot as plt
import joblib

parser = argparse.ArgumentParser()
parser.add_argument("infile")
parser.add_argument("--k", type=int, default=6, help="Number of clusters (gestures)")
parser.add_argument("--use_gyro", action="store_true", help="Include GyroX,Y,Z features")
args = parser.parse_args()

# === Load data ===
df = pd.read_csv(args.infile)

# Flex columns
flex_cols = ["Thumb", "Index", "Middle", "Ring", "Pinky"]
for c in flex_cols:
    if c not in df.columns:
        df[c] = 0

# Gyro columns (optional)
gyro_cols = ["GyroX", "GyroY", "GyroZ"]
for c in gyro_cols:
    if c not in df.columns:
        df[c] = 0

# Select features
features = flex_cols + gyro_cols if args.use_gyro else flex_cols

# Clean: drop rows where all flex = 0
df = df[df[flex_cols].sum(axis=1) > 0].reset_index(drop=True)
print(f"✅ Rows after cleaning: {len(df)}")
print(f"Training KMeans with k={args.k}, features={features}")

# === KMeans ===
X = df[features].astype(float).values
scaler = StandardScaler()
X_scaled = scaler.fit_transform(X)

kmeans = KMeans(n_clusters=args.k, random_state=42, n_init=10)
kmeans.fit(X_scaled)

df["cluster"] = kmeans.labels_

# Save models
models_dir = os.path.join("..", "models")
os.makedirs(models_dir, exist_ok=True)
joblib.dump(kmeans, os.path.join(models_dir, "kmeans.pkl"))
joblib.dump(scaler, os.path.join(models_dir, "scaler.pkl"))
print("💾 Saved KMeans + scaler to ../models/")

# Save labelled CSV to the correct "data processed" folder
out_csv = os.path.join("..", "data processed", "data_with_clusters.csv")
os.makedirs(os.path.dirname(out_csv), exist_ok=True)
df.to_csv(out_csv, index=False)
print("📁 Saved labelled data to:", out_csv)

# === Visualization ===
pca = PCA(n_components=2)
reduced = pca.fit_transform(X_scaled)
centers_pca = pca.transform(kmeans.cluster_centers_)

plt.figure(figsize=(8, 6))
plt.scatter(reduced[:, 0], reduced[:, 1], c=df["cluster"], cmap="tab10", s=30)
plt.scatter(centers_pca[:, 0], centers_pca[:, 1], c="black", marker="X", s=150, label="centroids")
plt.title(f"KMeans (k={args.k}) on gesture data (PCA projection)")
plt.xlabel("PC1")
plt.ylabel("PC2")
plt.legend()
plt.grid(True)
plot_path = os.path.join("..", "ml", "kmeans_clusters.png")
os.makedirs(os.path.dirname(plot_path), exist_ok=True)
plt.savefig(plot_path, dpi=150)
print("📊 Saved PCA cluster plot to:", plot_path)
plt.show()
