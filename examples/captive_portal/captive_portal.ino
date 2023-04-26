// Minimalist captive portal example
// 
// In your codebase you would use your existing AsyncWebServer instance

#include <ESPAsyncWebServer.h>
#include "captive_portal.h"

// The server which will redirect the clients and serve the captive portal webpage
AsyncWebServer server(80);

void setup()
{
    server.on("/portal", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(200, "text/plain", "Hello from the captive portal");
        Serial.printf("Requested %s%s\n", request->host().c_str(), request->url().c_str());
    });
    server.begin();

    //// Start unencrypted and redirect to the handler we set up above
    captive_portal_start("MyESPProject", nullptr, &server, "/portal");
    //// Example with password (min 8 chars)
    // captive_portal_start("MyESPProject", "esp32esp", &server, "/portal");
    //// Redirect to root (not setup here)
    // captive_portal_start("MyESPProject", nullptr, &server, "/");
    //// Redirect to index.html (not setup here)
    // captive_portal_start("MyESPProject", nullptr, &server, "/index.html");
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