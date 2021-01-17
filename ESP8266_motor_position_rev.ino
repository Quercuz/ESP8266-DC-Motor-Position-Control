//ESP8266 motor_position_rev

//comments on project: https://create.arduino.cc/projecthub/whitebank/dc-motor-position-control-986c10?f=1
//author https://create.arduino.cc/projecthub/whitebank

//commented 17-1-2021 Quercus62
//tested on NodeMCU V3 new version small footprint
//Arduino IDE 1.8.13
//ESP boards 2.7.4
//FS 4MB (FS:3MB OTA...) 
//hardware attached
//H-Bridge Steppermotor Dual Dc Motor Driver Controller Board HG7881 2.5-12V (no PWM)
//Small DC index motor from a scanner

//problems with original sketch:
//compiled oké but crashed immediately
//PWM removed -> not on HG7881
//added ICACHE_RAM_ATTR in declaration of the interupthandles (line 51)
//line 133 analogWrite(LED, rotation_value); removed, analogWrite ESP8266 in combination with interupts causes crash?
 
  #include <ESP8266WiFi.h>
  #include <ESPAsyncWebServer.h>
  #include "FS.h"

//#include <Arduino.h>              //not essential
//#include <Hash.h>                 //not essential
//#include <ESPAsyncTCP.h>          //not essential
  
// Replace with your network credentials
const char* ssid = "ABCDEFG";
const char* password = "123456";

#define LED 16

int rotation_value; //value of rotation from local web
//-----------------------position control
const byte pin_a = D1;   //for encoder pulse A
const byte pin_b = D2;   //for encoder pulse B
const byte pin_fwd = D5; //for H-bridge: run motor forward
const byte pin_bwd = D6; //for H-bridge: run motor backward
//const byte pin_pwm = 14; //for H-bridge: motor speed //no p

int encoder_r = 0;
int encoder_f = 0;
int position_pv = 0;
int position_sv = 0;
int m_direction = 0;

unsigned long prevMillis1;

void ICACHE_RAM_ATTR detect_a_r(); //essential for ESP
void ICACHE_RAM_ATTR detect_a_f(); //essential for ESP


// Create AsyncWebServer object on port 80
AsyncWebServer server(80);


const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <script src="jquery.min.js"></script>
  <script src="knob.js"></script>
  <meta name="viewport" content="width=device-width, initial-scale=1">
</head>

<body>
  <center>
  <h2>Motor position control</h2>
  <input type="text" class="dial" value="1" data-displayPrevious=true data-fgColor="#00A8A9" data-thickness=.6>
  </center>

  <script>
    $(".dial").knob({
        'change' : function (v) { console.log(Math.round(v,0));    
    $.ajax({url: "/setLOCATION?rotation="+Math.round(v,0), success: function(result){
            $("#div1").html(result);
        }});
  }
    });
  </script>

</body>
</html>)rawliteral";



void setup(){
  //---------declaration for position control
  pinMode(pin_a,INPUT_PULLUP);
  pinMode(pin_b,INPUT_PULLUP);
  pinMode(pin_fwd,OUTPUT);
  pinMode(pin_bwd,OUTPUT);
  //pinMode(pin_pwm,OUTPUT);


  digitalWrite(pin_fwd,0);  //stop motor
  digitalWrite(pin_bwd,0);  //stop motor

  attachInterrupt(digitalPinToInterrupt(pin_a), detect_a_r, RISING);
  
  // Serial port for debugging purposes
  Serial.begin(115200);
  
  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
  if(!SPIFFS.begin()){
        Serial.println("An Error has occurred while mounting SPIFFS");
        return;
  }

  // Print ESP32 Local IP Address
  Serial.println(WiFi.localIP());

  // Route for root / web page

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html);
  });
  server.on("/jquery.min.js", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/jquery.min.js", "text/javascript");
  });
  server.on("/knob.js", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/knob.js", "text/javascript");
  });
  server.on("/setLOCATION",HTTP_GET, [](AsyncWebServerRequest *request){
    String ROTATION = request->getParam("rotation")->value();
    rotation_value = 1024 - (ROTATION.toInt())*10;
    
    //analogWrite(LED, rotation_value);  //adjust LED light on NodeMCU
    request->send(200, "text/plane","");
    
  });
  // Start server
  server.begin();
}
 
void loop(){
  //
 
  //analogWrite(pin_pwm,650); //set motor speed = 650/1024
  position_sv = (1024 - rotation_value)/10; //10 unit: % (e.g. 0% ~ 0dg; 100% ~ 360dg)
  position_pv = (encoder_r + encoder_f)/2; //2 unit: %; encoder pulse per revolution: 200ppr

/*
  if (millis() - prevMillis1 > 1000){ //loop every 1000 ms
    prevMillis1 = millis();
    Serial.print("position_sv ");
    Serial.println(position_sv);
    Serial.print("position_pv ");
    Serial.println(position_pv);
    Serial.print("encoder_r ");
    Serial.println(encoder_r);        
    Serial.print("rotation_value ");
    Serial.println(rotation_value);
  }
*/

  if(position_pv-position_sv < 0){
      digitalWrite(pin_fwd,1);
      digitalWrite(pin_bwd,0);
    }
    else{
      if(position_pv-position_sv > 0){
        digitalWrite(pin_fwd,0);
        digitalWrite(pin_bwd,1);
      }
      else{
        //stop motor if position between +0 ~ -0
        digitalWrite(pin_fwd,0);  //stop motor
        digitalWrite(pin_bwd,0);  //stop motor
      }
    }
}

void detect_a_r() {
  m_direction = digitalRead(pin_b); //read direction of motor
  if(!m_direction){
    encoder_r += 1;   //increasing encoder at forward run
  }
  else{
    encoder_r += -1;  //decreasing encoder at backward run
  }
  attachInterrupt(digitalPinToInterrupt(pin_a), detect_a_f, FALLING); //change interrupt to Falling edge
}
void detect_a_f() {
  m_direction = digitalRead(pin_b); //read direction of motor
  if(m_direction){
    encoder_f += 1; //increasing encoder at forward run
  }
  else{
    encoder_f += -1; //decreasing encoder at backward run
  }
  attachInterrupt(digitalPinToInterrupt(pin_a), detect_a_r, RISING);  //change interrupt to Rising edge
}
