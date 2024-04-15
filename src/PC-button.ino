#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WebSocketsServer.h>

const char* ssid = "Telecentro-c318";
const char* password = "CDZMWMYKDZGD";

ESP8266WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);

String  pwrbtnstate = "off";
String  rstbuttonstate = "off";
#define LED_BUILTIN 2 // LED integrado
const int PWRbtn = 5; // Botón de encendido
const int RSTbtn = 4; // Botón de reset
const long pwrDuration = 500; // Duración de presionar el botón (en milisegundos)
const long pwrForceDuration = 6500; // Duración de presionar el botón (en milisegundos)
const long rstDuration = 200; // Duración de presionar el botón (en milisegundos)


void handlePWRbtn() {
  digitalWrite(LED_BUILTIN, LOW); // Encender el LED
  digitalWrite(PWRbtn, LOW);
    pwrbtnstate = "on";
  Serial.println("Power button pressed. LED on.");
  webSocket.broadcastTXT("ON");
  delay(pwrDuration);
  digitalWrite(PWRbtn, HIGH);
    pwrbtnstate = "off";
  digitalWrite(LED_BUILTIN, HIGH); // Apagar el LED
  Serial.println("Power button released. LED off.");
  webSocket.broadcastTXT("OFF");
  handleRoot();
}

void handleForcePWRbtn() {
  digitalWrite(LED_BUILTIN, LOW); // Encender el LED
  digitalWrite(PWRbtn, LOW);
    pwrbtnstate = "on";
  Serial.println("Power button pressed. LED on.");
  webSocket.broadcastTXT("ON");
  delay(pwrForceDuration);
  digitalWrite(PWRbtn, HIGH);
    pwrbtnstate = "off";
  digitalWrite(LED_BUILTIN, HIGH); // Apagar el LED
  Serial.println("Power button released. LED off.");
  webSocket.broadcastTXT("OFF");
  handleRoot();
}

void handleRSTbtn() {
  digitalWrite(LED_BUILTIN, LOW); // Encender el LED
  digitalWrite(RSTbtn, LOW);
  rstbuttonstate = "on";
  Serial.println("Power button pressed. LED on.");
  webSocket.broadcastTXT("RSTON");
  delay(rstDuration);
  digitalWrite(RSTbtn, HIGH);
  rstbuttonstate = "off";
  digitalWrite(LED_BUILTIN, HIGH); // Apagar el LED
  Serial.println("Power button released. LED off.");
  webSocket.broadcastTXT("RSTOFF");
  handleRoot();
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  switch(type) {
    case WStype_TEXT: {
      String message = "";
      for (size_t i = 0; i < length; i++) {
        message += (char)payload[i];
      }
      Serial.printf("[%u] Text received: %s\n", num, message.c_str());
      if (message == "ON") {
        pwrbtnstate = "on";
      } else if (message == "OFF") {
        pwrbtnstate = "off";
      }
      break;
    }
    case WStype_CONNECTED:
      Serial.printf("[%u] Connected!\n", num);
      break;
    case WStype_DISCONNECTED:
      Serial.printf("[%u] Disconnected!\n", num);
      break;
    case WStype_ERROR:
      Serial.printf("[%u] Error!\n", num);
      break;
    case WStype_PING:
      // PING es manejado automáticamente por la librería WebSockets
      break;
    case WStype_PONG:
      // PONG es manejado automáticamente por la librería WebSockets
      break;
    default:
      break;
  }
}

void handleRoot() {
  // Determinar el color del botón y del texto
  String btnColor = pwrbtnstate == "on" ? "#ECECEC" : "#171717";

  // Duración en milisegundos para mantener el botón verde
  int pwrDuration = 2000; // Cambia este valor según tus necesidades

  // HTML enviado al cliente al cargar la página
  String html = R"=====(<!DOCTYPE html>
<html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="icon" href="data:,">
  <style>
    html { 
      font-family: Helvetica; 
      display: inline-block; 
      margin: 0px auto; 
      text-align: center;
    }
    .button { 
      background-color: #ECECEC; 
      border: none; 
      color: #171717; 
      padding: 16px 40px; 
      text-decoration: none; 
      font-size: 30px; 
      margin: 2px; 
      cursor: pointer; 
      width: 150px; 
      border-radius: 30px; /* Bordes redondeados */
    }
    #circle, #circlerst { 
      display: inline-block;
      margin-left: 5px; 
      width: 25px; 
      height: 25px; 
      border-radius: 50%; 
      background-color: #ECECEC; 
      color: #171717;
      margin-top: 25px; 
    }
    #container { 
      display: flex; 
      flex-direction: column; 
      align-items: center; 
    }
    #statusText { 
      font-size: 20px; 
      color: #ECECEC; 
    }
  </style>
