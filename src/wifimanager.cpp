/**
 * @file    WifiManager.cpp
 * @brief   Clase profesional para gestión de WiFi en ESP32 con almacenamiento en LittleFS, portal cautivo y sincronización NTP.
 * 
 * @version 1.1.0
 * @author  Daniel Salgado
 * @date    2025-07-22
 *
 * @details
 * Esta clase ofrece:
 * - Conexión WiFi desde credenciales almacenadas
 * - Almacenamiento seguro en LittleFS
 * - Portal cautivo y servidor web para configuración
 * - Sincronización de hora mediante NTP
 * - Verificación de conexión a Internet
 * - Lógica de reintento automático
 * - Control por botón físico para reset de configuración
 * - Timestamp en milisegundos desde epoch
 */

#include "WifiManager.h"
#include <ArduinoJson.h>
#include <time.h>
#include <cstdint>
#include <HTTPClient.h>

#define DEFAULT_LED_PIN 2
#define DEFAULT_BUTTON_PIN 0
#define DEFAULT_AP_SSID "WiFi Manager"
#define DEFAULT_AP_PASS "123456789"
#define DNS_PORT 53

static bool autoReconnect = true; 


// Constructor con pines configurables para LED y botón
WifiManager::WifiManager(uint8_t ledPin, uint8_t buttonPin)
: server(80), ledPin(ledPin), buttonPin(buttonPin), connected(false), ultimoIntentoWiFi(0) {}

// Inicializa pines, monta el sistema de archivos y carga credenciales si existen
void WifiManager::begin() {
    pinMode(ledPin, OUTPUT);
    digitalWrite(ledPin, LOW);

    pinMode(buttonPin, INPUT_PULLUP);

    if (!LittleFS.begin(true)) {
        Serial.println("Error montando LittleFS");
        return;
    }

    loadCredentials();
}

// Ejecuta la lógica principal: chequea botón, intenta conexión o lanza portal cautivo
void WifiManager::run() {
    unsigned long startTime = millis();
    bool botonPresionado = false;

    // Indicación visual para permitir al usuario resetear configuración
    Serial.println("🔔 Mantené presionado el botón para borrar WiFi (parpadeo LED).");

    while (millis() - startTime < 2000) {
        digitalWrite(ledPin, HIGH);
        delay(100);
        digitalWrite(ledPin, LOW);
        delay(100);

        if (digitalRead(buttonPin) == LOW) {
            botonPresionado = true;
            break;
        }
    }

    // Si el botón se mantiene presionado por 5 segundos, se eliminan las credenciales
    if (botonPresionado) {
        Serial.println("⏳ Manteniendo presionado...");

        unsigned long confirmStart = millis();
        while (digitalRead(buttonPin) == LOW) {
            if (millis() - confirmStart >= 5000) {
                Serial.println("🩹 Botón presionado por 5 segundos. Borrando credenciales WiFi.");
                eraseCredentials();
                ESP.restart();
                return;
            }
            delay(100);
        }

        Serial.println("❌ Botón soltado antes de tiempo. No se borraron las credenciales.");
    }

    // Si hay credenciales, intenta conectar a WiFi
    if (connectToWiFi()) {
        Serial.println("✅ Conexión WiFi exitosa.");
        sincronizarHoraNTP();
        digitalWrite(ledPin, HIGH);
        connected = true;
        return;
    }

    digitalWrite(ledPin, LOW);
    if (!tieneCredenciales()) {
        Serial.println("🟡 No hay credenciales guardadas. Iniciando configuración WiFi...");
        setupAP();

        server.on("/", std::bind(&WifiManager::handleRoot, this));
        server.on("/save", std::bind(&WifiManager::handleSave, this));
        server.on("/scan", std::bind(&WifiManager::handleScan, this));
        server.onNotFound(std::bind(&WifiManager::handleNotFound, this));
        server.begin();

        Serial.println("🌐 Servidor web iniciado en 192.168.4.1");
    } else {
        Serial.println("🔴 Falló la conexión con la red WiFi configurada. No se abrirá el portal AP.");
    }
}

