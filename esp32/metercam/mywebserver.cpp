#include "mywebserver.h"


const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
</head>
<body>
  <div id="main">
    <h1>MeterCam</h1>
    <p>
      <button onclick="capturePhoto()">TAKE NEW PICTURE AND REFRESH PAGE</button>
      <button onclick="location.reload();">REFRESH PAGE</button>
      <button onclick="shutdown()">SHUT DOWN</button>
    </p>
    <p>Latest picture:</p>
    <div><img src="current-photo.jpg" id="photo"></div>
  </div>
</body>
<script>
  function capturePhoto() {
    var xhr = new XMLHttpRequest();
    xhr.open('GET', "/capture", true);
    xhr.send();
    setTimeout(function() {
      location.reload(); 
    }, 1500);
  }
</script>
</html>)rawliteral";

void init_webserver(AsyncWebServer & server, CamOperator * o)
{
  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send_P(200, "text/html", index_html);
  });

  server.on("/capture", HTTP_GET, [o](AsyncWebServerRequest * request) {
    o->requestPicture();
    request->send_P(200, "text/plain", "Taking Photo");
  });

  server.on("/current-photo.jpg", HTTP_GET, [o](AsyncWebServerRequest * request) {
    AsyncWebServerResponse *response = request->beginResponse_P(200, "image/jpg", o->getImgPtr(), o->getImgSizeInBytes());
    request->send(response);
    //request->send(LITTLEFS, FILE_PHOTO, "image/jpg", false);
  });
}
