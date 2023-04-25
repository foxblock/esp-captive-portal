# ESP32 Captive Portal

## Description

Create a captive portal using the ESP32's WiFi Access Point (AP) and ESPAsyncWebServer for the redirect and serving the captive portal website.

This code aims to be as thin and easy to integrate into your own project as possible. As such it does not provide its own web server or data for the captive portal. It uses whatever infrastructure you have already setup in your project anyway.

## Dependencies

* ESPAsyncWebServer library

## Installing and Usage

* Copy the captive_portal header and source files to your project
* Add the ESPAsyncWebServer library (i.e. in PlatformIO add ESPAsyncWebServer to your lib_deps in platformio.ini or install any fork of it from the libraries interface)
* Here is some example code to get started:

```C++
#include <ESPAsyncWebServer.h>
#include "captive_portal.h"

// The server which will redirect the clients and serve the captive
// portal webpage
AsyncWebServer server(80);

void setup()
{
    server.on("/portal", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(SPIFFS, "/portal.html");
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
    // stopping the portal when a button on PIN 5 is pressed
    if (digitalRead(5) == HIGH)
    {
        captive_portal_stop();
        //// If you want to keep the AP running (and only stop the redirect)
        // captive_portal_stop(true);
    }

}
```

## About Samsung phones

Samsung uses a hard-coded DNS to resolve connectivitycheck.gstatic.com for checking for an active internet connection. Therefore the simple approach of just starting a DNS server does not work, the IP of the AP also needs to be changed to 8.8.8.8 (to match the hard coded address).

This might conflict with some other things you might be doing on the AP. So you can disable this behavior by adding the define `CAPTIVE_PORTAL_NO_SAMSUNG` to your build config. Note that Android and iPhones will still handle the captive portal as normal in this case, but Samsung phones will not react to it.

## License

MIT-License

Begin license text.

Copyright 2023 Janek Schäfer

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the “Software”), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

End license text.