// Devuelve true si existe el archivo y las credenciales no están vacías
bool WifiManager::tieneCredenciales() const {
    return LittleFS.exists("/wifi.json") && !ssid.isEmpty() && !password.isEmpty();
}

// Carga las credenciales desde el archivo JSON en LittleFS
void WifiManager::loadCredentials() {
    if (!LittleFS.exists("/wifi.json")) {
        Serial.println("Archivo de credenciales no existe.");
        return;
    }

    File file = LittleFS.open("/wifi.json", "r");
    if (!file) {
        Serial.println("No se pudo abrir el archivo de credenciales.");
        return;
    }

    StaticJsonDocument<192> doc;
    DeserializationError error = deserializeJson(doc, file);
    if (error) {
        Serial.println("Error al deserializar JSON.");
        return;
    }

    String loadedSsid = doc["ssid"].as<String>();
    String loadedPassword = doc["password"].as<String>();

    if (loadedSsid.isEmpty() || loadedPassword.isEmpty()) {
        Serial.println("Credenciales vacías en el archivo. Ignorando.");
        return;
    }

    ssid = loadedSsid;
    password = loadedPassword;
    Serial.println("Credenciales cargadas correctamente.");
}

// Elimina el archivo de credenciales guardadas
void WifiManager::eraseCredentials() {
    LittleFS.remove("/wifi.json");
    LittleFS.remove("/setup.json");
    LittleFS.remove("/iporton.json");
    //LittleFS.remove("/wifi.json");
    Serial.println("Credenciales eliminadas.");
}



// Intenta conectar al WiFi utilizando las credenciales almacenadas
bool WifiManager::connectToWiFi() {
    if (!tieneCredenciales()) return false;

    WiFi.mode(WIFI_AP_STA);
    WiFi.begin(ssid.c_str(), password.c_str());

    Serial.print("Conectando a ");
    Serial.println(ssid);

    for (int i = 0; i < 30; i++) {
        if (WiFi.status() == WL_CONNECTED) {
            Serial.println("Conectado a WiFi.");
            WiFi.setSleep(false);
            return true;
        }
        delay(1000);
    }

    Serial.println("Tiempo agotado. No se pudo conectar.");
    return false;
}

// Configura el ESP32 como Access Point
void WifiManager::setupAP() {
    WiFi.mode(WIFI_AP);
    WiFi.softAP(DEFAULT_AP_SSID, DEFAULT_AP_PASS);
    Serial.println("Access Point creado: " DEFAULT_AP_SSID);
}

// Manejador para servir el archivo index.html desde LittleFS
void WifiManager::handleRoot() {
    String path = htmlPathPrefix + "index.html";
    if (!LittleFS.exists(path)) {
        server.send(500, "text/html", "<h1>Error: index.html no encontrado</h1>");
        return;
    }
    File file = LittleFS.open(path, "r");
    if (!file || file.isDirectory()) {
        server.send(500, "text/html", "<h1>Error abriendo index.html</h1>");
        return;
    }
    server.send(200, "text/html", file.readString());
    file.close();
}

// Manejador para guardar credenciales enviadas desde el formulario web
void WifiManager::handleSave() {
    if (server.method() != HTTP_POST) {
        server.send(405, "text/plain", "Método no permitido");
        return;
    }

    String ssid = server.arg("ssid");
    String password = server.arg("password");

    if (ssid.isEmpty() || password.isEmpty()) {
        mostrarPaginaError("Faltan datos para guardar.");
        return;
    }

    StaticJsonDocument<192> doc;
    doc["ssid"] = ssid;
    doc["password"] = password;

    File file = LittleFS.open("/wifi.json", "w");
    if (!file) {
        mostrarPaginaError("Error al guardar credenciales.");
        return;
    }

    serializeJson(doc, file);
    file.close();

    File success = LittleFS.open(htmlPathPrefix + "success.html", "r");
    if (!success) {
        server.send(200, "text/html", "<h1>Guardado. Reiniciando...</h1>");
    } else {
        server.send(200, "text/html", success.readString());
        success.close();
    }

    delay(1000);
    ESP.restart();
}

