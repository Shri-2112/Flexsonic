# FlexSonic – Smart Glove for Gesture-to-Speech Communication
A wearable assistive device that translates hand gestures into audible speech using flex sensors, IMU, ESP32, and DFPlayer Mini.

---

## 📑 Table of Contents
- [About the Project](#-about-the-project)  
- [Tech Stack](#️-tech-stack)  
- [File Structure](#-file-structure)  
- [Getting Started](#-getting-started)  
  - [Prerequisites](#prerequisites)  
  - [Installation](#installation)  
- [Usage](#️-usage)  
- [Results and Demo](#-results-and-demo)  
- [Future Work](#-future-work)  
- [Troubleshooting](#-troubleshooting)  
- [Contributors](#-contributors)  
- [Acknowledgements and Resources](#-acknowledgements-and-resources)  
- [License](#-license)  

---

## 📖 About the Project

### Aim  
FlexSonic aims to bridge the communication gap for speech-impaired individuals by converting hand gestures into audible phrases.

### Description  
The glove uses **flex sensors** (to detect finger bending) and an **MPU6050 IMU** (to capture orientation). Sensor data is processed on an **ESP32**, which runs a simple **ML model (K-Means clustering)** to classify gestures. The recognized gesture is mapped to a **pre-recorded audio file** on a **DFPlayer Mini**, producing real-time speech output through a speaker.  

## 🛠️ Tech Stack

**Hardware:**  
- ESP32  
- Flex Sensors  
- MPU6050  
- DFPlayer Mini  
- Speaker  
- SD Card  

**Software:**  
- Arduino IDE / PlatformIO (ESP32 firmware)  
- C / Embedded C for driver code  
- Python (for ML model training & testing)
- ML Libraries: scikit-learn

**Frameworks/Tools:**  
- FreeRTOS (ESP32 task scheduling)  
- I²C, UART communication protocols  

---

## 📂 File Structure

Flexsonic
├── .obsidian/                  # Notes (Obsidian workspace)
│   └── Pre-requisites.md       # Basic notes
│
├── audio/                      # All audio files in use
│
├── build/                      # ESP-IDF build output
│
├── data_collection/            # Raw data logging + visualization
│   ├── data.py                 # Data logging script
│   ├── data.txt                # Raw dataset (text format)
│   ├── graph.py                # Graphical representation of dataset
│   ├── data_processed/         # Processed dataset
│   ├── data_with_clusters.csv  # Clustered dataset
│   ├── gesture_labeled.csv     # Labeled dataset
│   └── gesture_parsed.csv      # Parsed dataset
│
├── main/                       # ESP32 firmware (C code)
│   ├── 1_flex.c
│   ├── 2_mpu.c
│   ├── 3_mpu_and_flex.c
│   ├── 4_sentence_gesture.c
│   └── 5_numbers_gesture.c
│
├── ml/                         # Machine Learning pipeline
│   ├── 1_raw_to_csv.py         # Convert raw logs → CSV
│   ├── 2_preprocess_train_kmeans.py  # Preprocessing + train KMeans
│   ├── 3_gesture_label.py      # Assign labels to gestures
│   ├── 4_predict_and_audio.py  # Live prediction + audio playback
│   └── kmeans_clusters.png     # Visualization of clusters
│
├── models/                     # Saved ML models
│   └── gesture_clusters.pkl
│
├── README.md                   # Project documentation
├── sdkconfig                   # ESP-IDF config file
└── CMakeLists.txt              # ESP-IDF build file
          

## Getting Started

### 📌 Prerequisites
- **ESP-IDF v4.0+** (or Arduino IDE with ESP32 boards installed)  
- **Python 3.8+**

**Required libraries (Arduino/ESP32):**  
- esp32 board support package  
- Adafruit_MPU6050  
- DFRobotDFPlayerMini  

**If using Python ML scripts:**  
```bash
pip install -r requirements.txt
```

## ⚙️ Installation

Clone the repo:
```bash
git clone https://github.com/your_username/flexsonic.git
cd flexsonic
```
## Usage

Follow these steps to run and test the project:

1. **Connect Hardware**
   - Attach all flex sensors to the ESP32.
   - Connect the MPU6050 via I2C.
   - Insert the SD card into the DFPlayer Mini.
   - Connect a small speaker to the DFPlayer Mini output.

2. **Upload Code**
   - Open the project folder in VS Code with PlatformIO or Arduino IDE.
   - Select the ESP32 board and correct COM port.
   - Upload the firmware.

3. **Run the Project**
   - Power on the ESP32 (via USB or battery).
   - Perform different hand gestures.
   - The ESP32 will recognize gestures using sensor data.
   - DFPlayer Mini will play the corresponding pre-recorded audio.

4. **Add New Gestures**
   - Update the gesture mapping logic in `main.cpp`.
   - Add new audio files (`.mp3`) to the SD card in sequential numbering (e.g., `0001.mp3`, `0002.mp3`).
   - Re-flash the code to include new mappings.

5. **Testing**
   - Open the Serial Monitor at 115200 baud rate.
   - Debug sensor readings and check recognized gestures.
```
```
## Results & Demo

### Results
- Successfully translates predefined hand gestures into corresponding audio outputs.  
- Flex sensors + MPU6050 data are processed in real-time by the ESP32.  
- DFPlayer Mini plays pre-recorded audio files with minimal delay.  
- System can be extended by adding more gestures and audio mappings.  
- Achieved smooth serial debugging and stable performance in lab tests.  
```
```
### Demo
You can check out the project demo here:

- **Video Demo**: [Link to Demo Video]((https://drive.google.com/drive/u/1/folders/15n2ZFe-JPjL3UnUXSdUzd99bNddICzaB))  
- **Presentation Slides**: [Link to Slides]((https://www.canva.com/design/DAGyOgvHR_g/AEBH3-gOgE6V5_-GN-xAhQ/edit?ui=eyJEIjp7IlAiOnsiQiI6ZmFsc2V9fX0))  
- **Code Repository**: [GitHub Repo]((https://discord.com/channels/@me/1413976057781944411/1413977835952930918))  

*(Add actual screenshots/images of your setup and Serial Monitor output here)*
```
```
## Future Work

- **Gesture Expansion**: Add more complex gesture combinations for a richer vocabulary.  
- **Dynamic Phrase Generation**: Integrate an ML model for real-time text-to-speech instead of fixed audio files.  
- **Miniaturization**: Design a compact, custom PCB to reduce size and improve portability.  
- **Wireless Connectivity**: Enable Bluetooth/Wi-Fi support for integration with smartphones or IoT devices.  
- **Energy Optimization**: Implement low-power modes for longer battery life.  
- **Multilingual Support**: Expand gesture-to-speech output across multiple languages.  
- **AI Integration**: Use advanced ML techniques (e.g., LSTM/RNN) for continuous gesture recognition.  
- **Cloud/Edge Support**: Store and update gesture mappings via a connected app or cloud server.  

## Troubleshooting

Here are some common issues and their solutions:

- **No audio output from DFPlayer Mini**  
  - Check if the SD card is properly formatted (FAT32).  
  - Ensure audio files are named correctly (e.g., `0001.mp3`, `0002.mp3`).  
  - Verify speaker connections and volume settings.  

- **ESP32 not detecting flex sensors**  
  - Recheck wiring of ADC pins.  
  - Calibrate sensor values in the code.  
  - Test with a multimeter to confirm sensor resistance changes.  

- **IMU (MPU6050) giving unstable readings**  
  - Ensure proper power supply (3.3V, not 5V).  
  - Add a small delay/filtering in code to stabilize readings.  
  - Check I2C connections (SDA, SCL).  

- **Code not compiling**  
  - Verify required libraries are installed (`DFRobotDFPlayerMini`, `Wire`, etc.).  
  - Ensure correct board and COM port are selected in Arduino IDE.  

- **Gestures not recognized correctly**  
  - Increase training data for the ML model.  
  - Adjust threshold values in the code.  
  - Check if flex sensors are securely attached to fingers.  

## Contributors

This project was made possible with the efforts of:

### Mentees
- **Janhavi Mokal**
- **Shrinivas Pathak**

### Mentors
- **Bhavesh Phundhkar**
- **Yash Suthar**
- **Swanand Patil**

Contributions are always welcome!  
Feel free to fork this repo, create a branch, and submit a pull request.  

---

## Acknowledgment

A big thank you to:  
- **SRA VJTI Mumbai** for providing the academic environment to pursue this project.  
- **Open-source communities** whose libraries and resources made implementation easier.  
- Everyone who inspired, tested, and supported FlexSonic in its early stages.  

---

## Resources

Here are some references and tools that helped during development:  

- [ESP32 Documentation](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/)  
- [DFPlayer Mini]((https://drive.google.com/file/d/1GSC8MQxoYqVRTvDOjcIIacYCaxSEa7fd/view))  
- [MPU6050 Guide]((https://invensense.tdk.com/wp-content/uploads/2015/02/MPU-6000-Datasheet1.pdf))
- [Flex Sensors]((https://cdn.sparkfun.com/assets/8/e/7/a/0/flex22.pdf))
- [EspIDF Libraries]((https://docs.espressif.com/projects/esp-idf/en/stable/esp32/get-started/index.html))  
- [K-means Algorithm (ML basics)](https://scikit-learn.org/stable/modules/clustering.html#k-means)  

---

## License

This project is licensed under the **MIT License** – see the [LICENSE](LICENSE) file for details.  
You are free to use, modify, and distribute this project, provided proper attribution is given.  

