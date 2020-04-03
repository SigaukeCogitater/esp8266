#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>


#include <Arduino.h>
#include <Hash.h>
#include <ESPAsyncTCP.h>
//#include <ESPAsyncWebServer.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <WiFiClientSecure.h>
#ifndef STASSID



//#define STASSID "UgaliSadzaFufu"
//#define STAPSK  "mhamhababa"


//#define STASSID "CS Student Lounge"
//#define STAPSK  "cscscs418"
//

#endif

// Replace with your network credentials
//const char* ssid = STASSID;
//const char* password = STAPSK;

#define DHTPIN 5     // Digital pin connected to the DHT sensor

// Uncomment the type of sensor in use:
#define DHTTYPE    DHT11     // DHT 11
//#define DHTTYPE    DHT22     // DHT 22 (AM2302)
//#define DHTTYPE    DHT21     // DHT 21 (AM2301)


//Variables
int i = 0;
int statusCode;
const char* ssid = "text";
const char* passphrase = "text";
String houseID = "text";
String st;
String content;


//Function Decalration
bool testWifi(void);
void launchWeb(void);
void setupAP(void);
void createWebServer(void);

//Establishing Local server at port 80 whenever required
ESP8266WebServer server(80);





DHT dht(DHTPIN, DHTTYPE);

// current temperature & humidity, updated in loop()
float t = 0.0;
float h = 0.0;


const char* host = "us-central1-research99046.cloudfunctions.net";
const int httpsPort = 443;

// Use web browser to view and copy
// SHA1 fingerprint of the certificate
const char fingerprint[] PROGMEM = "05 8B 82 BC FE B3 F4 3A FA 4F 77 45 D2 D0 48 E3 6E 29 BD CE";


//
// Create AsyncWebServer object on port 80
// Generally, you should use "unsigned long" for variables that hold time
// The value will quickly become too large for an int to store
unsigned long previousMillis = 0;    // will store last time DHT was updated

unsigned long previousMillisDatabase = 0; // will store last time DHT was sent to the database
const long intervalDatabase = 60000; // update database every minute

// Updates DHT readings every 10 seconds
const long interval = 10000;

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="stylesheet" href="https://use.fontawesome.com/releases/v5.7.2/css/all.css" integrity="sha384-fnmOCqbTlWIlj8LyTjo7mOUStjsKC4pOpQbqyi7RrhN7udi9RwhKkMHpvLbHG9Sr" crossorigin="anonymous">
  <style>
    html {
     font-family: Arial;
     display: inline-block;
     margin: 0px auto;
     text-align: center;
    }
    h2 { font-size: 3.0rem; }
    p { font-size: 3.0rem; }
    .units { font-size: 1.2rem; }
    .dht-labels{
      font-size: 1.5rem;
      vertical-align:middle;
      padding-bottom: 15px;
    }
  </style>
</head>
<body>
  <h2>ESP8266 DHT Server</h2>
  <p>
    <i class="fas fa-thermometer-half" style="color:#059e8a;"></i> 
    <span class="dht-labels">Temperature</span> 
    <span id="temperature">%TEMPERATURE%</span>
    <sup class="units">&deg;C</sup>
  </p>
  <p>
    <i class="fas fa-tint" style="color:#00add6;"></i> 
    <span class="dht-labels">Humidity</span>
    <span id="humidity">%HUMIDITY%</span>
    <sup class="units">%</sup>
  </p>
</body>
<script>
setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("temperature").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/temperature", true);
  xhttp.send();
}, 10000 ) ;

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("humidity").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/humidity", true);
  xhttp.send();
}, 10000 ) ;
</script>
</html>)rawliteral";

// Replaces placeholder with DHT values
String processor(const String& var) {
  //Serial.println(var);
  if (var == "TEMPERATURE") {
    return String(t);
  }
  else if (var == "HUMIDITY") {
    return String(h);
  }
  return String();
}