// Muestra una página de error o un mensaje HTML básico si el archivo no existe
void WifiManager::mostrarPaginaError(const String& mensajeFallback) {
    File errorFile = LittleFS.open(htmlPathPrefix + "error.html", "r");
    if (!errorFile) {
        server.send(500, "text/html", "<h1>Error: " + mensajeFallback + "</h1>");
    } else {
        server.send(500, "text/html", errorFile.readString());
        errorFile.close();
    }
}

// Escanea redes WiFi disponibles y devuelve un JSON con SSID, RSSI y seguridad
void WifiManager::handleScan() {
    Serial.println("🔍 Escaneando redes WiFi...");

    WiFi.mode(WIFI_AP_STA);  // Mantenemos el AP activo
    delay(200);

    WiFi.scanDelete();
    int n = WiFi.scanNetworks();
    Serial.printf("📱 %d redes encontradas\n", n);

    DynamicJsonDocument doc(1024);
    JsonArray arr = doc.to<JsonArray>();

    for (int i = 0; i < n; ++i) {
        JsonObject obj = arr.createNestedObject();
        obj["ssid"] = WiFi.SSID(i);
        obj["rssi"] = WiFi.RSSI(i);
        obj["secure"] = (WiFi.encryptionType(i) != WIFI_AUTH_OPEN);
    }

    WiFi.scanDelete();

    String output;
    serializeJson(doc, output);
    server.send(200, "application/json", output);
}



// Redirecciona al inicio si se accede a una ruta no válida
void WifiManager::handleNotFound() {
    server.sendHeader("Location", "/", true);
    server.send(302, "text/plain", "");
}

// Sincroniza la hora local con servidores NTP
void WifiManager::sincronizarHoraNTP() {
    configTime(0, 0, "pool.ntp.org", "time.nist.gov");
    for (int j = 0; j < 20; j++) {
        time_t now = time(nullptr);
        if (now > 100000) {
            Serial.print("Hora sincronizada: ");
            Serial.println(ctime(&now));
            return;
        }
        delay(200);
    }
    Serial.println("⚠️ NTP no respondió. Continuando sin sincronizar.");
}

// Devuelve timestamp actual en milisegundos si la hora fue sincronizada
uint64_t WifiManager::getTimestamp() {
    time_t now = time(nullptr);
    return (now > 100000) ? static_cast<uint64_t>(now) * 1000ULL : 0;
}

// Verifica si el dispositivo está conectado al WiFi
bool WifiManager::isConnected() {
    return connected && WiFi.status() == WL_CONNECTED;
}

// Devuelve el nivel de señal RSSI de la red actual
int WifiManager::getSignalStrength() {
    return WiFi.RSSI();
}

// Maneja las peticiones entrantes del cliente HTTP
void WifiManager::update() {
    server.handleClient();
}

// Define el prefijo de ruta para buscar archivos HTML
void WifiManager::setHtmlPathPrefix(const String& prefix) {
    htmlPathPrefix = prefix.endsWith("/") ? prefix : prefix + "/";
}

// Reintenta conectar a WiFi si está desconectado, cada 10 segundos
// void WifiManager::reintentarConexionSiNecesario() {
//     if (connected) return;
//     unsigned long ahora = millis();
//     if (ahora - ultimoIntentoWiFi < 10000) return;
//     ultimoIntentoWiFi = ahora;
//     if (!ssid.isEmpty() && !password.isEmpty()) {
//         Serial.println("🔁 Intentando reconexión WiFi...");
//         WiFi.mode(WIFI_AP_STA);
//         WiFi.begin(ssid.c_str(), password.c_str());
//         for (int i = 0; i < 10; i++) {
//             if (WiFi.status() == WL_CONNECTED) {
//                 Serial.println("🔌 Reconectado a WiFi.");
//                 sincronizarHoraNTP();
//                 connected = true;
//                 return;
//             }
//             delay(500);
//         }
//         Serial.println("❌ Reconexión WiFi fallida.");
//     }
// }

