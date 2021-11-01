#include <DNSServer.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncWebServer.h>
#include <FS.h>

#define DNS_PORT 53

IPAddress apIP(192, 168, 1, 1);
DNSServer dns_server;
AsyncWebServer web_server(80);

static size_t connections = 0;

const char captive_portal_payload[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html>
  <head>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <meta http-equiv="refresh" content="3;url=/connected" />
    <style>
      * {
        font-family: Arial;
      }
      h1 {
        text-align: center;
      }
      .loader {
          border: 16px solid #f3f3f3; /* Light grey */
          border-top: 16px solid #3498db; /* Blue */
          border-radius: 50%;
          width: 60px;
          height: 60px;
          -webkit-animation: spin 2s linear infinite; /* Safari */
          animation: spin 2s linear infinite;       
          margin: auto; 
      }

      @-webkit-keyframes spin {
        0% { -webkit-transform: rotate(0deg); }
        100% { -webkit-transform: rotate(360deg); }
      }
      
      @keyframes spin {
        0% { transform: rotate(0deg); }
        100% { transform: rotate(360deg); }
      }
      .center-container {
        text-align: center;
        display: block;
      }
    </style>
  </head>
  <body>

    </br>
    </br>
    </br>
    <div class="loader"></div>
    <div class="center-container">
        <h1>Loading...</h1>
    </div>
    
  </body>
</html>
)rawliteral";

const char connected_payload[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html>
  <head>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
      .center-container {
        text-align: center;
        display: block;
      }
      * {s
        font-family: Arial;
      }
      h2 {
        text-align: center;
      }
    </style>
  </head>
  <body>
    <div class="center-container">
      <img src="rr">
    </div>
    <h2>You have quite frankly been bamboozled my dude. No free internet for you</h2>
  </body>
</html>
)rawliteral";

void setup() {

    if (!SPIFFS.begin()) {
        return;
    }

    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
    WiFi.softAP("Free internet");

    dns_server.setErrorReplyCode(DNSReplyCode::ServerFailure);
    dns_server.start(DNS_PORT, "*", apIP);

    // We use on not found for captive portal since all different CPs will
    // redirect to some random domain
    web_server.onNotFound([](AsyncWebServerRequest *request) {
        request->send_P(200, "text/html", captive_portal_payload);
    });

    web_server.on("/connected", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send_P(200, "text/html", connected_payload);
        connections++;
    });

    web_server.on("/rr", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(SPIFFS, "/rr.gif", "image/gif");
    });

    web_server.on("/status", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/html", String(connections));
    });

    web_server.begin();
}

void loop() { dns_server.processNextRequest(); }
