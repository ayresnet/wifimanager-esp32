# WiFiManager ESP32

**WiFiManager ESP32** es una librería para gestión automática de WiFi en dispositivos ESP32, diseñada para facilitar la conexión y configuración en entornos IoT. De uso libre y orientado a quienes buscan simplicidad y autonomía en sus proyectos.

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

## 🚀 Características principales

- Escaneo y conexión automática a redes conocidas  
- Portal cautivo con páginas de respuesta personalizadas (`success.html`, `error.html`)  
- Integración sencilla en proyectos con PlatformIO  
- Código modular y fácil de mantener

## 🛠 Instrucciones de uso

1. **Instalar dependencias**: Asegurate de tener PlatformIO y ESP32 configurado.
2. **Agregar la librería**: Copiá `wifimanager.cpp` y `wifimanager.h` dentro de `lib/wifimanager/`.
3. **Interfaz web**: Colocá los archivos `index.html`, `success.html` y `error.html` en `data/wifimanager/`.
4. **Cargar archivos a la SPIFFS**:

   ```bash
   pio run --target uploadfs
   ```

5. **Compilar y flashear el código**:

   ```bash
   pio run --target upload
   ```

6. **Monitoreo en serie**:

   ```bash
   pio device monitor
   ```

## 📄 Licencia

Este proyecto se publica bajo licencia MIT — libertad total para copiar, modificar y distribuir, incluso comercialmente.

## 🤝 Contribuciones

¡Bienvenidas! Si querés sumar mejoras, ejemplos o documentación, podés abrir un *pull request* o dejar un *issue*.

---

**Creado por [@dcsalg](https://github.com/ayresnet)** — impulsado por la necesidad de automatización simple en proyectos ESP32.

---

📄 Preferís leer esto en **inglés**? Visitá [README.md](README.md)