void WifiManager::reintentarConexionSiNecesario() {
    if (!autoReconnect) return;  // ← si se deshabilitó, no reconecta
    if (connected) return;
    unsigned long ahora = millis();
    if (ahora - ultimoIntentoWiFi < 10000) return;
    ultimoIntentoWiFi = ahora;

    if (!ssid.isEmpty() && !password.isEmpty()) {
        Serial.println("🔁 Intentando reconexión WiFi...");
        WiFi.mode(WIFI_AP_STA);
        WiFi.begin(ssid.c_str(), password.c_str());
        for (int i = 0; i < 10; i++) {
            if (WiFi.status() == WL_CONNECTED) {
                Serial.println("🔌 Reconectado a WiFi.");
                sincronizarHoraNTP();
                connected = true;
                return;
            }
            delay(500);
        }
        Serial.println("❌ Reconexión WiFi fallida.");
    }
}

// Verifica si hay conexión real a Internet usando un endpoint de Google
bool WifiManager::hayInternet() {
    if (WiFi.status() != WL_CONNECTED) return false;
    WiFiClient client;
    HTTPClient http;
    http.begin(client, "http://clients3.google.com/generate_204");
    http.setConnectTimeout(3000);
    int httpCode = http.GET();
    http.end();
    return (httpCode == 204);
}

// Nuevo método para habilitar/deshabilitar reconexión automática
void WifiManager::setAutoReconnect(bool habilitado) {
    autoReconnect = habilitado;
}

/* ==============================================================
   Detección de que la red preferida volvió a aparecer (modo AP)
   ============================================================== */
bool WifiManager::scanRedDetectada() {
    unsigned long ahora = millis();
    if (ahora - ultimoScan < SCAN_INTERVAL_MS) return false;   // evita spam
    ultimoScan = ahora;

    int n = WiFi.scanNetworks(/*async=*/false, /*show_hidden=*/false);
    bool encontrada = false;
    for (int i = 0; i < n; ++i) {
        if (WiFi.SSID(i) == ssid) { encontrada = true; break; }
    }
    WiFi.scanDelete();              // libera RAM
    return encontrada;
}

/* ==============================================================
   Fuerza reconexión STA manteniendo (por ahora) el AP
   ============================================================== */
void WifiManager::forzarReconexion() {
    Serial.println("🔄  Forzando reconexión STA…");
    WiFi.mode(WIFI_AP_STA);                     // mantiene portal activo
    WiFi.begin(ssid.c_str(), password.c_str());
    ultimoIntentoWiFi = millis();
}


////////////////////////////////////////
// #include "WifiManager.h"
// #include <ArduinoJson.h>
// #include <time.h>
// #include <cstdint>
// #include <HTTPClient.h>

// #define LED_PIN 2
// #define BUTTON_PIN 0
// #define DNS_PORT 53

// WifiManager::WifiManager()
// : server(80)
// {}

// void WifiManager::begin() {
//     pinMode(LED_PIN, OUTPUT);
//     digitalWrite(LED_PIN, LOW);

//     pinMode(BUTTON_PIN, INPUT_PULLUP);

//     if (!LittleFS.begin(true)) {
//         Serial.println("Error montando LittleFS");
//         return;
//     }

//     loadCredentials();
// }

// void WifiManager::run() {
//     unsigned long startTime = millis();
//     bool botonPresionado = false;

//     Serial.println("🔔 Mantené presionado el botón para borrar WiFi (parpadeo LED).");

//     while (millis() - startTime < 2000) {
//         digitalWrite(LED_PIN, HIGH);
//         delay(100);
//         digitalWrite(LED_PIN, LOW);
//         delay(100);

//         if (digitalRead(BUTTON_PIN) == LOW) {
//             botonPresionado = true;
//             break;
//         }
//     }

//     if (botonPresionado) {
//         Serial.println("⏳ Manteniendo presionado...");

