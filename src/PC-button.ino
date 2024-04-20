#ifdef ESP32
#include <WiFi.h>
#include <WebServer.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#define WebServer ESP8266WebServer
#endif
#include <WebSocketsServer.h>

#define LED_BUILTIN 2 // LED integrado
const char* http_username = "admin";
const char* http_password = "admin123";
const char* ssid = "andy";
const char* password = "12345678i";

WebServer server(80);

WebSocketsServer webSocket = WebSocketsServer(81);

String  pwrbtnstate = "off";
String  rstbuttonstate = "off";
String  stsbuttonstate = "off";

const int PWRbtn = 0; // Botón de encendido
const int RSTbtn = 3; // Botón de reset
const int STSbtn = 3; // Boton de entrada de señal de control / Usa el mismo pin que el botón de reset
const long pwrDuration = 300; // Duración de presionar el botón (en milisegundos)
const long pwrForceDuration = 6500; // Duración de presionar el botón (en milisegundos)
const long rstDuration = 300; // Duración de presionar el botón (en milisegundos)

unsigned long lastSTSbtnCheck = 0; // Variable para almacenar el último momento en que se verificó el botón STSbtn
unsigned long STSbtnCheckInterval = 300; // Intervalo de tiempo entre cada verificación del botón STSbtn (en milisegundos)
unsigned long actualState = 0;
unsigned long lastState=0;
boolean auth = false;

void handleLogin() {
  String html = "<form action='/login' method='POST'>Username: <input type='text' name='username'><br>Password: <input type='password' name='password'><br><input type='submit' value='Submit'></form>";
  server.send(200, "text/html", html);
}

void handleLogout() {
  auth = false;
  server.sendHeader("Location", "/", true);
  server.send(302, "text/plain", "");
}

void handlePWRbtn() {
  if (auth) {
  Serial.println("Power button pressed on client.");
  Serial.println("Se enviara websocket ON");
  webSocket.broadcastTXT("ON");
  Serial.println("WEBSOCKET -ON- ENVIADO AL CLIENTE!");
  Serial.println("Apretando boton, y encendiendo led fisico");
  //digitalWrite(LED_BUILTIN, LOW); // Encender el LED
  digitalWrite(PWRbtn, LOW);
  pwrbtnstate = "on";
  delay(pwrDuration);
  digitalWrite(PWRbtn, HIGH);
  Serial.println("Boton soltado");
  pwrbtnstate = "off";
  //digitalWrite(LED_BUILTIN, HIGH); // Apagar el LED
  Serial.println("LED fisico apagado");
  Serial.println("Se enviara websocket OFF");
  webSocket.broadcastTXT("OFF");
  Serial.println("WEBSOCKET -OFF- ENVIADO AL CLIENTE!");
  server.send(200,"OK");
  Serial.println("Se envio respuesta 200 al cliente");
  }
}

void handleForcePWRbtn() {
  if (auth) {
  webSocket.broadcastTXT("ONFRC");
  //digitalWrite(LED_BUILTIN, LOW); // Encender el LED
  digitalWrite(PWRbtn, LOW);
  pwrbtnstate = "on";
  delay(pwrForceDuration);
  digitalWrite(PWRbtn, HIGH);
  pwrbtnstate = "off";
  //digitalWrite(LED_BUILTIN, HIGH); // Apagar el LED
  webSocket.broadcastTXT("OFFFRC");
  server.send(200,"OK");
  }
}

void handleRSTbtn() {
  if (auth) {
  Serial.println("RST button pressed on client.");
  pinMode(STSbtn, OUTPUT);
  webSocket.broadcastTXT("RSTON");
  //digitalWrite(LED_BUILTIN, LOW); // Encender el LED
  digitalWrite(RSTbtn, LOW);
  rstbuttonstate = "on";
  delay(rstDuration);
  digitalWrite(RSTbtn, HIGH);
  rstbuttonstate = "off";
  //digitalWrite(LED_BUILTIN, HIGH); // Apagar el LED
  webSocket.broadcastTXT("RSTOFF");
  server.send(200,"OK");
  pinMode(STSbtn,INPUT);
  }
}

