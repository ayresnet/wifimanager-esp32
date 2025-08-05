# AyresWiFiManager – ESP32 WiFi Manager with Captive Portal

<p align="center">
  <img src="https://res.cloudinary.com/dxunooptp/image/upload/v1754425162/Screenshot_20250805-165306_Acceso_a_portal_cautivo_wzrqwd.png" alt="Captive Portal on ESP32" height="500">
</p>

**AyresWiFiManager** is a library for ESP32 that allows configuring WiFi credentials and custom parameters through a local captive portal – ideal for IoT environments like alarms, automation, and smart devices.

> Compatible with **PlatformIO** and **Arduino IDE**.  
> Open source, modular, and easy to integrate into any ESP32 project.

---

## 📂 Project Structure

```plaintext
.
├── lib/
│   └── wifimanager/
│       ├── wifimanager.cpp
│       └── wifimanager.h
├── data/
│   └── wifimanager/
│       ├── index.html
│       ├── success.html
│       └── error.html
├── src/
│   └── main.cpp
├── platformio.ini
└── README_en.md
```

---

## 🧩 Main Features

- 🔌 Auto-connects to known WiFi networks
- 🌐 Local captive portal when no network is configured
- 💾 HTML/CSS/JS served from LittleFS
- ⚙️ Supports custom parameters (e.g., MQTT, tokens, etc.)
- 🧰 Compatible with PlatformIO and Arduino IDE
- 📲 Ideal for headless systems (no screen required)

---

## 🗂️ Library Package Structure

```plaintext
/wifimanager-esp32
├── src/
│   ├── WifiManager.cpp
│   └── WifiManager.h
├── data/
│   └── wifimanager/
│       ├── index.html
│       ├── style.css
│       └── script.js
├── examples/
│   ├── ejemplo_basico/
│   │   └── ejemplo_basico.ino
│   └── ejemplo_platformio/
│       └── ejemplo.cpp
├── library.json
├── library.properties
├── README_en.md
└── LICENSE
```

---

## 🛠️ Installation

### 🔷 PlatformIO

1. Add this to your `platformio.ini`:
   ```ini
   lib_deps = https://github.com/ayresnet/wifimanager-esp32
   ```

2. Upload web files from `/data` to ESP32 flash:
   ```bash
   pio run --target uploadfs
   ```

3. Flash your main code:
   ```bash
   pio run --target upload
   ```

---

### 🟢 Arduino IDE

1. Download this repo as ZIP and add it via:  
   `Sketch → Include Library → Add .ZIP Library`.

2. Install the plugin **ESP32 Sketch Data Upload** to upload HTML files to the device.

3. Place your web portal files in:
   ```
   /data/wifimanager/
   ```

4. Upload them via:  
   `Tools → ESP32 Sketch Data Upload`

---

## 🧪 Basic Example

```cpp
#include <WifiManager.h>

WifiManager wifiManager;

void setup() {
  Serial.begin(115200);
  wifiManager.setHtmlPathPrefix("/wifimanager/");
  wifiManager.autoConnect("AyresIoT-Setup");
  Serial.println("✅ Connected to WiFi!");
}

void loop() {
  // Your main logic here
}
```

---

## 🌐 Captive Portal Preview

<p align="center">
  <img src="https://res.cloudinary.com/dxunooptp/image/upload/v1754425162/Screenshot_20250805-165306_Acceso_a_portal_cautivo_wzrqwd.png" alt="Captive Portal Preview" height="500">
</p>

---

## 📝 License

This project is licensed under the **MIT License** – feel free to use, modify, and distribute it even for commercial use.

---

## 🤝 Contributions

Contributions are welcome!  
Open an [Issue](https://github.com/ayresnet/wifimanager-esp32/issues) or submit a Pull Request to add improvements or fixes.

---

📄 Prefer to read this in **Spanish**? Visit [README.es.md](README.es.md)

---

**Created by [Daniel Cristian Salgado](https://ayresnet.com) – AyresNet.**
