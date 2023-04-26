// Minimalist captive portal example
// Starts a WiFi AP called "MyESPProject", which will show the captive
// portal popup upon connecting with any device like a laptop or phone.
//
// The captive portal redirect supports any target you have set-up in
// the AsyncWebServer (so it can be a hard-coded text, file from SPIFFS,
// basically anything that library supports)
// 
// In your project you would use your existing AsyncWebServer instance

#include <ESPAsyncWebServer.h>
#include "captive_portal.h"

// The server which will redirect the clients and serve the captive portal webpage
AsyncWebServer server(80);

void setup()
{
    //// Start unencrypted and redirect to the handler we set up below
    captive_portal_start("MyESPProject", nullptr, &server, "/portal");
    //// Example with password (min 8 chars)
    // captive_portal_start("MyESPProject", "esp32esp", &server, "/portal");
    //// Redirect to root (not setup here)
    // captive_portal_start("MyESPProject", nullptr, &server, "/");
    //// Redirect to index.html (not setup here)
    // captive_portal_start("MyESPProject", nullptr, &server, "/index.html");

    // In this minimalist example we have to start the server after the captive portal
    // (which starts the AP). In your code you can do it the other way around, if you
    // init any WiFi interface before starting the server.
    server.on("/portal", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(200, "text/plain", "Hello from the captive portal");
        log_d("Requested %s%s", request->host().c_str(), request->url().c_str());
    });
    server.begin();
}

void loop()
{
    // stopping the portal when a button on PIN 4 is pressed
    if (digitalRead(4) == HIGH)
    {
        captive_portal_stop();
        //// If you want to keep the AP running (and only stop the redirect)
        // captive_portal_stop(true);
    }
}