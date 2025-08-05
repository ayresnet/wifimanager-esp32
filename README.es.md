# AyresWiFiManager – ESP32 WiFi Manager con portal cautivo

<p align="center">
  <img src="https://res.cloudinary.com/dxunooptp/image/upload/v1754425162/Screenshot_20250805-165306_Acceso_a_portal_cautivo_wzrqwd.png" alt="Portal cautivo en ESP32" height="500">
</p>

**AyresWiFiManager** es una librería para ESP32 que permite configurar redes WiFi y parámetros personalizados a través de un portal web cautivo, ideal para entornos IoT como alarmas, automatización o domótica.

> Compatible con **PlatformIO** y **Arduino IDE**.  
> Código libre, modular y fácil de integrar en cualquier proyecto.


## 📂 Estructura del proyecto

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

---

## 🧩 Características principales

- 🔌 Conexión automática a redes WiFi conocidas
- 🌐 Portal cautivo cuando no hay red guardada
- 💾 Archivos web servidos desde LittleFS
- ⚙️ Soporte para parámetros personalizados (ej. MQTT, tokens, etc.)
- 🧰 Compatible con PlatformIO y Arduino IDE
- 📲 Ideal para sistemas sin pantalla (headless setup)

---

## 🗂️ Estructura del proyecto

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
├── README.md
└── LICENSE
```

---

## 🛠️ Instalación

### 🔷 PlatformIO

1. Agregá esta línea en `platformio.ini`:
   ```ini
   lib_deps = https://github.com/ayresnet/wifimanager-esp32
   ```

2. Subí los archivos web (desde `/data`) a la memoria del ESP32:
   ```bash
   pio run --target uploadfs
   ```

3. Flasheá tu código:
   ```bash
   pio run --target upload
   ```

---

### 🟢 Arduino IDE

1. Descargá este repo como ZIP y agregalo desde:  
   `Sketch → Include Library → Add .ZIP Library`.

2. Instalá el plugin **ESP32 Sketch Data Upload** para subir los archivos del portal web.

3. Colocá los HTML en:
   ```
   /data/wifimanager/
   ```

4. Subilos desde `Tools → ESP32 Sketch Data Upload`

---

## 🧪 Ejemplo básico

```cpp
#include <WifiManager.h>
#include <LittleFS.h>
#include <WiFi.h>

WifiManager wifiManager;

void setup() {
  Serial.begin(115200);
  delay(200);

  // Iniciar LittleFS (requerido para leer wifi.json)
  if (!LittleFS.begin()) {
    Serial.println("❌ Error al montar LittleFS");
    return;
  }

  // Configurar ruta donde están los archivos HTML del portal cautivo
  wifiManager.setHtmlPathPrefix("/wifimanager/");

  // Iniciar WiFiManager (carga configuración desde wifi.json si existe)
  wifiManager.begin();

  // Si no hay archivo wifi.json, lanzar portal cautivo y detener ejecución
  if (!LittleFS.exists("/wifi.json")) {
    Serial.println("⚠️ No se encontró wifi.json → iniciando portal cautivo");
    wifiManager.run();  // Modo AP (se queda esperando que el usuario configure)
    return;
  }

  // Conectar a la red WiFi guardada
  wifiManager.run();

  // Verificar si la conexión fue exitosa
  if (!wifiManager.isConnected()) {
    Serial.println("❌ No se pudo conectar a WiFi");
    return;
  }

  Serial.print("✅ Conectado a: ");
  Serial.println(WiFi.SSID());
}

void loop() {
  wifiManager.update();  // mantiene viva la conexión y escucha eventos
}

```

---

## 🌐 Vista previa del portal

<p align="center">
  <img src="https://res.cloudinary.com/dxunooptp/image/upload/v1754425162/Screenshot_20250805-165306_Acceso_a_portal_cautivo_wzrqwd.png" alt="Portal cautivo en ESP32" height="500">
</p>
---

## 📝 Licencia

Este proyecto está licenciado bajo la **MIT License** – podés usarlo, modificarlo y distribuirlo libremente incluso con fines comerciales.

---

## 🤝 Contribuciones

¡Son bienvenidas!  
Podés abrir un [Issue](https://github.com/ayresnet/wifimanager-esp32/issues) o un Pull Request si querés mejorar algo, agregar ejemplos o reportar errores.

---

📄 Preferís leer esto en **inglés**? Visitá [README_en.md](README_en.md)

---

**Creado por [Daniel Cristian Salgado](https://ayresnet.com) – AyresNet.**
