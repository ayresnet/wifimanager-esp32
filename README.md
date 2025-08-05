# WiFiManager ESP32

**WiFiManager ESP32** is a library for automatic WiFi management on ESP32 devices, designed to simplify connectivity and configuration in IoT environments. Open-source and focused on providing simplicity and autonomy for your projects.

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
└── README.md
```

## 🚀 Main Features

- Scans and connects automatically to known networks  
- Captive portal with custom response pages (`success.html`, `error.html`)  
- Easy integration with PlatformIO-based projects  
- Modular and maintainable code

## 🛠 How to Use

1. **Install dependencies**: Make sure you have PlatformIO and ESP32 set up.
2. **Add the library**: Copy `wifimanager.cpp` and `wifimanager.h` into `lib/wifimanager/`.
3. **Web interface**: Place the `index.html`, `success.html`, and `error.html` files inside `data/wifimanager/`.
4. **Upload files to SPIFFS**:

   ```bash
   pio run --target uploadfs
   ```

5. **Compile and flash the code**:

   ```bash
   pio run --target upload
   ```

6. **Serial monitor**:

   ```bash
   pio device monitor
   ```

## 📄 License

This project is released under the MIT License — full freedom to copy, modify, and distribute, even commercially.

## 🤝 Contributions

Contributions are welcome! If you'd like to add improvements, examples, or documentation, feel free to open a pull request or an issue.

---

**Created by [@dcsalg](https://github.com/ayresnet)** — driven by the need for simple automation in ESP32 projects.

---

📄 Prefer to read this in **Spanish**? Check out [README.es.md](README.es.md)