</head>
<body style="background-color: #171717;">
  <h1 style="color: white;">ESP8266 PC Buttons</h1>
  <div id="container"> <!-- Contenedor para centrar el botón y el círculo -->
    <p><button id="pwrBtn" class="button">PWR</button></p>
    <p><button id="ForcepwrBtn" class="button">Force-PWR</button></p>
    <p><button id="RSTbtn" class="button">RST</button></p>
    <p id="statusText">Real power signal: <span id="circle"></span></p> <!-- Texto para el estado del círculo -->
    <p id="statusText">Real reset signal: <span id="circlerst"></span></p> <!-- Texto para el estado del círculo -->
     
  </div>
  <script>
    var pwrBtn = document.getElementById('pwrBtn');
    var ForcepwrBtn = document.getElementById('ForcepwrBtn');
    var RSTbtn = document.getElementById('RSTbtn');
    var circle = document.getElementById('circle'); // Obtener el elemento del círculo
    var circlerst = document.getElementById('circlerst');
    var statusText = document.getElementById('statusText'); // Obtener el elemento del texto de estado

    //--------------------------------------EVENTOS EN NAVEGADOR PC -----------------------------------------

    // Verificar si es un dispositivo móvil
    var isMobile = /iPhone|iPad|iPod|Android/i.test(navigator.userAgent);
    if (isMobile) {
        // Agregar eventos táctiles solo en dispositivos móviles
        pwrBtn.addEventListener('touchstart', function() {
            var xhr = new XMLHttpRequest();
            xhr.open('GET', '/PWRbtn', true);
            xhr.send();
            pwrBtn.style.backgroundColor = 'green';
        });

        pwrBtn.addEventListener('touchend', function() {
            pwrBtn.style.backgroundColor = '#ECECEC';
        });

        ForcepwrBtn.addEventListener('touchstart', function() {
            var xhr = new XMLHttpRequest();
            xhr.open('GET', '/forcePWRbtn', true);
            xhr.send();
            ForcepwrBtn.style.backgroundColor = 'green';
        });

        ForcepwrBtn.addEventListener('touchend', function() {
            ForcepwrBtn.style.backgroundColor = '#ECECEC';
        });

        RSTbtn.addEventListener('touchstart', function() {
            var xhr = new XMLHttpRequest();
            xhr.open('GET', '/RSTbtn', true);
            xhr.send();
            RSTbtn.style.backgroundColor = 'green';
        });

        RSTbtn.addEventListener('touchend', function() {
            RSTbtn.style.backgroundColor = '#ECECEC';
        });
    } else {
        // Mantener eventos de mouse solo para dispositivos de escritorio
        pwrBtn.addEventListener('mousedown', function() {
            var xhr = new XMLHttpRequest();
            xhr.open('GET', '/PWRbtn', true);
            xhr.send();
            pwrBtn.style.backgroundColor = 'green';
        });

        pwrBtn.addEventListener('mouseup', function() {
            pwrBtn.style.backgroundColor = '#ECECEC';
        });

        ForcepwrBtn.addEventListener('mousedown', function() {
            var xhr = new XMLHttpRequest();
            xhr.open('GET', '/forcePWRbtn', true);
            xhr.send();
            ForcepwrBtn.style.backgroundColor = 'green';
        });

        ForcepwrBtn.addEventListener('mouseup', function() {
            ForcepwrBtn.style.backgroundColor = '#ECECEC';
        });

        RSTbtn.addEventListener('mousedown', function() {
            var xhr = new XMLHttpRequest();
            xhr.open('GET', '/RSTbtn', true);
            xhr.send();
            RSTbtn.style.backgroundColor = 'green';
        });

        RSTbtn.addEventListener('mouseup', function() {
            RSTbtn.style.backgroundColor = '#ECECEC';
        });
    }

    //---------------------------------------WebSocket---------------------------------------------------------

    var webSocket = new WebSocket('ws://' + window.location.hostname + ':81/');
    webSocket.onmessage = function(event) {
      if (event.data === 'ON') {
        circle.style.backgroundColor = 'blue'; // Cambiar el color del círculo cuando el LED se enciende
      } else if (event.data === 'OFF') {
        circle.style.backgroundColor = '#ECECEC'; // Cambiar el color del círculo cuando el LED se apaga
      }
      if (event.data === 'RSTON') {
        circlerst.style.backgroundColor = 'blue'; // Cambiar el color del círculo cuando el LED se enciende
      } else if (event.data === 'RSTOFF') {
        circlerst.style.backgroundColor = '#ECECEC'; // Cambiar el color del círculo cuando el LED se enciende
      }
    };
  </script>
</body>
</html>)=====";
  server.send(200, "text/html", html);
}

void setup() {
  Serial.begin(115200);
  pinMode(PWRbtn, OUTPUT);
  digitalWrite(PWRbtn, HIGH);
  pinMode(RSTbtn, OUTPUT);
  digitalWrite(RSTbtn, HIGH);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH); // Apagar el LED al inicio

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  server.on("/", handleRoot);
  server.on("/PWRbtn", handlePWRbtn);
  server.on("/forcePWRbtn", handleForcePWRbtn);
  server.on("/RSTbtn", handleRSTbtn);
  server.begin();
  
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
}

void loop() {
  server.handleClient();
  webSocket.loop();
}