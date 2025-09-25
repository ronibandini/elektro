// Robot Elektro  
// Roni Bandini, Mauro de Giuseppe, Septiembre 2025
// NodeMCU 1.0 ESP12E ESP8266  
// Motor A1A D3 (GPIO0) A1B D4 (GPIO2)
// DfPlayer RX D1 (GPIO5) TX D2 (GPIO4)
// Mouth Servo D5 (GPIO14)
// Ns4110 Amp 18 watts x 2 ch

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Servo.h>
#include <SoftwareSerial.h>
#include "DFRobotDFPlayerMini.h"

void motorA(char d);
void smoke();
void cough();
void mute();
void speak(int track);

// Servo config
const int servoPin = D5;

// Motor pins
const int A1A = D3;
const int A1B = D4;

// DFPlayer pins
const int playerRxPin = D2; 
const int playerTxPin = D1; 

// Servo mouth range
const int minPos = 20;
const int maxPos = 70;

// Delay range for mouth movement
const int minDelay = 150;
const int maxDelay = 300;

// for looping
int counter=0;
int counterSeconds=20;
long counterLimit=counterSeconds*1000;
const int smokingRounds=10;

// for audio
const int howManyAudios=33;
int saleGenerico=0;
int muteElektro=0;

Servo myservo;

SoftwareSerial mySerial(playerRxPin, playerTxPin);
DFRobotDFPlayerMini player;

// WiFi AP config
const char* ssid = "Elektro";
const char* password = "AAAAAAAAAAAAAAAA"; 
ESP8266WebServer server(80);

// HTML content for the web server
const char* HTML_CONTENT = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Elektro Control</title>
    <style>
        body { font-family: 'Inter', sans-serif; }
    </style>
</head>
<body class="bg-slate-900 text-white min-h-screen flex items-center justify-center p-4">
    <div class="bg-slate-800 p-8 rounded-2xl shadow-xl max-w-sm w-full text-center">
        <h1 class="text-4xl font-bold mb-6 text-slate-100">Elektro Control</h1>
        
        <div class="grid grid-cols-1 sm:grid-cols-2 gap-4 mb-6">
        <img src="data:image/png;base64,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">
        <br>
            <button onclick="sendCommand('speak')" class="bg-cyan-500 hover:bg-cyan-600 text-white font-bold py-4 px-6 rounded-xl shadow-lg transition duration-200">
                Speak
            </button>
            <button onclick="sendCommand('mute')" class="bg-orange-500 hover:bg-orange-600 text-white font-bold py-4 px-6 rounded-xl shadow-lg transition duration-200">
                Mute/unmute
            </button>
            <button onclick="sendCommand('smoke')" class="bg-purple-500 hover:bg-purple-600 text-white font-bold py-4 px-6 rounded-xl shadow-lg transition duration-200">
                Smoke
            </button>
            <button onclick="sendCommand('cough')" class="bg-fuchsia-500 hover:bg-fuchsia-600 text-white font-bold py-4 px-6 rounded-xl shadow-lg transition duration-200">
                Cough
            </button>
        </div>

        <div id="status" class="mt-4 text-sm text-slate-400">Ready to command.</div>
        
        <p class="mt-10 text-xs text-slate-500">Roni Bandini - Nerdearla 2025</p>
    </div>

    <script>
        // Sends a command to the ESP8266 server using the Fetch API
        async function sendCommand(endpoint) {
            const statusEl = document.getElementById('status');
            statusEl.textContent = 'Executing...';
            try {
                const response = await fetch(`/api/${endpoint}`);
                if (response.ok) {
                    const result = await response.text();
                    statusEl.textContent = 'Success: ' + result;
                } else {
                    statusEl.textContent = 'Error: ' + response.status;
                }
            } catch (error) {
                statusEl.textContent = 'Failed to connect to Elektro.';
                console.error(error);
            }
        }
    </script>
</body>
</html>
)rawliteral";

/////////////////////////////////////////////////////////////// SETUP

