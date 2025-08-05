#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <WiFi.h>
#include <WebServer.h>
#include <LittleFS.h>

/**
 * @class WifiManager
 * @brief Clase para gestionar conexión WiFi con almacenamiento de credenciales y portal cautivo.
 */
class WifiManager {
public:
    /** Constructor
     *  @param ledPin    Pin LED de estado
     *  @param buttonPin Pin botón borrado de credenciales
     */
    WifiManager(uint8_t ledPin = 2, uint8_t buttonPin = 0);

    // -------- ciclo de vida ----------
    void begin();
    void run();
    void update();                        // atiende servidor HTTP

    // -------- utilidades -------------
    void setHtmlPathPrefix(const String& prefix);
    bool isConnected();
    int  getSignalStrength();
    uint64_t getTimestamp();
    bool connectToWiFi();
    void reintentarConexionSiNecesario();
    bool hayInternet();
    bool tieneCredenciales() const;
    void setAutoReconnect(bool habilitado);

    /* ===== NUEVO: recuperación automática tras caída de Wi‑Fi ===== */
    bool scanRedDetectada();      ///< ¿el SSID guardado volvió a aparecer?
    void forzarReconexion();      ///< llama WiFi.begin() manteniendo el AP

private:
    // -------- portal AP -------------
    void setupAP();
    void handleRoot();
    void handleSave();
    void handleScan();
    void handleNotFound();
    void mostrarPaginaError(const String& mensajeFallback);

    // -------- credenciales ----------
    void loadCredentials();
    void saveCredentials(String ssid, String password);
    void eraseCredentials();

    // -------- NTP -------------------
    void sincronizarHoraNTP();

    // -------- datos -----------------
    String ssid, password;
    String htmlPathPrefix = "/";

    unsigned long ultimoIntentoWiFi = 0;
    unsigned long ultimoScan       = 0;                 ///< NUEVO
    static constexpr unsigned long SCAN_INTERVAL_MS = 15000; ///< NUEVO

    WebServer server{80};
    bool connected = false;

    uint8_t ledPin, buttonPin;
};

#endif




// En WifiManager.h (CORREGIDO)
// #ifndef WIFI_MANAGER_H
// #define WIFI_MANAGER_H

// #include <WiFi.h>
// #include <WebServer.h>
// #include <LittleFS.h>

// class WifiManager {
// public:
//     WifiManager();
//     void begin();
//     void run();
//     void setHtmlPathPrefix(const String& prefix);
//     void update();
//     bool isConnected();
//     int getSignalStrength();
//     uint64_t getTimestamp();
//     bool connectToWiFi();
//     void reintentarConexionSiNecesario();
//     bool hayInternet();
//     bool tieneCredenciales() const;


// private:
//     String htmlPathPrefix = "/";
//     void setupAP();
//     void handleRoot();
//     void handleSave();
//     void handleScan();
//     void handleNotFound();
//     void loadCredentials();
//     void saveCredentials(String ssid, String password);
//     void eraseCredentials();
//     void mostrarPaginaError(const String& mensajeFallback);
//     void sincronizarHoraNTP();

//     // SOLO UNA VEZ:
//     String ssid;
//     String password;
//     unsigned long ultimoIntentoWiFi = 0;
//     WebServer server;
//     bool connected = false;
// };

// #endif