//         unsigned long confirmStart = millis();
//         while (digitalRead(BUTTON_PIN) == LOW) {
//             if (millis() - confirmStart >= 5000) {
//                 Serial.println("🩹 Botón presionado por 5 segundos. Borrando credenciales WiFi.");
//                 eraseCredentials();
//                 ESP.restart();
//                 return;
//             }
//             delay(100);
//         }

//         Serial.println("❌ Botón soltado antes de tiempo. No se borraron las credenciales.");
//     }

//     if (connectToWiFi()) {
//         Serial.println("✅ Conexión WiFi exitosa.");
//         sincronizarHoraNTP();
//         digitalWrite(LED_PIN, HIGH);
//         connected = true;
//         return;
//     }

//     digitalWrite(LED_PIN, LOW);
//     if (ssid.isEmpty() || password.isEmpty()) {
//         Serial.println("🟡 No hay credenciales guardadas. Iniciando configuración WiFi...");
//         setupAP();

//         // Montar el servidor web manualmente
//         server.on("/", std::bind(&WifiManager::handleRoot, this));
//         server.on("/save", std::bind(&WifiManager::handleSave, this));
//         server.on("/scan", std::bind(&WifiManager::handleScan, this));
//         server.onNotFound(std::bind(&WifiManager::handleNotFound, this));
//         server.begin();

//         Serial.println("🌐 Servidor web iniciado en 192.168.4.1");
//     } else {
//         Serial.println("🔴 Falló la conexión con la red WiFi configurada. No se abrirá el portal AP.");
//         return;
//     }


// }

// void WifiManager::setHtmlPathPrefix(const String& prefix) {
//     if (!prefix.endsWith("/")) {
//         htmlPathPrefix = prefix + "/";
//     } else {
//         htmlPathPrefix = prefix;
//     }
// }

// void WifiManager::update() {
//     //dnsServer.processNextRequest();
//     server.handleClient();
// }

// bool WifiManager::isConnected() {
//     return connected && WiFi.status() == WL_CONNECTED;
// }

// int WifiManager::getSignalStrength() {
//     return WiFi.RSSI();
// }

// uint64_t WifiManager::getTimestamp() {
//     time_t now = time(nullptr);
//     if (now > 100000) {
//         return static_cast<uint64_t>(now) * 1000ULL;
//     } else {
//         return 0;
//     }
// }

// void WifiManager::eraseCredentials() {
//     LittleFS.remove("/wifi.json");
//     Serial.println("Credenciales eliminadas.");
// }

// void WifiManager::loadCredentials() {
//     if (!LittleFS.exists("/wifi.json")) return;

//     File file = LittleFS.open("/wifi.json", "r");
//     if (!file) {
//         Serial.println("No se pudo abrir el archivo de credenciales.");
//         return;
//     }

//     StaticJsonDocument<192> doc;
//     DeserializationError error = deserializeJson(doc, file);
//     if (error) {
//         Serial.println("Error al deserializar JSON.");
//         return;
//     }

//     ssid = doc["ssid"].as<String>();
//     password = doc["password"].as<String>();
// }

// void WifiManager::saveCredentials(String ssid, String password) {
//     StaticJsonDocument<192> doc;
//     doc["ssid"] = ssid;
//     doc["password"] = password;

//     File file = LittleFS.open("/wifi.json", "w");
//     if (!file) {
//         Serial.println("No se pudo abrir archivo para guardar.");
//         return;
//     }

//     serializeJson(doc, file);
//     file.close();
//     Serial.println("Credenciales guardadas.");
// }

// bool WifiManager::tieneCredenciales() const {
//     return !ssid.isEmpty() && !password.isEmpty();
// }


// bool WifiManager::connectToWiFi() {
//     if (ssid.isEmpty() || password.isEmpty()) return false;

//     WiFi.mode(WIFI_AP_STA);
//     WiFi.begin(ssid.c_str(), password.c_str());

//     Serial.print("Conectando a ");
//     Serial.println(ssid);

