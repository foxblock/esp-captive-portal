
// Captive Portal - what is needed
// - DNS Server + Catch-all Handler class
// - OR: DNS-Server + device specific request handlers (/generate_204, /success.html, etc.)
// -> these seem to work with recent stock Android and iPhones
//
// - IP-config seems to be necessary for all Samsung phones (since they use hard-coded
//   DNS for connectivitycheck.gstatic.com)

#include "captive_portal.h"

#include <WiFi.h>
#include <DNSServer.h>
#include <freertos/FreeRTOS.h>
#include <ESPAsyncWebServer.h>

static DNSServer _dnsServer;
static TaskHandle_t _dnsUpdateTask = nullptr;
static void _updateDnsTask(void *params);

static AsyncWebHandler *_captiveHandler = nullptr;
static AsyncWebServer *_serverExt = nullptr;

static bool _captiveIsStarted = false;

// Inspiration: https://github.com/andig/vzero/blob/master/src/webserver.cpp
class CaptiveRequestHandler : public AsyncWebHandler
{
public:
    explicit CaptiveRequestHandler(String redirectTargetURL) :
        targetURL("http://" + WiFi.softAPIP().toString() + redirectTargetURL)
    {
    }
    virtual ~CaptiveRequestHandler() {}

    const String targetURL;

    bool canHandle(AsyncWebServerRequest *request) override
    {
        // redirect if not in wifi client mode (through filter)
        // and request for different host (due to DNS * response)
        if (request->host() != WiFi.softAPIP().toString())
            return true;
        else
            return false;
    }

    void handleRequest(AsyncWebServerRequest *request) override
    {
        request->redirect(targetURL);
        log_d("Captive handler triggered. Requested %s%s -> redirecting to %s", request->host().c_str(), request->url().c_str(), targetURL.c_str());
    }
};

bool captive_portal_start(const char *ssid, const char *pass, AsyncWebServer *server, const String &redirectTargetURL)
{
    assert(server != nullptr);
    assert(_captiveIsStarted == false);

    // // Alternatively to calling the WiFi.* functions we could just configure
    // // the AP adapter here and rely on being started/handled externally.
    // // For this we need to get the right netif and copy the necessary
    // // parts of the WifiGeneric private function set_esp_interface_ip.
    // // Ultimately this might be the way to go, since we don't need half
    // // of what that function does and want to get rid of Wifi anyway.
    // esp_netif_t* netif = NULL;
    // netif = esp_netif_get_handle_from_ifkey("WIFI_AP_DEF");
    // // For STA-mode the parameter is "WIFI_STA_DEF"
    // // We need to copy the WifiGeneric internal (private) function:
    // set_esp_interface_ip(ESP_IF_WIFI_AP, apIP, apIP, subnet, 0);

    if (! WiFi.softAP(ssid, pass))
    {
        log_e("Captive portal failed to start: WiFi.softAP failed!");
        return false;
    }

#ifndef CAPTIVE_PORTAL_NO_SAMSUNG
    // Set AP IP 8.8.8.8 and subnet 255.255.255.0
    if (! WiFi.softAPConfig(0x08080808, 0x08080808, 0x00FFFFFF))
    {
        log_e("Captive portal failed to start: WiFi.softAPConfig failed!");
        WiFi.enableAP(false);
        return false;
    }
#endif

    if (! _dnsServer.start(53, "*", WiFi.softAPIP()))
    {
        log_e("Captive portal failed to start: no sockets for DNS server available!");
        WiFi.enableAP(false);
        return false;
    }
    _captiveHandler = new CaptiveRequestHandler(redirectTargetURL);
    _serverExt = server;
    _serverExt->addHandler(_captiveHandler).setFilter(ON_AP_FILTER); //only when requested from AP

    auto temp = xTaskCreatePinnedToCore(
                    _updateDnsTask,
                    "CaptiveDnsServ",
                    2048, // High watermark testing showed 1496 bytes usage max
                    nullptr,
                    1,
                    &_dnsUpdateTask,
                    0);
    if (temp != pdPASS)
    {
        _serverExt->removeHandler(_captiveHandler);
        delete _captiveHandler;
        _dnsServer.stop();
        WiFi.enableAP(false);
        return false;
    }

    _captiveIsStarted = true;

    log_d("Captive portal started. Redirecting all requests to %s", redirectTargetURL.c_str());

    return true;
}

void captive_portal_stop(bool keepAPrunning)
{
    if (!_captiveIsStarted)
        return;

    xTaskNotifyGive(_dnsUpdateTask);
    _dnsUpdateTask = nullptr;

    _serverExt->removeHandler(_captiveHandler);
    delete _captiveHandler;
    _serverExt = nullptr;
    _captiveHandler = nullptr;
    _dnsServer.stop();

    if (! keepAPrunning)
        WiFi.enableAP(false);

    _captiveIsStarted = false;
}

static void _updateDnsTask(void *params)
{
    log_d("Captive portal DNS update task started.");

    while (ulTaskNotifyTake(pdTRUE, 0) == 0)
    {
        _dnsServer.processNextRequest();
        delay(1);

#ifdef _DEBUG
        static unsigned long printTime = -300000;
        if (millis() - printTime > 300000)
        {
            log_d("Captive portal DNS update thread watermark: %d", uxTaskGetStackHighWaterMark(NULL));
            printTime = millis();
        }
#endif
    }

    log_d("Captive portal DNS update task shutting down...");

    vTaskDelete(nullptr);
}