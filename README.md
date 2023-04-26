# ESP32 Captive Portal

## Description

Create a captive portal using the ESP's WiFi Access Point (AP) and ESPAsyncWebServer for the redirect and serving the captive portal website.

This code aims to be as thin and easy to integrate into your own project as possible. As such it does not provide its own web server or data for the captive portal. It uses whatever infrastructure you have already setup in your project anyway.

## Dependencies

* ESPAsyncWebServer library
* Arduino framework for ESP (WiFi, DNSServer and String)
* FreeRTOS (for spawning a background task)

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
    // stopping the portal when a button on PIN 5 is pressed
    if (digitalRead(5) == HIGH)
    {
        captive_portal_stop();
        //// If you want to keep the AP running (and only stop the redirect)
        // captive_portal_stop(true);
    }

}
```

## How it works

Depending on the OS of the device, it tries to connect to one of the following endpoints to get a specific result:

* Android (4.4+): http://clients3.google.com/generate_204 (HTTP code: 204)
* Android (6.0+): http://connectivitycheck.gstatic.com/generate_204 (HTTP code: 204)
* iOS: http://captive.apple.com (simple HTML page with "Success" in title and body)
* Windows (8 and older): http://www.msftncsi.com:80/ncsi.txt (Text: Microsoft NCSI)
* Windows (10+): http://www.msftconnecttest.com:80/connecttest.txt (Text: Microsoft Connect Test)
* Firefox: http://detectportal.firefox.com/canonical.html

This library sets up a DNSServer, which will redirect all DNS requests to the IP of the ESP's access point. Because the devices do not get the result they expect, they will assume a captive portal is redirecting all traffic and will show the respective pop-up.

The other thing this library does is set-up a HTTP redirect to an URL of your chosing, which will be the page actually displayed when the device opens the captive portal. This can be any URL you have already set-up on your ESPAsyncWebserver instance.

NOTE: This only applies to the AP and all devices connected to it. You can still use the STA part of the ESP's WiFi interface to connect to any network and access the internet normally (e.g. connect to an MQTT server).

### About Samsung phones

Samsung uses a hard-coded DNS to resolve connectivitycheck.gstatic.com for checking for an active internet connection. Therefore it ignores the DNS redirect and instead assumes the network has no connection at all. To get around this, the IP of the AP also needs to be changed to 8.8.8.8 (to match the hard coded DNS address).

This might conflict with some other things you are doing on the AP. So you can disable this behavior by adding the define `CAPTIVE_PORTAL_NO_SAMSUNG` to your build config. Note that Samsung phones will then not show the captive portal popup and might even flag the network as broken / having no internet access.

## Confirmed working

So far it has only been tested on the original ESP32, but it should work on the -C* and -S* models, as well as the ESP8266 (basically any controller also supported by ESPAsyncWebserver and the Arduino framework).

Phones:

* Sony Xperia ZX2 (Android 10)
* Huawai (2020ish model)
* iPad (2020ish model)
* iPhone (2020ish model)
* Samsung S7 (Android 8)


## Other notes

Compiling with `_DEBUG` defined will print a stack watermark for the DNS thread every 5 minutes.

Compiling with `DCORE_DEBUG_LEVEL` set to 4 or higher will print debug messages in certain situations.

## License

MIT-License

Begin license text.

Copyright 2023 Janek Schäfer

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the “Software”), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

End license text.