//     for (int i = 0; i < 30; i++) {
//         if (WiFi.status() == WL_CONNECTED) {
//             Serial.println("Conectado a WiFi.");

//             WiFi.setSleep(false);
//             //WiFi.setLogLevel(WIFI_LOG_NONE);  no sirve

//             return true;
//         }
//         delay(1000);
//     }

//     Serial.println("Tiempo agotado. No se pudo conectar.");
//     return false;
// }

// void WifiManager::setupAP() {
//     WiFi.mode(WIFI_AP);
//     WiFi.softAP("WiFi Manager", "123456789");
//     Serial.println("Access Point creado: WiFi Manager");
// }

// /*
// void WifiManager::startCaptivePortal() {
//     dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());

//     server.on("/", std::bind(&WifiManager::handleRoot, this));
//     server.on("/save", std::bind(&WifiManager::handleSave, this));
//     server.on("/scan", std::bind(&WifiManager::handleScan, this));
//     server.onNotFound(std::bind(&WifiManager::handleNotFound, this));

//     server.begin();
//     Serial.println("Servidor web iniciado.");
// }
// */

// void WifiManager::handleRoot() {
//     String path = htmlPathPrefix + "index.html";
//     Serial.print("📄 Intentando abrir: ");
//     Serial.println(path);

//     if (!LittleFS.exists(path)) {
//         Serial.println("❌ No existe el archivo en LittleFS");
//         server.send(500, "text/html", "<h1>Error: index.html no encontrado</h1>");
//         return;
//     }

//     File file = LittleFS.open(path, "r");
//     if (!file || file.isDirectory()) {
//         Serial.println("❌ Error abriendo el archivo o es directorio");
//         server.send(500, "text/html", "<h1>Error abriendo index.html</h1>");
//         return;
//     }

//     String html = file.readString();
//     file.close();

//     Serial.println("✅ Archivo leído correctamente");
//     server.send(200, "text/html", html);
// }

// void WifiManager::handleSave() {
//     if (server.method() != HTTP_POST) {
//         server.send(405, "text/plain", "Método no permitido");
//         return;
//     }

//     String ssid = server.arg("ssid");
//     String password = server.arg("password");

//     if (ssid.isEmpty() || password.isEmpty()) {
//         Serial.println("❌ SSID o contraseña vacíos. No se guardará.");
//         mostrarPaginaError("Faltan datos para guardar.");
//         return;
//     }

//     StaticJsonDocument<192> doc;
//     doc["ssid"] = ssid;
//     doc["password"] = password;

//     File file = LittleFS.open("/wifi.json", "w");
//     if (!file) {
//         Serial.println("❌ No se pudo abrir /wifi.json para guardar.");
//         mostrarPaginaError("Error al guardar credenciales.");
//         return;
//     }

//     serializeJson(doc, file);
//     file.close();
//     Serial.println("✅ Credenciales guardadas.");

//     File success = LittleFS.open(htmlPathPrefix + "success.html", "r");
//     if (!success) {
//         server.send(200, "text/html", "<h1>Guardado. Reiniciando...</h1>");
//     } else {
//         server.send(200, "text/html", success.readString());
//         success.close();
//     }

//     delay(1000);
//     ESP.restart();
// }

// void WifiManager::mostrarPaginaError(const String& mensajeFallback) {
//     File errorFile = LittleFS.open(htmlPathPrefix + "error.html", "r");
//     if (!errorFile) {
//         server.send(500, "text/html", "<h1>Error: " + mensajeFallback + "</h1>");
//     } else {
//         server.send(500, "text/html", errorFile.readString());
//         errorFile.close();
//     }
// }

// void WifiManager::handleScan() {
//     Serial.println("🔍 Iniciando escaneo de redes WiFi...");

//     WiFi.mode(WIFI_AP_STA);
//     delay(100);

//     int n = WiFi.scanNetworks();
//     Serial.printf("📱 %d redes encontradas\n", n);

//     if (n <= 0) {
//         server.send(200, "application/json", "[]");
//         return;
//     }

