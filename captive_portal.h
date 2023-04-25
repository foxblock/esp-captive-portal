#ifndef _JS_CAPTIVE_PORTAL_H_INCLUDE
#define _JS_CAPTIVE_PORTAL_H_INCLUDE

#include <Arduino.h>

class AsyncWebServer;

// Configure and start an AP with a captive portal active.
// The captive portal redirects to the passed URL on the passed server.
// ssid is mandatory and max 63 chars
// password is 8 chars MINIMUM or pass nullptr for open AP
// Only one portal can be active at any given time!
//
// NOTE: This changes AP IP to 8.8.8.8 and starts a DNSServer on port 53.
// The DNS server is handled in a background task "CaptiveDnsServ" 
// which uses 2kB of stack memory.
// Setting the AP IP is required for the captive portal to work on Samsung phones.
// If you want to prevent AP reconfiguration add the
//      CAPTIVE_PORTAL_NO_SAMSUNG
// define to your build options.
// i.e. for platformIO add -DCAPTIVE_PORTAL_NO_SAMSUNG to build_flags 
// in platformio.ini
bool captive_portal_start(const char *ssid, const char *pass, AsyncWebServer *server, const String &redirectTargetURL);

// Stop a currently active captive portal.
// Pass true to keep the AP running and only stop the captive portal handlers
// and DNS server. (e.g. if you want the user to stay connected and do stuff,
// but no longer need the captive portal)
void captive_portal_stop(bool keepAPrunning = false);

#endif // _JS_CAPTIVE_PORTAL_H_INCLUDE