void setup()
{

  Serial.begin(115200); //Initialising if(DEBUG)Serial Monitor
  dht.begin(); //===========start dth
  
  Serial.println();
  Serial.println("Disconnecting previously connected WiFi");
  WiFi.disconnect();
  EEPROM.begin(512); //Initialasing EEPROM
  delay(10);
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.println();
  Serial.println();
  Serial.println("Startup");

  //---------------------------------------- Read eeprom for ssid and pass
  Serial.println("Reading EEPROM ssid");


//  for (int i = 0; i < 160; ++i) {
//          EEPROM.write(i, 0);
//        }
//
//   EEPROM.commit();
//   ESP.reset();
  String esid;
  for (int i = 0; i < 32; ++i)
  {
    esid += char(EEPROM.read(i));
  }
  Serial.println();
  Serial.print("SSID: ");
  Serial.println(esid);
  Serial.println("Reading EEPROM pass");

  String epass = "";
  for (int i = 32; i < 96; ++i)
  {
    epass += char(EEPROM.read(i));
  }
  Serial.print("PASS: ");
  Serial.println(epass);

  String ehouseId = "";
  for (int i = 96; i < 121; ++i)
  {
    ehouseId += char(EEPROM.read(i));
  }
  Serial.print("HOUSE_ID: ");
  Serial.println(ehouseId);

  houseID = ehouseId;
  Serial.println(houseID);

  WiFi.begin(esid.c_str(), epass.c_str());
  if (testWifi())
  {
    Serial.println("Succesfully Connected!!!");
    return;
  }
  else
  {
    Serial.println("Turning the HotSpot On");
    launchWeb();
    setupAP();// Setup HotSpot
  }

  Serial.println();
  Serial.println("Waiting.");
    
  while ((WiFi.status() != WL_CONNECTED))
  {
    Serial.print(".");
    delay(100);
    server.handleClient();
  }

}




void post(){

  WiFiClientSecure client;
  
  Serial.print("connecting to ");
  Serial.println(host);

  Serial.printf("Using fingerprint '%s'\n", fingerprint);
  client.setFingerprint(fingerprint);

  if (!client.connect(host, httpsPort)) {
    Serial.println("connection failed");
    return;
  }
  
  
//  String url = String("/app/datatwo/" + String(t) + "/" + String(h));

  
  Serial.println("=======XXXXX=======");
  Serial.println(houseID);
//  houseID = "TATATAISOURNEWIS";
  String url = String("/app/datathreee/" + String(t) + "/" + String(h) + "/" + String(houseID)); //+"/");
  
  Serial.println("=================");
  Serial.print("requesting URL: ");
  Serial.println(url);


  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "User-Agent: arduino/1.0\r\n" +
               "Connection: close\r\n\r\n");

  Serial.println("request sent");
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      Serial.println("headers received");
      break;
    }
  }

  String res = client.readString();
  
  Serial.println("reply was:");
  Serial.println("==========");
  Serial.println(res);
  Serial.println("==========");
  Serial.println("closing connection"); 
  
  }






void loop() {

//    for (int i = 0; i < 160; ++i) {
//          EEPROM.write(i, 0);
//        }
  
  if ((WiFi.status() == WL_CONNECTED))
  {
    unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    // save the last time you updated the DHT values
    previousMillis = currentMillis;
    // Read temperature as Celsius (the default)
    float newT = dht.readTemperature();
    // Read temperature as Fahrenheit (isFahrenheit = true)
    //float newT = dht.readTemperature(true);
    // if temperature read failed, don't change t value
    if (isnan(newT)) {
      Serial.println("Failed to read from DHT sensor!");
    }
    else {
      t = newT;
      Serial.println(t);
    }
    // Read Humidity
    float newH = dht.readHumidity();
    // if humidity read failed, don't change h value
    if (isnan(newH)) {
      Serial.println("Failed to read from DHT sensor!");
    }
    else {
      h = newH;
      Serial.println(h);
    } 
  }

  // call post method every 60 seconds
  currentMillis = millis();
  if((currentMillis - previousMillisDatabase) >= intervalDatabase){
    
    previousMillisDatabase = currentMillis; 
    post();
    }


  }
  else
  {
  }

}