//     DynamicJsonDocument doc(1024);
//     JsonArray arr = doc.to<JsonArray>();

//     for (int i = 0; i < n; ++i) {
//         JsonObject obj = arr.createNestedObject();
//         obj["ssid"] = WiFi.SSID(i);
//         obj["rssi"] = WiFi.RSSI(i);
//         obj["secure"] = (WiFi.encryptionType(i) != WIFI_AUTH_OPEN);
//     }

//     WiFi.scanDelete();

//     String output;
//     serializeJson(doc, output);
//     server.send(200, "application/json", output);

//     WiFi.mode(WIFI_AP);
// }

// void WifiManager::handleNotFound() {
//     server.sendHeader("Location", "/", true);
//     server.send(302, "text/plain", "");
// }

// void WifiManager::sincronizarHoraNTP() {
//     configTime(0, 0, "pool.ntp.org", "time.nist.gov");
//     Serial.println("⏳ Intentando sincronizar hora NTP...");

//     for (int j = 0; j < 20; j++) {
//         time_t now = time(nullptr);
//         if (now > 100000) {
//             Serial.print("✅ Hora sincronizada: ");
//             Serial.println(ctime(&now));
//             return;
//         }
//         delay(200);
//     }

//     Serial.println("⚠️ NTP no respondió. Continuando sin sincronizar.");
// }


// // void WifiManager::reintentarConexionSiNecesario() {
// //     if (connected) return;
    
// //     // 🚫 No intentar reconectar si estamos en modo AP (se está usando el portal cautivo)
// //     //if (WiFi.getMode() == WIFI_AP || WiFi.getMode() == WIFI_AP_STA) {
// //     //    return;
// //     //}

// //     unsigned long ahora = millis();
// //     if (ahora - ultimoIntentoWiFi < 10000) return; // Intentar cada 10 segundos

// //     ultimoIntentoWiFi = ahora;

// //     if (!ssid.isEmpty() && !password.isEmpty()) {
// //         Serial.println("🔁 Intentando reconexión WiFi...");
// //         if (connectToWiFi()) {
// //             Serial.println("🔌 Reconectado a WiFi.");
// //             sincronizarHoraNTP();
// //             connected = true;
// //         } else {
// //             Serial.println("❌ Reconexión WiFi fallida.");
// //         }
// //     }
// // }

// void WifiManager::reintentarConexionSiNecesario() {
//     if (connected) return;

//     unsigned long ahora = millis();
//     if (ahora - ultimoIntentoWiFi < 10000) return;

//     ultimoIntentoWiFi = ahora;

//     if (!ssid.isEmpty() && !password.isEmpty()) {
//         Serial.println("🔁 Intentando reconexión WiFi...");
//         WiFi.mode(WIFI_AP_STA);  // 🔄 aseguramos modo mixto por si quedó solo en AP
//         WiFi.begin(ssid.c_str(), password.c_str());

//         for (int i = 0; i < 10; i++) {
//             if (WiFi.status() == WL_CONNECTED) {
//                 Serial.println("🔌 Reconectado a WiFi.");
//                 sincronizarHoraNTP();
//                 connected = true;
//                 return;
//             }
//             delay(500);
//         }

//         Serial.println("❌ Reconexión WiFi fallida.");
//     }
// }


// // bool WifiManager::hayInternet() {
// //     HTTPClient http;
// //     http.begin("http://clients3.google.com/generate_204"); // URL rápida y liviana de Google
// //     int code = http.GET();
// //     http.end();
// //     return (code == 204);
// // }

// bool WifiManager::hayInternet() {
//     WiFiClient client;
//     HTTPClient http;

//     if (WiFi.status() != WL_CONNECTED) return false;

//     http.begin(client, "http://clients3.google.com/generate_204"); // Servicio liviano de Google
//     http.setConnectTimeout(3000);  // máximo 3 segundos

//     int httpCode = http.GET();
//     http.end();

//     return (httpCode == 204); // respuesta esperada si hay Internet real
// }