void handleSTSbtn(){
  if (auth) {
  if (actualState == HIGH) {
    Serial.println("Estado de encendido del PC: ON.");
    webSocket.broadcastTXT("STSON");
    stsbuttonstate = "on";
  }else{
    Serial.println("Estado de encendido del PC: APAGADO.");
    webSocket.broadcastTXT("STSOFF");
    stsbuttonstate = "off";
  }
  }
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  switch(type) {
    //case WStype_TEXT: {
      //String message = "";
      //for (size_t i = 0; i < length; i++) {
      //  message += (char)payload[i];
      //}
      //Serial.printf("[%u] Text received: %s\n", num, message.c_str());
      //if (message == "ON") {
      //  pwrbtnstate = "on";
      //} else if (message == "OFF") {
      //  pwrbtnstate = "off";
      //}
      //break;
    //}
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
  String stsbtncolor = actualState == HIGH ? "green" : "#ECECEC"; //la pagina carga el estado por primera vez
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
      #circle, #circlerst{ 
        display: inline-block;
        margin-left: 5px; 
        width: 25px; 
        height: 25px; 
        border-radius: 50%; 
        background-color: #ECECEC; 
        color: #171717;
        margin-top: 25px; 
      }
      #circlepwr{ 
        display: inline-block;
        margin-left: 5px; 
        width: 25px; 
        height: 25px; 
        border-radius: 50%; 
        background-color: )=====" + stsbtncolor + R"=====(; 
        color: #171717;
        margin-top: 25px; 
      }

      body {
            margin: 0;
            font-family: Arial, sans-serif;
        }
        .overlay {
            position: fixed;
            width: 100%;
            height: 100%;
            top: 0;
            left: 0;
            background-color: rgba(0,0,0,0.5);
            display: flex;
            align-items: center;
            justify-content: center;
        }
        .login-form {
            background-color: #ECECEC;
            padding: 20px;
            border-radius: 8px;
        }
        .login-form input[type="text"],
        .login-form input[type="password"] {
            width: 100%;
            padding: 10px;
            margin: 5px 0;
            border: none;
            border-radius: 4px;
            box-sizing: border-box;
        }
        .login-form input[type="submit"] {
            width: 100%;
            padding: 10px;
            margin-top: 10px;
            border: none;
            border-radius: 4px;
            background-color: #007bff;
            color: white;
            cursor: pointer;
        }
        .login-form input[type="submit"]:hover {
            background-color: #0056b3;
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
      <p id="statusText">Power LED: <span id="circlepwr"></span></p> <!-- Texto para el estado del círculo -->
      <p id="statusText">Real power signal: <span id="circle"></span></p> <!-- Texto para el estado del círculo -->
      <p id="statusText">Real reset signal: <span id="circlerst"></span></p> <!-- Texto para el estado del círculo -->
      
    </div>
    <script>
      var pwrBtn = document.getElementById('pwrBtn');
      var ForcepwrBtn = document.getElementById('ForcepwrBtn');
      var RSTbtn = document.getElementById('RSTbtn');
      var circlepwr = document.getElementById('circlepwr');
      var circle = document.getElementById('circle'); // Obtener el elemento del círculo
      var circlerst = document.getElementById('circlerst');
      var statusText = document.getElementById('statusText'); // Obtener el elemento del texto de estado
      var authenticated = )=====" + (auth) + R"=====(;
      //--------------------------------------EVENTOS EN NAVEGADOR PC -----------------------------------------

      // Verificar si es un dispositivo móvil
      var isMobile = /iPhone|iPad|iPod|Android/i.test(navigator.userAgent);
      if (isMobile) {
          // Agregar eventos táctiles solo en dispositivos móviles
          pwrBtn.addEventListener('touchstart', function() {
            if (authenticated) {
              var xhr = new XMLHttpRequest();
              xhr.open('GET', '/PWRbtn', true);
              xhr.send();
              pwrBtn.style.backgroundColor = 'green';
            } else {
                showLoginPopup();
            }
          });

          pwrBtn.addEventListener('touchend', function() {
              pwrBtn.style.backgroundColor = '#ECECEC';
          });

          ForcepwrBtn.addEventListener('touchstart', function() {
            if (authenticated) {
              var xhr = new XMLHttpRequest();
              xhr.open('GET', '/forcePWRbtn', true);
              xhr.send();
              ForcepwrBtn.style.backgroundColor = 'green';
            } else {
                showLoginPopup();
            }

          });

          ForcepwrBtn.addEventListener('touchend', function() {
              ForcepwrBtn.style.backgroundColor = '#ECECEC';
          });

          RSTbtn.addEventListener('touchstart', function() {
            if (authenticated) {
              var xhr = new XMLHttpRequest();
              xhr.open('GET', '/RSTbtn', true);
              xhr.send();
              RSTbtn.style.backgroundColor = 'green';
            } else {
                showLoginPopup();
            }
          });

          RSTbtn.addEventListener('touchend', function() {
              RSTbtn.style.backgroundColor = '#ECECEC';
          });
      } else {
          // Mantener eventos de mouse solo para dispositivos de escritorio
          pwrBtn.addEventListener('mousedown', function() {
            if (authenticated) {
              var xhr = new XMLHttpRequest();
              xhr.open('GET', '/PWRbtn', true);
              xhr.send();
              pwrBtn.style.backgroundColor = 'green';
            } else {
                showLoginPopup();
            }
          });

          pwrBtn.addEventListener('mouseup', function() {
              pwrBtn.style.backgroundColor = '#ECECEC';
          });

          ForcepwrBtn.addEventListener('mousedown', function() {
            if (authenticated) {
              var xhr = new XMLHttpRequest();
              xhr.open('GET', '/forcePWRbtn', true);
              xhr.send();
              ForcepwrBtn.style.backgroundColor = 'green';
            } else {
                showLoginPopup();
            }
          });

          ForcepwrBtn.addEventListener('mouseup', function() {
              ForcepwrBtn.style.backgroundColor = '#ECECEC';
          });

          RSTbtn.addEventListener('mousedown', function() {
            if (authenticated) {
              var xhr = new XMLHttpRequest();
              xhr.open('GET', '/RSTbtn', true);
              xhr.send();
              RSTbtn.style.backgroundColor = 'green';
            } else {
                showLoginPopup();
            }
          });

          RSTbtn.addEventListener('mouseup', function() {
              RSTbtn.style.backgroundColor = '#ECECEC';
          });
      }
      
      function showLoginPopup() {
        var loginForm = document.createElement('form');
        loginForm.className = 'login-form';
        loginForm.setAttribute('action', '/login');
        loginForm.setAttribute('method', 'POST');
        loginForm.innerHTML = "Username: <input type='text' name='username'><br>Password: <input type='password' name='password'><br><input type='submit' value='Submit'>";

        var overlay = document.createElement('div');
        overlay.className = 'overlay';

        overlay.appendChild(loginForm);
        document.body.appendChild(overlay);

        loginForm.addEventListener('submit', function(event) {
            event.preventDefault();
            var username = loginForm.elements['username'].value;
            var password = loginForm.elements['password'].value;
            var xhr = new XMLHttpRequest();
            xhr.open('POST', '/login', true);
            xhr.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded');
            xhr.onreadystatechange = function() {
                if (xhr.readyState === 4) {
                    if (xhr.status === 200) {
                        overlay.style.display = 'none';
                        authenticated = true;
                    } else {
                        alert('Authentication failed');
                    }
                }
            };
            xhr.send('username=' + username + '&password=' + password);
        });
      }

      //---------------------------------------WebSocket---------------------------------------------------------

      var webSocket = new WebSocket('ws://' + window.location.hostname + ':81/');
      webSocket.onmessage = function(event) {

        if (event.data === 'ON') {
          circle.style.backgroundColor = 'blue'; // Cambiar el color del círculo cuando el LED se enciende
        } else if (event.data === 'OFF') {
          setTimeout(function() {
            circle.style.backgroundColor = '#ECECEC'; // Cambiar el color del círculo cuando el LED se apaga
          }, 200); // Agrega 200ms por si el OFF llega junto con el ON por retraso en la red
        }

        if (event.data === 'RSTON') {
          circlerst.style.backgroundColor = 'blue'; // Cambiar el color del círculo cuando el LED se enciende
          setTimeout(function() {
          circle.style.backgroundColor = '#ECECEC'; // Cambiar el color de vuelta después de un cierto tiempo
          }, 300); // Cambia 3000 por el tiempo en milisegundos que desees
        } else if (event.data === 'RSTOFF') {
            setTimeout(function() {
              circlerst.style.backgroundColor = '#ECECEC'; // Cambiar el color del círculo cuando el LED se apaga
          }, 200); // Agrega 200ms por si el OFF llega junto con el ON por retraso en la red
        }

        if (event.data === 'ONFRC') {
          circle.style.backgroundColor = 'blue'; // Cambiar el color del círculo cuando el LED se enciende
        } else if (event.data === 'OFFFRC') {
          setTimeout(function() {
            circle.style.backgroundColor = '#ECECEC'; // Cambiar el color del círculo cuando el LED se apaga
          }, 1000); // Agrega 1000ms por si el OFF llega junto con el ON por retraso en la red
        }

        if (event.data === 'STSON') {
          circlepwr.style.backgroundColor = 'green'; // Cambiar el color del círculo cuando el LED se enciende
        } else if (event.data === 'STSOFF') {
          circlepwr.style.backgroundColor = '#ECECEC'; // Cambiar el color del círculo cuando el LED se enciende
        }
      };
    </script>
  </body>
  </html>)=====";
  server.send(200, "text/html", html);
}