//-------- Fuctions used for WiFi credentials saving and connecting to it which you do not need to change 
bool testWifi(void)
{
  int c = 0;
  Serial.println("Waiting for Wifi to connect");
  while ( c < 20 ) {
    if (WiFi.status() == WL_CONNECTED)
    {
      return true;
    }
    delay(500);
    Serial.print("*");
    c++;
  }
  Serial.println("");
  Serial.println("Connect timed out, opening AP");
  return false;
}

void launchWeb()
{
  Serial.println("");
  if (WiFi.status() == WL_CONNECTED)
    Serial.println("WiFi connected");
  Serial.print("Local IP: ");
  Serial.println(WiFi.localIP());
  Serial.print("SoftAP IP: ");
  Serial.println(WiFi.softAPIP());
  createWebServer();
  // Start the server
  server.begin();
  Serial.println("Server started");
}

void setupAP(void)
{
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  int n = WiFi.scanNetworks();
  Serial.println("scan done");
  if (n == 0)
    Serial.println("no networks found");
  else
  {
    Serial.print(n);
    Serial.println(" networks found");
    for (int i = 0; i < n; ++i)
    {
      // Print SSID and RSSI for each network found
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(WiFi.SSID(i));
      Serial.print(" (");
      Serial.print(WiFi.RSSI(i));
      Serial.print(")");
      Serial.println((WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " " : "*");
      delay(10);
    }
  }
  Serial.println("");
  st = "<ol>";
  for (int i = 0; i < n; ++i)
  {
    // Print SSID and RSSI for each network found
    st += "<li>";
    st += WiFi.SSID(i);
    st += " (";
    st += WiFi.RSSI(i);

    st += ")";
    st += (WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " " : "*";
    st += "</li>";
  }
  st += "</ol>";
  delay(100);
  WiFi.softAP("WeatherSensor", "");
  Serial.println("softap");
  launchWeb();
  Serial.println("over");
}



void createWebServer()
{
 {
    server.on("/", []() {

      IPAddress ip = WiFi.softAPIP();
      String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
      content = "<!DOCTYPE HTML>\r\n<html>Hello from ESP8266 at ";
      content += "<form action=\"/scan\" method=\"POST\"><input type=\"submit\" value=\"scan\"></form>";
      content += ipStr;
      content += "<p>";
      content += st;
      content += "</p><form method='get' action='setting'><label>SSID: </label> <input name='ssid' length=32> <br/> <label>Password: </label> <input name='pass' length=64 type='password' placeholder='password'> <br /> <label>House ID: </label> <input placeholder='House ID - No Spaces' name='houseId' length=25> <br /> <input type='submit'></form>";
      content += "</html>";
      server.send(200, "text/html", content);
    });
    server.on("/scan", []() {
      //setupAP();
      IPAddress ip = WiFi.softAPIP();
      String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);

      content = "<!DOCTYPE HTML>\r\n<html>go back";
      server.send(200, "text/html", content);
    });

    server.on("/setting", []() {
      String qsid = server.arg("ssid");
      String qpass = server.arg("pass");
      String qhouseId = server.arg("houseId");
      
      if (qsid.length() > 0 && qpass.length() > 0) {
        Serial.println("clearing eeprom");
        for (int i = 0; i < 121; ++i) {
          EEPROM.write(i, 0);
        }
        Serial.println(qsid);
        Serial.println("");
        Serial.println(qpass);
        Serial.println("");
        Serial.println(qhouseId);
        Serial.println("");

        Serial.println("writing eeprom ssid:");
        for (int i = 0; i < qsid.length(); ++i)
        {
          EEPROM.write(i, qsid[i]);
          Serial.print("Wrote: ");
          Serial.println(qsid[i]);
        }
        Serial.println("writing eeprom pass:");
        for (int i = 0; i < qpass.length(); ++i)
        {
          EEPROM.write(32 + i, qpass[i]);
          Serial.print("Wrote: ");
          Serial.println(qpass[i]);
        }
        Serial.println("writing eeprom houseId:");
        for (int i = 0; i < qhouseId.length(); ++i)
        {
          EEPROM.write(96 + i, qhouseId[i]);
          Serial.print("Wrote: ");
          Serial.println(qhouseId[i]);
        }
        EEPROM.commit();

        content = "{\"Success\":\"saved to eeprom... reset to boot into new wifi\"}";
        statusCode = 200;
        ESP.reset();
      } else {
        content = "{\"Error\":\"404 not found\"}";
        statusCode = 404;
        Serial.println("Sending 404");
      }
      server.sendHeader("Access-Control-Allow-Origin", "*");
      server.send(statusCode, "application/json", content);

    });
  } 
}


void server2(){
  
 
    // Print ESP8266 Local IP Address
  Serial.println(WiFi.localIP());
  server.on("/", []() {
  content = "<!DOCTYPE HTML><html>";
  content += "<head>";
  content += "  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">";
  content += "  <link rel=\"stylesheet\" href=\"https://use.fontawesome.com/releases/v5.7.2/css/all.css\" integrity=\"sha384-fnmOCqbTlWIlj8LyTjo7mOUStjsKC4pOpQbqyi7RrhN7udi9RwhKkMHpvLbHG9Sr\" crossorigin=\"anonymous\">";
  content += "  <style>";
  content += "    html {";
  content += "     font-family: Arial;";
  content += "     display: inline-block;";
  content += "     margin: 0px auto;";
  content += "     text-align: center;";
  content += "    }";
  content += "    h2 { font-size: 3.0rem; }";
  content += "    p { font-size: 3.0rem; }";
  content += "    .units { font-size: 1.2rem; }";
  content += "    .dht-labels{";
  content += "      font-size: 1.5rem;";
  content += "      vertical-align:middle;";
  content += "      padding-bottom: 15px;";
  content += "    }";
  content += "  </style>";
  content += "</head>";
  content += "<body>";
  content += "  <h2>ESP8266 DHT Server</h2>";
  content += "  <p>";
  content += "    <i class=\"fas fa-thermometer-half\" style=\"color:#059e8a;\"></i>"; 
  content += "    <span class=\"dht-labels\">Temperature</span> ";
  content += "    <span id=\"temperature\">%TEMPERATURE%</span>";
  content += "    <sup class=\"units\">&deg;C</sup>";
  content += "  </p>";
  content += "  <p>";
  content += "    <i class=\"fas fa-tint\" style=\"color:#00add6;\"></i> ";
  content += "    <span class=\"dht-labels\">Humidity</span>";
  content += "    <span id=\"humidity\">%HUMIDITY%</span>";
  content += "    <sup class=\"units\">%</sup>";
  content += "  </p>";
  content += "</body>";
  content += "<script>";
  content += "setInterval(function ( ) {";
  content += "  var xhttp = new XMLHttpRequest();";
  content += "  xhttp.onreadystatechange = function() {";
  content += "    if (this.readyState == 4 && this.status == 200) {";
  content += "      document.getElementById(\"temperature\").innerHTML = this.responseText;";
  content += "    }";
  content += "  };";
  content += "  xhttp.open(\"GET\", \"/temperature\", true);";
  content += "  xhttp.send();";
  content += "}, 10000 ) ;";

  content += "setInterval(function ( ) {";
  content += "  var xhttp = new XMLHttpRequest();";
  content += "  xhttp.onreadystatechange = function() {";
  content += "    if (this.readyState == 4 && this.status == 200) {";
  content += "      document.getElementById(\"humidity\").innerHTML = this.responseText;";
  content += "    }";
  content += "  };";
  content += "  xhttp.open(\"GET\", \"/humidity\", true);";
  content += "  xhttp.send();";
  content += "}, 10000 ) ;";
  content += "</script>";
  content += "</html>";
  server.send(200, "text/html", content);
    });



//
//  // Route for root / web page
//  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
//    request->send_P(200, "text/html", index_html, processor);
//  });
//  server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest * request) {
//    request->send_P(200, "text/plain", String(t).c_str());
//  });
//  server.on("/humidity", HTTP_GET, [](AsyncWebServerRequest * request) {
//    request->send_P(200, "text/plain", String(h).c_str());
//  });
  
  }