void setup() {
    
    randomSeed(analogRead(A0) ^ millis());

    pinMode(A1A, OUTPUT);
    pinMode(A1B, OUTPUT);

    myservo.attach(servoPin);
    myservo.write(0); 

    Serial.begin(115200);

    // DFPlayer
    mySerial.begin(9600);
    delay(2000); 

    myservo.write(90); 

    Serial.println("Elektro started");

    // Start DFPlayer
    if (player.begin(mySerial, /*isACK = */true, /*doReset = */true)) {
        Serial.println("DFPlayer ok.");
        player.volume(30); 
    } else {
        Serial.println("DFPlayer failed! Check wiring and SD card.");
    }

    // Start WiFi AP
    WiFi.softAP(ssid, password);
    Serial.print("AP started: ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.softAPIP());

    // Web server routes
    server.on("/", HTTP_GET, []() {
        server.send(200, "text/html", HTML_CONTENT);
    });

    // API endpoints for commands
    server.on("/api/speak", HTTP_GET, []() {
        speak(random(1, 9));
        server.send(200, "text/plain", "...Elektro spoke!");
        counter=0;
    });

    server.on("/api/smoke", HTTP_GET, []() {
        smoke();
        server.send(200, "text/plain", "...Elektro smoked!");
        counter=0;
    });

    server.on("/api/cough", HTTP_GET, []() {
        cough();
        server.send(200, "text/plain", "...Elektro coughed!");
        counter=0;
    });

    server.on("/api/mute", HTTP_GET, []() {      
        mute();
        if (muteElektro==1){
        server.send(200, "text/plain", "...Elektro muted!");}
        else
        {server.send(200, "text/plain", "...Elektro unmuted!");}
        counter=0;
    });

    server.begin();
    Serial.println("HTTP server started");

    // Greeting   
    speak(1);
    
    
}

////////////////////////////////////// MOTOR A

void motorA(char d) {
    if (d == 'R') {
        digitalWrite(A1A, LOW);
        digitalWrite(A1B, HIGH);
    } else if (d == 'L') {
        digitalWrite(A1A, HIGH);
        digitalWrite(A1B, LOW);
    } else {
        digitalWrite(A1A, LOW);
        digitalWrite(A1B, LOW);
    }
}

//////////////////////////////////////// SMOKE
void smoke() {
    
    Serial.println("Colocar cigarrillo");
    speak(10);
    myservo.write(0);
    delay(10000);    

    for (int i=0; i<smokingRounds; i++) {
        myservo.write(60);
        motorA('R'); delay(3000);
        myservo.write(30);
        motorA('O'); delay(3000);
        if (i==2 or i==5){cough();}
    }
    Serial.println("Smoking ended");
    cough();
}

////////////////////////////////////////  COUGH
void cough() {
    Serial.println("Coughing...");
    myservo.write(0);
    speak(11);
    delay(1000);
}

////////////////////////////////////// MUTE
void mute() {
    if (muteElektro==0){
      Serial.println("Mute!");
      muteElektro=1;
    }
    else{
      Serial.println("Unmute!");
      muteElektro=0;
    }
    
    
}

//////////////////////////////////////// SPEAK
void speak(int track) {

  if (muteElektro==0) {
      Serial.println("Speaking"); 
      Serial.println(track); 


      player.play(track);
      
      delay(500);
      while (true) {
        if (player.available()) {
          uint8_t type = player.readType();
          int value = player.read();
          if (type == DFPlayerPlayFinished) {
            break;
          }
        }
        
        int pos = random(minPos, maxPos + 1);   
        myservo.write(pos);  
        int wait = random(minDelay, maxDelay + 1);
        delay(wait);
      }
          
      Serial.println("Audio finalizado!");
      myservo.write(90);  

  } // mute 

}

//////////////////////////////////////// LOOP
void loop() {
    server.handleClient(); 
    delay(10);    
    counter++;
    Serial.println(String(counter)+"/"+String(counterLimit));

    if (counter>counterLimit){
      
      if (saleGenerico==1){
        Serial.println("Idle, call generico");
        int myTrack = random(1, 9);
        speak(myTrack);
        counter=0;
        saleGenerico=0;
        // assign a new limit
        int myLimit = random(700, 1000);
        counterLimit=counterSeconds*myLimit;
        Serial.println("New limit:");
        Serial.println(counterLimit);
      }
      else
      {
        Serial.println("Idle, call anuncios");
        int myTrack = random(12, howManyAudios);
        speak(myTrack);
        counter=0;
        saleGenerico=1;
        // assign a new limit
        int myLimit = random(700, 1000);
        counterLimit=counterSeconds*myLimit;
        Serial.println("New limit:");
        Serial.println(counterLimit);
      }// no sale generico

    }// counter

}// loop