void handleLoginRequest() {
  String username = server.arg("username");
  String password = server.arg("password");
  if (username.equals(http_username) && password.equals(http_password)) {
    auth = true;
    server.sendHeader("Location", "/", true);
    server.send(302, "text/plain", "");
  } else {
    server.send(403, "text/plain", "Access denied");
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(PWRbtn, OUTPUT);
  digitalWrite(PWRbtn, HIGH);
  //pinMode(LED_BUILTIN, OUTPUT);
  //digitalWrite(LED_BUILTIN, HIGH); // Apagar el LED al inicio
  pinMode(STSbtn, INPUT);
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
  server.on("/login", HTTP_POST, handleLoginRequest);
  server.on("/logout", handleLogout);
  server.on("/PWRbtn", handlePWRbtn);
  server.on("/forcePWRbtn", handleForcePWRbtn);
  server.on("/RSTbtn", handleRSTbtn);
  //server.on("/STSbtn", handleSTSbtn);
  server.begin();
  
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
  //actualState = digitalRead(STSbtn);
}



void loop() {
  server.handleClient();
  webSocket.loop();
  unsigned long currentMillis = millis(); // Obtener el tiempo actual
  // Verificar si ha pasado el intervalo de tiempo desde la última verificación del botón STSbtn
  if (currentMillis - lastSTSbtnCheck >= STSbtnCheckInterval) {
    actualState = digitalRead(STSbtn);
    if (lastState != actualState){
      Serial.println("Se detecto un cambio en el estado del boton STSbtn");
      handleSTSbtn(); // Llamar a la función handleSTSbtn
    }
    lastSTSbtnCheck = currentMillis; // Actualizar el tiempo de la última verificación
    lastState = actualState;
  }
}
