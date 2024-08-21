#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <Update.h>
#include <esp_adc_cal.h>
#include "DHT.h"
#include <PubSubClient.h>
// #include "co2FromAdc.h"
#include "co2FromPwm.h"

const char *host = "esp32";
const char *ssid = "Loading..."; //"JRB_Laptop";
const char *password = "driekeerraden123";
WebServer server(80);

// Add your MQTT Broker IP address, example:
// const char* mqtt_server = "192.168.1.144";
const char *mqtt_server = "jrbubuntu.ddns.net";

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char mqttMessage[256]; //128
int value = 0;
const char *mqttUser = "jochem";
const char *mqttPass = "Wachtwoord";

// Voor Relais en LED strip
uint8_t r, g, b;
String c, c2;

// Voor LED
uint8_t ledR = 33;
uint8_t ledCW = 32;
uint8_t ledWW = 13; // 35
uint8_t ledW = 25;
uint8_t ledR2 = 14;
uint8_t ledG2 = 27;
uint8_t ledB2 = 26;

// Digital LED strip
#include <FastLED.h>
#include <Arduino.h>
#define NUM_LEDS 50 // 300 // 30 LEDs per meter, 10 meters    +30 White LEDs per meter
#define DATA_PIN 23
#define CLOCK_PIN 13
CRGBArray<NUM_LEDS> leds;
// CRGB leds[NUM_LEDS];

bool waving, dot, dots, dots2, cal, pingpong, colortemp, colortemp2 = false;
bool rainbow, thunder, sinelon, bpm, juggle, confetti = false;
volatile int waveSpeed = 33;
volatile int delaySpeed = 500;
volatile int dotSpeed = 1;
volatile int dotsSpeed = 1;
volatile int dots2Speed = 1;
volatile int pingpongSpeed = 50;
volatile int pingpongFade = 1;
volatile int coldWhite = 0;
volatile int warmWhite = 0;
volatile int ThunderSpeed = 80;
volatile uint8_t BeatsPerMinute = 62;
volatile int ThunderFlickerSpeedValue = 80;
#define BRIGHTNESS 255       // 96
#define FRAMES_PER_SECOND 60 // 120
uint8_t gHue = 0;            // rotating "base color" used by many of the patterns

/*          ColorTemperature          */
#define TEMPERATURE_1 Tungsten100W
#define TEMPERATURE_2 OvercastSky
#define DISPLAYTIME 20 // How many seconds to show each temperature before switching
#define BLACKTIME 3    // How many seconds to show black between switches

// Voor Relais
#define GPIO23 23
#define GPIO19 19
#define GPIO18 18

// Set initial fading speed
volatile int fadeSpeed, fadeSpeed2; // Default fading speed
bool isFading, isFading2 = false;

/*        Sensor Setup        */
#include <DHT.h>
#define DHT_SENSOR_PIN 17 // GPIO17
#define DHT_SENSOR_TYPE DHT22
DHT dht_sensor(DHT_SENSOR_PIN, DHT_SENSOR_TYPE);

float temperature, temperature1, temperature2, temperature3, temperature4, humidity, humidity1, humidity2, humidity3, humidity4 = 0.00;
float ppmCO2 = 0.00;

// #define GPIO34 34 // ADC MHZ14
#define PWM_CO2 4 // PWM MHZ14
Co2FromPwm co2FromPwm(PWM_CO2);

const char *loginIndex = R"html(

<!DOCTYPE html>
  <html lang="en">
    <head>
      <meta name='viewport' content='width=device-width, initial-scale=1'>
      <link rel='icon' href='data:,'>
      <link rel='stylesheet' href='https://stackpath.bootstrapcdn.com/bootstrap/4.3.1/css/bootstrap.min.css'>
      <script src='https://cdnjs.cloudflare.com/ajax/libs/jscolor/2.0.4/jscolor.min.js'></script>
    <style>
      .graph-container {
      display: inline-block; /* Display the graph containers inline-block */
      width: 48%; /* Adjust width to leave space for margins */
      margin: 1%; /* Add some margin between graph containers */
      vertical-align: top; /* Align the graph containers to the top */
      }
      .graph {
          width: 100%; /* Make sure the graph takes up the full width of its container */
      }
      .graph-row {
          display: flex; /* Use flexbox for layout */
      }
      .graph-column {
          flex: 1; /* Each column takes up equal space */
          margin: 10px; /* Add some margin between columns */
      }
      #grafanaEmbedContainer1 {
          width: 100%; /* Ensure graphs take up full width */
      }

      .dropbtn {
          background-color: #04AA6D;
          color: white;
          padding: 16px;
          font-size: 16px;
          border: none;
      }
      .dropdown {
          position: relative;
          display: inline-block;
      }
      .dropdown-content {
          display: none;
          position: absolute;
          background-color: #f1f1f1;
          min-width: 160px;
          box-shadow: 0px 8px 16px 0px rgba(0, 0, 0, 0.2);
          z-index: 1;
      }
      .dropdown-content a {
          color: black;
          padding: 12px 16px;
          text-decoration: none;
          display: block;
      }
      .dropdown-content a:hover {
          background-color: #ddd;
      }
      .dropdown:hover .dropdown-content {
          display: block;
      }
      .dropdown:hover .dropbtn {
          background-color: #3e8e41;
      }      
      .button { 
        background-color: #4CAF50; 
        border: none; 
        color: white; 
        padding: 16px 40px;
        text-decoration: none; 
        font-size: 30px; 
        margin: 2px; 
        cursor: pointer;
      }
        
		#temperatureSlider {
        width: 80%;
        margin: 20px auto;
    	}
    </style>
    <script>
    var ipAddress;
    var xhr = new XMLHttpRequest();
    xhr.onreadystatechange = function () {
        if (xhr.readyState === 4 && xhr.status === 200) {
            ipAddress = xhr.responseText;
        }
    };
    xhr.open('GET', '/getIPAddress');
    xhr.send();       
</script>
</head>
<body>
<h1>ESP Color picker</h1> 
    <div class='container'>
        <div class='row'>
        	<div class="column">   
            <div> 
        <a class='btn btn-primary btn-lg' href="javascript:window.open('http://' + ipAddress + '/serverIndex', 'yourWindowName', 'width=200,height=150');" role='button'>Update firmware</a>              
        <button onclick="Restart()">Restart ESP32</button>
        </div>
        
       <div class="dropdown">
  	   <button class="dropbtn">Relais</button>    
        <div class="dropdown-content">
            <button onclick="Xmaslights()">Christmas LED</button>
            <button onclick="BigTV()">Small TV</button>                   
            <button onclick="SmallTV()">Big TV</button>
        </div>
        </div>
        <div class="dropdown">
  		<button class="dropbtn">Digital RGB</button>      
         <div class="dropdown-content">
            <button onclick="Rainbow()">Rainbow ESP32</button>           
            <button onclick="Sinelon()">Sinelon ESP32</button>
            <button onclick="Juggle()">Juggle ESP32</button>
            <button onclick="Confetti()">Confetti ESP32</button>
		</div>
    </div>
        <div class="dropdown">
  	    <button class="dropbtn">Thunder</button>    
        <div class="dropdown-content">
            <form action="/Thunder">
                Thunder Function <input type='range' id='ThunderSpeed' name='ThunderSpeed' min='1' max='100' value='100'><span id='ThunderSpeedValue'>100</span>
                <input type="submit" value="Submit">
                </form><br>
                <form action="/ThunderFlicker">
                ThunderFlicker Function <input type='range' id='ThunderFlickerSpeed' name='ThunderFlickerSpeed' min='1' max='100' value='100'><span id='ThunderFlickerSpeedValue'>100</span>
                <input type="submit" value="Submit">
                </form><br>
                </div>
        </div>
         <div class="dropdown">
  	   <button class="dropbtn">Others</button>    
        <div class="dropdown-content">
            <form action="/Bpm">
                Bpm Function <input type='range' id='BeatsPerMinute' name='BeatsPerMinute' min='1' max='100' value='100'><span id='BeatsPerMinuteValue'>100</span>
                <input type="submit" value="Submit">
                </form><br>
                       
  <form action="/Fade2">
  Fading Speed Bedroom <input type='range' id='fadeSpeed2' name='fadeSpeed2' min='1' max='100' value='100'><span id='fadeSpeedValue2'>100</span>
  <input type="submit" value="Submit">
  </form><br>

  <form action="/FadeBoth">
  Fading Speed Synch <input type='range' id='fadeSpeed' name='fadeSpeed2' min='1' max='100' value='100'><span id='fadeSpeedValue'>100</span>
  <input type="submit" value="Submit">
  </form><br>

  <form action="/dot">
  moving dot <input type='range' id='dotSpeed' name='dotSpeed' min='1' max='1000' value='100'><span id='dotSpeedValue'>100</span>
  <input type="submit" value="Submit">
  </form><br>

    <form action="/dots">
  moving dots <input type='range' id='dotsSpeed' name='dotsSpeed' min='1' max='1000' value='100'><span id='dotsSpeedValue'>100</span>
  <input type="submit" value="Submit">
  </form><br>

    <form action="/dots2">
  moving dots2 <input type='range' id='dots2Speed' name='dots2Speed' min='1' max='1000' value='100'><span id='dots2SpeedValue'>100</span>
  <input type="submit" value="Submit">
  </form><br>

 <form action="/wave">
  Wave Speed Kitchen <input type='range' id='waveSpeed' name='waveSpeed' min='1' max='1000' value='100'><span id='waveSpeedValue'>100</span><br>
  <input type="submit" value="Submit">
  </form><br>

  <form action="/waveDuration">
  Delay wave Speed <input type='range' id='delaySpeed' name='delaySpeed' min='1' max='10000' value='100'><span id='delaySpeedValue'>100</span><br>
  <input type="submit" value="Submit">
  </form><br>

  <form action="/pingpongDuration">
  Delay movement pingpong <input type='range' id='pingpongSpeed' name='pingpongSpeed' min='1' max='1000' value='100'><span id='pingpongSpeedValue'>100</span><br>
  <input type="submit" value="Submit">
  </form><br>

  <form action="/pingpongScale">
  Delay pingpong fade <input type='range' id='pingpongFade' name='pingpongFade' min='1' max='10' value='1'><span id='pingpongFadedValue'>1</span><br>
  <input type="submit" value="Submit">
  </form><br>
</div>
</div>
	<div class="dropdown">
    <button class="dropbtn">Temperature</button>    
    <div class="dropdown-content"> 
    <input type="range" min="3000" max="6500" value="4000" id="temperatureSlider">
    <p>Color Temperature: <span id="temperatureValue">4000</span> K</p>
    <input type="range" min="0" max="100" value="100" id="brightnessSlider">
    <p>Brightness: <span id="brightnessValue">100</span></p>
    <button onclick="submitLEDValues()">Submit</button>
    <button onclick="controlBothColors()">Control Both Colors</button>
    <button onclick="turnOffLEDs()">Turn Off</button>
    </div>
  </div>
    <button onclick="stop()">stop digital LED</button>
     
  <a class="btn btn-success" href="javascript:window.open('http://' + ipAddress + '/FadeStop', 'width=200,height=150');">Stop Fade</a> <br>
  <form action="/setColor">
  <input type='color' id='colorPicker' name='colorPicker' value="#FF5733">Choose color Kichten<span id='colorPickervalue'></span>
  <input type="submit" value="Submit">
  </form>  
  <form action="/setColor2">   
  Set colour <input type='color' id='colorPicker2' name='colorPicker2' value="#FF5733">Choose color Bed<span id='colorPickervalue2'></span>
  <input type="submit" value="Submit">
  </form>
  <form action="/setColorBoth">   
  Set colour <input type='color' id='colorPicker' name='colorPicker' value="#FF5733">Choose color Both<span id='colorPickervalue'></span>
  <input type="submit" value="Submit">
  </form>    
  <br>
  <div class="graph-row">
    <div class="graph-column">
        <h2>Studio Air Quality</h2>
        <input type="datetime-local" id="startDateTimeSelector">
        <input type="datetime-local" id="endDateTimeSelector">
        <button id="updateGraphBtn1">Update Graph</button>
        <button id="autoRefresh1">Auto update</button>
        <button id="5mButton1">5Min</button>
        <button id="1mButton1">1Min</button>
        <div id="grafanaEmbedContainer1"> 
        <iframe src="https://jrbubuntu.ddns.net:3000/d-solo/cdffdw22v78xsc/studio-air-quality?orgId=1&panelId=1" style="height: 500px; width: 100%;" frameborder="0"></iframe> 
        </div>
    </div>
    </div>

  </body>
  <script>   
  document.getElementById('updateGraphBtn1').addEventListener('click', function() {
    var startDateTime = document.getElementById('startDateTimeSelector').value; // Get selected start date and time
    var endDateTime = document.getElementById('endDateTimeSelector').value; // Get selected end date and time
    
    var startTimestamp = Math.floor(new Date(startDateTime).getTime()); // Convert start date and time to Unix timestamp
    var endTimestamp = Math.floor(new Date(endDateTime).getTime()); // Convert end date and time to Unix timestamp

    var grafanaEmbedUrl = 'https://jrbubuntu.ddns.net:3000/d-solo/cdffdw22v78xsc/studio-air-quality?orgId=1&panelId=1';
    var updatedEmbedUrl = grafanaEmbedUrl + '&from=' + startTimestamp + '&to=' + endTimestamp;
    console.log("Received URL: ");
    console.log(grafanaEmbedUrl);
    console.log("Updated URL: ");
    console.log(updatedEmbedUrl);
    // Update the src attribute of the existing iframe
    document.querySelector('#grafanaEmbedContainer1 iframe').src = updatedEmbedUrl;
});
document.getElementById('5mButton1').addEventListener('click', function() {
    var grafanaEmbedUrl = 'https://jrbubuntu.ddns.net:3000/d-solo/cdffdw22v78xsc/studio-air-quality?orgId=1&panelId=1';
    var updatedEmbedUrl = grafanaEmbedUrl + '&from=now-5m&to=now';
   
    console.log("Updated URL: ");
    console.log(updatedEmbedUrl);
    // Update the src attribute of the existing iframe
    document.querySelector('#grafanaEmbedContainer1 iframe').src = updatedEmbedUrl;
});
document.getElementById('1mButton1').addEventListener('click', function() {
    var grafanaEmbedUrl = 'https://jrbubuntu.ddns.net:3000/d-solo/cdffdw22v78xsc/studio-air-quality?orgId=1&panelId=1';
    var updatedEmbedUrl = grafanaEmbedUrl + '&from=now-1m&to=now';
   
    console.log("Updated URL: ");
    console.log(updatedEmbedUrl);
    // Update the src attribute of the existing iframe
    document.querySelector('#grafanaEmbedContainer1 iframe').src = updatedEmbedUrl;
});
document.getElementById('autoRefresh1').addEventListener('click', function() {
    var iframe = document.querySelector('#grafanaEmbedContainer1 iframe');
    var currentUrl = iframe.src;

    // Check if refresh parameter already exists
    var refreshParamIndex = currentUrl.indexOf('refresh=');

    // If refresh parameter already exists, update its value to 5s
    if (refreshParamIndex !== -1) {
        var ampersandIndex = currentUrl.indexOf('&', refreshParamIndex);
        if (ampersandIndex !== -1) {
            currentUrl = currentUrl.substring(0, refreshParamIndex) + 'refresh=5s' + currentUrl.substring(ampersandIndex);
        } else {
            currentUrl = currentUrl.substring(0, refreshParamIndex) + 'refresh=5s';
        }
    } else { // If refresh parameter does not exist, add it
        currentUrl += (currentUrl.includes('?') ? '&' : '?') + 'refresh=5s';
    }

    iframe.src = currentUrl;
    console.log("Updated URL: ");
    console.log(currentUrl);
});
 function toggleFunction(functionName, isChecked, numberParam) {
    var url = 'http://' + ipAddress + '/toggle?' + functionName + '=' + (isChecked ? '1' : '0'); // Convert isChecked to '1' or '0'
    if (numberParam !== null && numberParam !== undefined) {
        url += '&numberParam=' + numberParam;
    }
    console.log("Function Name: ", functionName);
    console.log("Is Checked: ", isChecked);
    console.log("Number Param: ", numberParam);
    console.log("URL: ", url);
    var xhr = new XMLHttpRequest();
    xhr.open("GET", url, false);
    xhr.send(null);
    var serverResponse = xhr.responseText;
    var stateMessage = serverResponse.includes("ON") ? functionName + " is now ON" : functionName + " is now OFF";
    alert(stateMessage);
}

  function Rainbow() {      
      var xhReq = new XMLHttpRequest();
      xhReq.open("GET", 'http://' + ipAddress + '/Rainbow', false);
      xhReq.send(null);
      var serverResponse = xhReq.responseText;
      alert("ESP32 Rainbow started.");
    }
    function Sinelon() {      
      var xhReq = new XMLHttpRequest();
      xhReq.open("GET", 'http://' + ipAddress + '/Sinelon', false);
      xhReq.send(null);
      var serverResponse = xhReq.responseText;
      alert("ESP32 Sinelon started.");
    }
    function Thunder() {      
      var xhReq = new XMLHttpRequest();
      xhReq.open("GET", 'http://' + ipAddress + '/Thunder', false);
      xhReq.send(null);
      var serverResponse = xhReq.responseText;
      alert("ESP32 Thunder started.");
    }
    function ThunderFlicker() {      
      var xhReq = new XMLHttpRequest();
      xhReq.open("GET", 'http://' + ipAddress + '/ThunderFlicker', false);
      xhReq.send(null);
      var serverResponse = xhReq.responseText;
      alert("ESP32 ThunderFlicker started.");
    }  
    function Bpm() {      
      var xhReq = new XMLHttpRequest();
      xhReq.open("GET", 'http://' + ipAddress + '/Bpm', false);
      xhReq.send(null);
      var serverResponse = xhReq.responseText;
      alert("ESP32 Bpm started.");
    }
    function Juggle() {      
      var xhReq = new XMLHttpRequest();
      xhReq.open("GET", 'http://' + ipAddress + '/Juggle', false);
      xhReq.send(null);
      var serverResponse = xhReq.responseText;
      alert("ESP32 Juggle started.");
    }
    function Confetti() {      
      var xhReq = new XMLHttpRequest();
      xhReq.open("GET", 'http://' + ipAddress + '/Confetti', false);
      xhReq.send(null);
      var serverResponse = xhReq.responseText;
      alert("ESP32 Confetti.");
    }

  function stop() {
      var xhReq = new XMLHttpRequest();
      xhReq.open("GET", 'http://' + ipAddress + '/stop', false);
      xhReq.send(null);
      var serverResponse = xhReq.responseText;
      alert("Digital LED stopped");
  }
  function Restart() {      
      var xhReq = new XMLHttpRequest();
      xhReq.open("GET", 'http://' + ipAddress + '/restart', false);
      xhReq.send(null);
      var serverResponse = xhReq.responseText;
      alert("ESP32 restarted, reload webbrowser.");
    }
  function WhiteOn() {      
      var xhReq = new XMLHttpRequest();
      xhReq.open("GET", 'http://' + ipAddress + '/Aan', false);
      xhReq.send(null);
      var serverResponse = xhReq.responseText;
      alert("White LED toggled");
    }
     function WhiteOff() {      
      var xhReq = new XMLHttpRequest();
      xhReq.open("GET", 'http://' + ipAddress + '/Uit', false);
      xhReq.send(null);
      var serverResponse = xhReq.responseText;
      alert("White LED toggled");
    }
    function Xmaslights() {      
      var xhReq = new XMLHttpRequest();
      xhReq.open("GET", 'http://' + ipAddress + '/Relay1?', false);
      xhReq.send(null);
      var serverResponse = xhReq.responseText;
      var stateMessage = serverResponse.includes("ON") ? "Relay1 is now ON" : "Relay1 is now OFF";
      alert(stateMessage);
    }
    function BigTV() {      
      var xhReq = new XMLHttpRequest();
      xhReq.open("GET", 'http://' + ipAddress + '/Relay2?', false);
      xhReq.send(null);
      var serverResponse = xhReq.responseText;
      var stateMessage = serverResponse.includes("ON") ? "Relay2 is now ON" : "Relay2 is now OFF";
      alert(stateMessage);
    }
    function SmallTV() {      
      var xhReq = new XMLHttpRequest();
      xhReq.open("GET", 'http://' + ipAddress + '/Relay3?', false);
      xhReq.send(null);
      var serverResponse = xhReq.responseText;
      var stateMessage = serverResponse.includes("ON") ? "Relay3 is now ON" : "Relay3 is now OFF";
      alert(stateMessage);
    }    
    
    // Update the value display when temperature slider value changes
        const temperatureSlider = document.getElementById('temperatureSlider');
        const temperatureValue = document.getElementById('temperatureValue');
        temperatureSlider.addEventListener('input', function() {
            temperatureValue.textContent = temperatureSlider.value;
        });

        // Update the value display when brightness slider value changes
        const brightnessSlider = document.getElementById('brightnessSlider');
        const brightnessValue = document.getElementById('brightnessValue');
        brightnessSlider.addEventListener('input', function() {
            brightnessValue.textContent = brightnessSlider.value + '%';
        });

        // Function to submit LED values to ESP32
        function submitLEDValues() {
            const temperature = temperatureSlider.value;
            const brightness = brightnessSlider.value / 100.0;
            const warmWhite = Math.round(calculateWarmWhite(temperature) * brightness);
            const coldWhite = Math.round(calculateColdWhite(temperature) * brightness);

            // Send values to ESP32
            sendLEDValuesToESP(warmWhite, coldWhite);
        }
        function controlBothColors() {
          // Get only the brightness value
          const brightness = brightnessSlider.value / 100.0;
          
          // Use full brightness for both warm and cold white LEDs
          const maxIntensity = 255; // Assuming 255 is the max intensity value
          
          const warmWhite = Math.round(maxIntensity * brightness);
          const coldWhite = Math.round(maxIntensity * brightness);
          
          // Send calculated values to ESP32
          sendLEDValuesToESP(warmWhite, coldWhite);
      }

        // Function to turn off LEDs
        function turnOffLEDs() {
            // Send zero values to turn off LEDs
            sendLEDValuesToESP(0, 0);
        }

        // Function to send warm white and cold white values to ESP32
        function sendLEDValuesToESP(warmWhite, coldWhite) {
            var xhr = new XMLHttpRequest();
            xhr.open('GET', '/setLEDValues?warmWhite=' + warmWhite + '&coldWhite=' + coldWhite, true);
            xhr.send();
        }

        // Function to calculate warm white LED brightness based on temperature
        function calculateWarmWhite(temperature) {
            return Math.round((temperature - 3000) / (6500 - 3000) * 255);
        }

        // Function to calculate cold white LED brightness based on temperature
        function calculateColdWhite(temperature) {
            return Math.round((6500 - temperature) / (6500 - 3000) * 255);
        }

    var slider2 = document.getElementById("fadeSpeed2");
    var output2 = document.getElementById("fadeSpeedValue2");
    output2.innerHTML = slider2.value; 
    slider2.oninput = function() {    output2.innerHTML = this.value;    }

    var dotSlider = document.getElementById("dotSpeed");
    var dotOutput = document.getElementById("dotSpeedValue");
    dotOutput.innerHTML = dotSlider.value; 
    dotSlider.oninput = function() {    dotOutput.innerHTML = this.value;    }

    var dotsSlider = document.getElementById("dotsSpeed");
    var dotsOutput = document.getElementById("dotsSpeedValue");
    dotsOutput.innerHTML = dotsSlider.value; 
    dotsSlider.oninput = function() {    dotsOutput.innerHTML = this.value;    }

    var dots2Slider = document.getElementById("dots2Speed");
    var dots2Output = document.getElementById("dots2SpeedValue");
    dots2Output.innerHTML = dots2Slider.value; 
    dots2Slider.oninput = function() {    dots2Output.innerHTML = this.value;    }

  var delayWaveSlider = document.getElementById("delaySpeed");
  var delayOutput = document.getElementById("delaySpeedValue");
  delayOutput.innerHTML = delayWaveSlider.value;   
  delayWaveSlider.oninput = function() {    delayOutput.innerHTML = this.value;  } 

  var waveSlider = document.getElementById("waveSpeed");
  var waveOutput = document.getElementById("waveSpeedValue");
  waveOutput.innerHTML = waveSlider.value;
  waveSlider.oninput = function() {    waveOutput.innerHTML = this.value;  }

  var pingpongSlider = document.getElementById("pingpongSpeed");
  var pingpongOutput = document.getElementById("pingpongSpeedValue");
  pingpongOutput.innerHTML = pingpongSlider.value;
  pingpongSlider.oninput = function() {    pingpongOutput.innerHTML = this.value;  }

  var pingpongFader = document.getElementById("pingpongFade");
  var pingpongFaderOutput = document.getElementById("pingpongFadedValue");
  pingpongFaderOutput.innerHTML = pingpongFader.value;
  pingpongFader.oninput = function() {    pingpongFaderOutput.innerHTML = this.value;  }

</script>
</html>)html";

const char *serverIndex = R"html(
<!DOCTYPE html>
  <html>
    <head>
      <script src='https://ajax.googleapis.com/ajax/libs/jquery/3.2.1/jquery.min.js'></script>
    </head>
  <body>
    <form method='POST' action='#' enctype='multipart/form-data' id='upload_form'>
        <input type='file' name='update'>
        <input type='submit' value='Update'>
    </form>
    <div id='prg'>progress: 0%</div>
    <script>
        $('form').submit(function(e){
            e.preventDefault();
            var form = $('#upload_form')[0];
            var data = new FormData(form);
            $.ajax({
                url: '/update',
                type: 'POST',
                data: data,
                contentType: false,
                processData: false,
                xhr: function() {
                    var xhr = new window.XMLHttpRequest();
                    xhr.upload.addEventListener('progress', function(evt) {
                        if (evt.lengthComputable) {
                            var per = evt.loaded / evt.total;
                            $('#prg').html('progress: ' + Math.round(per*100) + '%');
                        }
                    }, false);
                    return xhr;
                },
                success: function(d, s) {
                    console.log('success!')
                },
                error: function (a, b, c) {
                }
            });
        });
    </script>
</body>
</html>)html";

void fadeall()
{
  for (int i = 0; i < NUM_LEDS / pingpongFade; i++)
  {
    leds[i].nscale8(50);
  }
} // 250

void reconnect()
{
  // Loop until we're reconnected
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP32Client", mqttUser, mqttPass))
    {
      Serial.println("connected");
      // Subscribe
      client.subscribe("esp32/output");
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void codeForTask2(void *parameter)
{
  Serial.print("Task2() running on core ");
  Serial.println(xPortGetCoreID());
  for (;;)
  {
    FastLED.delay(1000 / FRAMES_PER_SECOND); // insert a delay to keep the framerate modest

    if (rainbow == true)
    {
      fill_rainbow(leds, NUM_LEDS, gHue, 7);
      FastLED.show();
      EVERY_N_MILLISECONDS(20) { gHue++; } // slowly cycle the "base color" through the rainbow
    }
    if (thunder == true)
    {
      if (random8() < ThunderSpeed)
      {
        leds[random16(NUM_LEDS)] += CRGB::White;
      }
      FastLED.show();
      fadeToBlackBy(leds, NUM_LEDS, (ThunderSpeed / 2));
    }
    if (sinelon == true)
    {
      // a colored dot sweeping back and forth, with fading trails
      fadeToBlackBy(leds, NUM_LEDS, 20);
      int pos = beatsin16(13, 0, NUM_LEDS - 1);
      leds[pos] += CHSV(gHue, 255, 192);
      FastLED.show();
      EVERY_N_MILLISECONDS(20) { gHue++; } // slowly cycle the "base color" through the rainbow
    }
    if (bpm == true)
    {
      // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
      CRGBPalette16 palette = PartyColors_p;
      uint8_t beat = beatsin8(BeatsPerMinute, 64, 255);
      for (int i = 0; i < NUM_LEDS; i++)
      { // 9948
        leds[i] = ColorFromPalette(palette, gHue + (i * 2), beat - gHue + (i * 10));
      }
      FastLED.show();
      EVERY_N_MILLISECONDS(20) { gHue++; } // slowly cycle the "base color" through the rainbow
    }
    if (juggle == true)
    {
      // eight colored dots, weaving in and out of sync with each other
      fadeToBlackBy(leds, NUM_LEDS, 20);
      uint8_t dothue = 0;
      for (int i = 0; i < 8; i++)
      {
        leds[beatsin16(i + 7, 0, NUM_LEDS - 1)] |= CHSV(dothue, 200, 255);
        dothue += 32;
      }
      FastLED.show();
    }
    if (confetti == true)
    {
      // random colored speckles that blink in and fade smoothly
      fadeToBlackBy(leds, NUM_LEDS, 10);
      int pos = random16(NUM_LEDS);
      leds[pos] += CHSV(gHue + random8(64), 200, 255);
    }
    if (pingpong == true)
    {
      static uint8_t hue = 0;
      Serial.print("x"); // First slide the led in one direction
      for (int i = 0; i < NUM_LEDS; i++)
      {
        leds[i] = CHSV(hue++, 255, 255);
        FastLED.show();
        fadeall();
        vTaskDelay(pingpongSpeed);
      }
      Serial.print("x"); // Now go in the other direction.
      for (int i = (NUM_LEDS)-1; i >= 0; i--)
      {
        leds[i] = CHSV(hue++, 255, 255);
        FastLED.show();
        fadeall();
        vTaskDelay(pingpongSpeed);
      }
    }

    if (colortemp == true)
    {
      static uint8_t starthue = 0;
      fill_rainbow(leds + 5, NUM_LEDS - 5, --starthue, 20);

      // Choose which 'color temperature' profile to enable.
      uint8_t secs = (millis() / 1000) % (DISPLAYTIME * 2);
      if (secs < DISPLAYTIME)
      {
        FastLED.setTemperature(TEMPERATURE_1); // first temperature
        leds[0] = TEMPERATURE_1;               // show indicator pixel
      }
      else
      {
        FastLED.setTemperature(TEMPERATURE_2); // second temperature
        leds[0] = TEMPERATURE_2;               // show indicator pixel
      }

      // Black out the LEDs for a few secnds between color changes
      // to let the eyes and brains adjust
      if ((secs % DISPLAYTIME) < BLACKTIME)
      {
        memset8(leds, 0, NUM_LEDS * sizeof(CRGB));
      }

      FastLED.show();
      FastLED.delay(8);
    }

    if (colortemp2 == true)
    {
      if (cal == false)
      {
        FastLED.clear();
        vTaskDelay(50 / portTICK_PERIOD_MS);
        FastLED.clearData();
        vTaskDelay(50 / portTICK_PERIOD_MS);
        CRGB leds[NUM_LEDS];
        vTaskDelay(50 / portTICK_PERIOD_MS);
        FastLED.addLeds<WS2812B, DATA_PIN, RGB>(leds, NUM_LEDS).setCorrection(TypicalSMD5050);
        cal = true;
        vTaskDelay(50 / portTICK_PERIOD_MS);
      }
      static uint8_t starthue = 0;
      fill_rainbow(leds + 5, NUM_LEDS - 5, --starthue, 20);

      // Choose which 'color temperature' profile to enable.
      uint8_t secs = (millis() / 1000) % (DISPLAYTIME * 2);
      if (secs < DISPLAYTIME)
      {
        FastLED.setTemperature(TEMPERATURE_1); // first temperature
        leds[0] = TEMPERATURE_1;               // show indicator pixel
      }
      else
      {
        FastLED.setTemperature(TEMPERATURE_2); // second temperature
        leds[0] = TEMPERATURE_2;               // show indicator pixel
      }

      // Black out the LEDs for a few secnds between color changes
      // to let the eyes and brains adjust
      if ((secs % DISPLAYTIME) < BLACKTIME)
      {
        memset8(leds, 0, NUM_LEDS * sizeof(CRGB));
      }
      FastLED.show();
      FastLED.delay(8);
    }

    if (isFading == true)
    {
    }

    if (waving == true)
    {
      Serial.println("inside waving.");
      static uint8_t hue;
      for (int i = 0; i < NUM_LEDS / 2; i++)
      {
        leds.fadeToBlackBy(40);                                       // fade everything out
        leds[i] = CHSV(hue++, 255, 255);                              // let's set an led value
        leds(NUM_LEDS / 2, NUM_LEDS - 1) = leds(NUM_LEDS / 2 - 1, 0); // now, let's first 20 leds to the top 20 leds,
        FastLED.delay(waveSpeed);
      }
      // vTaskDelay(delaySpeed / portTICK_PERIOD_MS);
      vTaskDelay(delaySpeed);
    }

    if (dot == true)
    {
      for (int whiteLed = 0; whiteLed < NUM_LEDS; whiteLed = whiteLed + 1)
      {
        leds[whiteLed] = CRGB::White; // Turn our current led on to white, then show the leds
        FastLED.show();               // Show the leds (only one of which is set to white, from above)
        vTaskDelay(dotSpeed / portTICK_PERIOD_MS);
        leds[whiteLed] = CRGB::Black; // Turn our current led back to black for the next loop around
        vTaskDelay(dotSpeed / portTICK_PERIOD_MS);
      }
    }
    if (dots == true)
    {
      for (int whiteLed = 0; whiteLed < NUM_LEDS; whiteLed = whiteLed + 1)
      {
        leds[whiteLed] = CRGB::White;
        FastLED.show();
        leds[whiteLed + 1] = CRGB::Black;
        FastLED.show();
        leds[whiteLed + 2] = CRGB::Red;
        FastLED.show();
        leds[whiteLed + 3] = CRGB::Black;
        FastLED.show();
        leds[whiteLed + 4] = CRGB::Green;
        FastLED.show();
        leds[whiteLed + 5] = CRGB::Black;
        FastLED.show();
        leds[whiteLed + 6] = CRGB::Blue;
        FastLED.show();
        vTaskDelay(dotsSpeed);
        leds[whiteLed] = CRGB::Black;
        vTaskDelay(dotsSpeed);
      }
    }
    if (dots2 == true)
    {
      for (int whiteLed = 0; whiteLed < NUM_LEDS; whiteLed = whiteLed + 1)
      {
        leds[whiteLed] = CRGB::White;
        FastLED.show();
        leds[whiteLed + 4] = CRGB::Red;
        FastLED.show();
        leds[whiteLed + 8] = CRGB::Green;
        FastLED.show();
        leds[whiteLed + 16] = CRGB::Blue;
        FastLED.show();
        vTaskDelay(dots2Speed);
        leds[whiteLed] = CRGB::Black;
        vTaskDelay(dots2Speed);
      }
    }
    vTaskDelay(50 / portTICK_PERIOD_MS);
  }
  Serial.println("Task2() ended");
}

void codeForTask3(void *parameter)
{
  Serial.print("Task3() running on core ");
  Serial.println(xPortGetCoreID());
  ledcSetup(5, 50000, 8);
  ledcSetup(6, 50000, 8);
  ledcSetup(7, 50000, 8);
  // Voor LED. assign RGB led pins to channels
  ledcAttachPin(ledG2, 5);
  ledcAttachPin(ledB2, 6);
  ledcAttachPin(ledR2, 7);
  for (;;)
  {
    // Serial.println(fadeSpeed2);
    //  Serial.print("Code is running on Core: ");Serial.println( xPortGetCoreID());
    if (isFading2 == true)
    {
      for (int l = 1; l <= 250; l += fadeSpeed2)
      {
        ledcWrite(5, l);
        vTaskDelay(50);
      }
      for (int l = 250; l >= 0; l -= fadeSpeed2)
      {
        ledcWrite(5, l);
        vTaskDelay(50);
      }
      for (int l = 1; l <= 250; l += fadeSpeed2)
      {
        ledcWrite(6, l);
        vTaskDelay(50);
      }
      for (int l = 250; l >= 0; l -= fadeSpeed2)
      {
        ledcWrite(6, l);
        vTaskDelay(50);
      }
      for (int l = 1; l <= 250; l += fadeSpeed2)
      {
        ledcWrite(7, l);
        vTaskDelay(50);
      }
      for (int l = 250; l >= 0; l -= fadeSpeed2)
      {
        ledcWrite(7, l);
        vTaskDelay(50);
      }
    }
    vTaskDelay(50 / portTICK_PERIOD_MS);
  }
  Serial.println("Task3() ended");
}

void TaskAirQuality(void *parameter)
{
  Serial.print("TaskAirQuality() running on core ");
  Serial.println(xPortGetCoreID());
  for (;;)
  {
    Serial.println("Inside AirQuality task.  ");
    if (!client.connected())
    {
      reconnect();
    }
    vTaskDelay(500 / portTICK_PERIOD_MS);

    float temperature_avg = 0, humidity_avg = 0;
    for(int i=0;i<10;i++)
    {
      temperature_avg += dht_sensor.readTemperature();
      vTaskDelay(1250 / portTICK_PERIOD_MS);
      humidity_avg += dht_sensor.readHumidity();
      vTaskDelay(1250 / portTICK_PERIOD_MS);
    }
    temperature = temperature_avg / 10;
    humidity = humidity_avg / 10;
    ppmCO2 = 12.88; //co2FromPwm.getCO2();
    vTaskDelay(1000 / portTICK_PERIOD_MS);

    Serial.println("Temperature: " + String(temperature));
    Serial.println("Humidity: " + String(humidity));
    Serial.println("analog: " + String(ppmCO2));

    long now = millis();
    if (now - lastMsg > (1000 * 60))
    {
      lastMsg = now;
      // Convert the value to a char array
      sprintf(mqttMessage, "{\"temperature\":%.2f,\"humidity\":%.2f, \"co2\":%.2f}", temperature, humidity, ppmCO2);
      Serial.println(mqttMessage);
      bool result = client.publish("studio/airquality", mqttMessage);
      if (result) {
        Serial.println("Message published successfully");
      } else {
        Serial.println("Error publishing message");
      }
    }
    client.loop(); /* Deze verplaatsen naar onderaan taak.*/

  }
  Serial.println("TaskAirQuality() ended");
}

TaskHandle_t Task2, Task3, Task4;

void hexToRgb_digital(const String &colorString)
{
  // Remove the "#" symbol if present
  String color = colorString;
  color.remove(0, 1);

  // Convert the color to integer values
  uint32_t colorValue = strtoul(color.c_str(), NULL, 16);

  // Extract R, G, and B components
  r = (colorValue >> 16) & 0xFF;
  g = (colorValue >> 8) & 0xFF;
  b = colorValue & 0xFF;

  Serial.print("Value Red: ");
  Serial.println(r);
  Serial.print("Value Green: ");
  Serial.println(g);
  Serial.print("Value Blue: ");
  Serial.println(b);
  // ledcWrite(1, r);
  // ledcWrite(2, g);
  // ledcWrite(3, b);
  for (int i = 0; i < NUM_LEDS; i++)
  {
    leds[i].setRGB(r, g, b);
    delay(200);
  }

  // fill_solid(leds, NUM_LEDS, CRGB::Green);
  FastLED.show();
}
void hexToRgb2(const String &colorString)
{
  // Remove the "#" symbol if present
  String color = colorString;
  color.remove(0, 1);

  // Convert the color to integer values
  uint32_t colorValue = strtoul(color.c_str(), NULL, 16);

  // Extract R, G, and B components
  r = (colorValue >> 16) & 0xFF;
  g = (colorValue >> 8) & 0xFF;
  b = colorValue & 0xFF;

  Serial.print("Value Red: ");
  Serial.println(r);
  Serial.print("Value Green: ");
  Serial.println(g);
  Serial.print("Value Blue: ");
  Serial.println(b);
  ledcWrite(5, r);
  ledcWrite(6, b);
  ledcWrite(7, g);

  return;
}

void colourTemp(int warmWhite, int coldWhite)
{
  ledcWrite(2, warmWhite);
  ledcWrite(3, coldWhite);
}

// Setup function
void setup(void)
{
  Serial.begin(115200);
  co2FromPwm.init();
  dht_sensor.begin();
  vTaskDelay(1000);
  Serial.println();
  Serial.print("ESP Board MAC Address:  ");
  Serial.println(WiFi.macAddress());
  pinMode(GPIO23, OUTPUT);
  digitalWrite(GPIO23, LOW);
  pinMode(GPIO19, OUTPUT);
  digitalWrite(GPIO19, LOW);
  pinMode(GPIO18, OUTPUT);
  digitalWrite(GPIO18, LOW);
  pinMode(ledW, OUTPUT);
  digitalWrite(ledW, LOW);

  ledcSetup(1, 5000, 8); // 5 kHz PWM, 8-bit resolution
  ledcSetup(2, 5000, 8);
  ledcSetup(3, 5000, 8);
  ledcSetup(4, 5000, 8);
  ledcSetup(5, 5000, 8);
  ledcSetup(6, 5000, 8);
  ledcSetup(7, 5000, 8);

  // Voor LED. assign RGB led pins to channels
  ledcAttachPin(ledR, 1);
  ledcAttachPin(ledCW, 2);
  ledcAttachPin(ledWW, 3);
  ledcAttachPin(ledR2, 5);
  ledcAttachPin(ledG2, 6);
  ledcAttachPin(ledB2, 7);

  // To initialize or smt
  ledcWrite(1, 0);
  ledcWrite(2, 0);
  ledcWrite(3, 0);
  ledcWrite(4, 250);
  ledcWrite(5, 0);
  ledcWrite(6, 0);
  ledcWrite(7, 0);

  // Setup for digital LED strip
  // FastLED.addLeds<WS2812, DATA_PIN, RGB>(leds, NUM_LEDS); // GRB ordering is typical  //Pulses red
  FastLED.addLeds<WS2812B, DATA_PIN, RGB>(leds, NUM_LEDS).setCorrection(TypicalSMD5050);
  FastLED.setMaxRefreshRate(60); //120 FPS 
  vTaskDelay(500);
  FastLED.clear();
  FastLED.show();

  // Connect to WiFi network
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.print("ESP Board MAC Address:  ");
  Serial.println(WiFi.macAddress());
  uint32_t ipAddress = (uint32_t)WiFi.localIP();
  String ipAddressStr = String(ipAddress & 0xFF) + "." +
                        String((ipAddress >> 8) & 0xFF) + "." +
                        String((ipAddress >> 16) & 0xFF) + "." +
                        String((ipAddress >> 24) & 0xFF);
  Serial.println(ipAddressStr);

  client.setServer(mqtt_server, 1883);

  // Use ipAddressStr within a lambda function
  server.on("/getIPAddress", HTTP_GET, [ipAddressStr]()
            { server.send(200, "text/plain", ipAddressStr); });

  /*return index page which is stored in serverIndex */
  server.on("/", HTTP_GET, []()
            {
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.sendHeader("Connection", "close");    
    server.send(200, "text/html", loginIndex); });

  server.on("/Aan", HTTP_GET, []()
            {
              digitalWrite(ledW, LOW);
              Serial.println("Aan");
              server.send(200, "text/plain", "Wit licht aan"); });
  server.on("/Uit", HTTP_GET, []()
            {
              digitalWrite(ledW, HIGH);
              Serial.println("Uit");
              server.send(200, "text/plain", "Wit licht uit"); });

  server.on("/Relay1", HTTP_GET, []()
            {      
    bool currentState = !digitalRead(GPIO23);
    digitalWrite(GPIO23, currentState);
    String message = "Relay1 state: ";
    message += currentState ? "ON" : "OFF";
    Serial.println(message);
    server.send(200, "text/plain", message); 
    colortemp = true; });

  server.on("/Relay2", HTTP_GET, []()
            {
      bool currentState = !digitalRead(GPIO19);
      digitalWrite(GPIO19, currentState);
      String message = "Relay2 state: ";
      message += currentState ? "ON" : "OFF";
      Serial.println(message);
      server.send(200, "text/plain", message);
      colortemp2 = true; });

  server.on("/Relay3", HTTP_GET, []()
            {
    bool currentState = !digitalRead(GPIO18);
    digitalWrite(GPIO18, currentState);
    String message = "Relay3 state: ";
    message += currentState ? "ON" : "OFF";
    Serial.println(message);
    server.send(200, "text/plain", message); });

  server.on("/setColor", HTTP_GET, []()
            {
              if(server.hasArg("colorPicker")) {
                c = server.arg("colorPicker");
                Serial.println(c);
                hexToRgb_digital(c);           
                }   
              else
              {
                server.send(400, "text/plain", "Bad Request bij color");
              } });
  server.on("/setColor2", HTTP_GET, []()
            {
              if(server.hasArg("colorPicker2")) {
                c2 = server.arg("colorPicker2");
                Serial.println(c);
                hexToRgb2(c2);            
                }   
              else
              {
                server.send(400, "text/plain", "Bad Request bij color");
              } });
  server.on("/setColorBoth", HTTP_GET, []()
            {
              if(server.hasArg("colorPicker")) {
                c = server.arg("colorPicker");
                Serial.println(c);
                hexToRgb_digital(c);
                hexToRgb2(c);             
                }   
              else
              {
                server.send(400, "text/plain", "Bad Request bij color");
              } });
  server.on("/Fade", HTTP_GET, []()
            {
              if (server.hasArg("fadeSpeed"))
              {
                String inp1 = server.arg("fadeSpeed");
                Serial.println("fadeSpeed: " + inp1);
                fadeSpeed = inp1.toInt();
                if (fadeSpeed >= 1 && fadeSpeed <= 100)
                {
                  Serial.print("Received requestedSpeed: ");
                  Serial.println(fadeSpeed);
                  server.send(200, "text/plain", "Fading started");
                }
              }
              else
              {
                server.send(400, "text/plain", "Bad Request - Invalid fadeSpeed: " + String(fadeSpeed));
              }
              if (!isFading)
              {
                isFading = true;
              }
              // return fadeSpeed;
            });
  server.on("/Fade2", HTTP_GET, []()
            {
              if (server.hasArg("fadeSpeed2"))
              {
                String inp1 = server.arg("fadeSpeed2");
                Serial.println("fadeSpeed2: " + inp1);
                fadeSpeed2 = inp1.toInt();
                if (fadeSpeed2 >= 1 && fadeSpeed2 <= 100)
                {
                  Serial.print("Received requestedSpeed2: ");
                  Serial.println(fadeSpeed2);
                  server.send(200, "text/plain", "Fading started");
                }
              }
              else
              {
                server.send(400, "text/plain", "Bad Request - Invalid fadeSpeed: " + String(fadeSpeed));
              }
              if (!isFading2)
              {
                isFading2 = true;
              } });
  server.on("/dot", HTTP_GET, []()
            {
    if (server.hasArg("dotSpeed"))
    {
      String inp1 = server.arg("dotSpeed");
      Serial.println("dotSpeed: " + inp1);
      dotSpeed = inp1.toInt();
      if (dotSpeed >= 1 && dotSpeed <= 1000)
      {
        Serial.print("Received dotSpeed: ");
        Serial.println(dotSpeed);
        server.send(200, "text/plain", "dot started");
        dot = true;
      }
    }
    else
    {      server.send(400, "text/plain", "Bad Request - Invalid fadeSpeed: " + String(dotSpeed));
    } });
  server.on("/dots", HTTP_GET, []()
            {
    if (server.hasArg("dotsSpeed"))
    {
      String inp1 = server.arg("dotsSpeed");
      Serial.println("dotsSpeed: " + inp1);
      dotsSpeed = inp1.toInt();
      if (dotsSpeed >= 1 && dotsSpeed <= 1000)
      {
        Serial.print("Received dotsSpeed: ");
        Serial.println(dotsSpeed);
        server.send(200, "text/plain", "dots started");
        dots = true;
      }
    }
    else
    {      server.send(400, "text/plain", "Bad Request - Invalid fadeSpeed: " + String(dotsSpeed));
    } });
  server.on("/dots2", HTTP_GET, []()
            {
    if (server.hasArg("dots2Speed"))
    {
      String inp1 = server.arg("dots2Speed");
      Serial.println("dots2Speed: " + inp1);
      dots2Speed = inp1.toInt();
      if (dots2Speed >= 1 && dots2Speed <= 1000)
      {
        Serial.print("Received dots2Speed: ");
        Serial.println(dots2Speed);
        server.send(200, "text/plain", "dots2 started");
        dots2 = true;
      }
    }
    else
    {      
      server.send(400, "text/plain", "Bad Request - Invalid fadeSpeed: " + String(dots2Speed));
    } 
      });
  server.on("/waveDuration", HTTP_GET, []()
            {
      if (server.hasArg("delaySpeed")) 
      {
        String inp2 = server.arg("delaySpeed");      
        Serial.println("delaySpeed: " + inp2);        
        delaySpeed = inp2.toInt();        
        
        if (delaySpeed >= 1 && delaySpeed <= 10000)
        {
            Serial.println("Received delaySpeed: " + String(delaySpeed));         
            server.send(200, "text/plain", "Wave delay received");
            waving = true;  
        }
        else
        {
            server.send(400, "text/plain", "Bad Request - Invalid delaySpeed");
        }       
      }
      else
      {
          server.send(400, "text/plain", "Bad Request - Missing arguments");
      } });
  server.on("/wave", HTTP_GET, []()
            {
      if (server.hasArg("waveSpeed")) 
      {
        String inp1 = server.arg("waveSpeed");        
        Serial.println("waveSpeed: " + inp1);         
        waveSpeed = inp1.toInt();                 
        
        
        if (waveSpeed >= 1 && waveSpeed <= 1000)
        {
            Serial.println("Received waveSpeed: " + String(waveSpeed));  
            server.send(200, "text/plain", "Waving started");
            waving = true;  
        }
        else
        {
            server.send(400, "text/plain", "Bad Request - Invalid waveSpeed");
        }       
      }
      else
      {
          server.send(400, "text/plain", "Bad Request - Missing arguments");
      } });
  server.on("/pingpongScale", HTTP_GET, []()
            {
      if (server.hasArg("pingpongFade")) 
      {
        String inp1 = server.arg("pingpongFade");        
        Serial.println("pingpongFade: " + inp1);         
        pingpongFade = inp1.toInt();              
        if (pingpongFade >= 1 && pingpongFade <= 10)
        {
            Serial.println("Received pingpongFade: " + String(pingpongFade));  
            server.send(200, "text/plain", "Pingpong started");              
        }
        else {
            server.send(400, "text/plain", "Bad Request - Invalid pingpongFade");
        }       
      }
      else {
          server.send(400, "text/plain", "Bad Request - Missing arguments");
      } });
  server.on("/pingpongDuration", HTTP_GET, []()
            {
      if (server.hasArg("pingpongSpeed")) 
      {
        String inp1 = server.arg("pingpongSpeed");        
        Serial.println("pingpongSpeed: " + inp1);         
        pingpongSpeed = inp1.toInt();      
        
        if (pingpongSpeed >= 1 && pingpongSpeed <= 1000)
        {
            Serial.println("Received pingpongSpeed: " + String(pingpongSpeed));  
            server.send(200, "text/plain", "Pingpong started");
            pingpong = true;  
        }
        else {
            server.send(400, "text/plain", "Bad Request - Invalid pingpongSpeed");
        }       
      }
      else {
          server.send(400, "text/plain", "Bad Request - Missing arguments");
      } });
  server.on("/FadeBoth", HTTP_GET, []()
            {
              if (server.hasArg("fadeSpeed"))
              {
                String inp1 = server.arg("fadeSpeed");
                Serial.println("fadeSpeed: " + inp1);
                fadeSpeed = inp1.toInt();
                fadeSpeed2 = inp1.toInt();
                if (fadeSpeed >= 1 && fadeSpeed <= 100)
                {
                  Serial.print("Received requestedSpeed: ");
                  Serial.println(fadeSpeed);
                  server.send(200, "text/plain", "Fading started");
                }
              }
              else
              {
                server.send(400, "text/plain", "Bad Request - Invalid fadeSpeed: " + String(fadeSpeed));
              }
              if (!isFading)
              {
                isFading = true;
                isFading2 = true;
              }
              // return fadeSpeed;
       });
  server.on("/FadeStop", HTTP_GET, []()
            {
      ledcWrite(1, 0);      ledcWrite(2, 0);
      ledcWrite(3, 0);      ledcWrite(5, 0);
      ledcWrite(6, 0);      ledcWrite(7, 0);

    isFading = false;    isFading2 = false;
    waving = false;    dot = false;
    dots = false;      dots2 = false;
    pingpong=false;
    colortemp = false;
    colortemp2 = false;

  FastLED.clear();      FastLED.show();
  server.send(200, "text/plain", "Fading stopped"); 
      });
  server.on("/stop", HTTP_GET, []()
            {      
      waving= dot = dots= dots2= cal= pingpong= colortemp= colortemp2= rainbow= thunder= sinelon= bpm= juggle= confetti = false; 
      FastLED.clear();
      FastLED.show();

      server.send(200, "text/plain", "stopped digital LED"); 
      });

  server.on("/setLEDValues", HTTP_GET, []()
            {
    if (server.hasArg("warmWhite") && server.hasArg("coldWhite")) {
        String warmWhiteValue = server.arg("warmWhite");
        String coldWhiteValue = server.arg("coldWhite");
        String inp1 = server.arg("warmWhite");        Serial.println("warmWhite: " + inp1);
        String inp2 = server.arg("coldWhite");        Serial.println("coldWhite: " + inp2);

        warmWhite = inp1.toInt(); 
        coldWhite = inp2.toInt(); 

        colourTemp(warmWhite, coldWhite);
        // Process the warm white and cold white values here
        server.send(200, "text/plain", "LED values received: Warm White = " + warmWhiteValue + ", Cold White = " + coldWhiteValue);
    } else {
        server.send(400, "text/plain", "Bad Request - Missing warmWhite or coldWhite parameter");
    } });

  server.on("/Rainbow", HTTP_GET, []()
            {      
    bool currentState = rainbow;
    rainbow = !rainbow;
    String message = "rainbow state: ";
    message += currentState ? "ON" : "OFF";
    Serial.println(message);
    server.send(200, "text/plain", message); });

  server.on("/Sinelon", HTTP_GET, []()
            {      
    bool currentState = sinelon;
    sinelon = !sinelon;
    String message = "sinelon state: ";
    message += currentState ? "ON" : "OFF";
    Serial.println(message);
    server.send(200, "text/plain", message); });

  server.on("/Thunder", HTTP_GET, []()
            {
      if (server.hasArg("ThunderSpeed")) 
      {
        String inp1 = server.arg("ThunderSpeed");        
        Serial.println("ThunderSpeed: " + inp1);         
        ThunderSpeed = inp1.toInt();                 
        
        
        if (ThunderSpeed >= 1 && ThunderSpeed <= 1000)
        {
            Serial.println("Received ThunderSpeed: " + String(ThunderSpeed));  
            server.send(200, "text/plain", "Thunder started");
            thunder = !thunder;

        }
        else
        {
            server.send(400, "text/plain", "Bad Request - Invalid ThunderSpeed");
        }       
      }
      else
      {
          server.send(400, "text/plain", "Bad Request - Missing arguments");
      } });

  server.on("/ThunderFlicker", HTTP_GET, []()
            {
      if (server.hasArg("ThunderFlickerSpeedValue")) 
      {
        String inp1 = server.arg("ThunderFlickerSpeedValue");        
        Serial.println("ThunderFlickerSpeedValue: " + inp1);         
        ThunderFlickerSpeedValue = inp1.toInt();                 
        
        
        if (ThunderFlickerSpeedValue >= 1 && ThunderFlickerSpeedValue <= 1000)
        {
            Serial.println("Received ThunderFlickerSpeedValue: " + String(ThunderFlickerSpeedValue));  
            server.send(200, "text/plain", "ThunderFlicker started");
            thunder = !thunder;

        }
        else
        {
            server.send(400, "text/plain", "Bad Request - Invalid ThunderSpeed");
        }       
      }
      else
      {
          server.send(400, "text/plain", "Bad Request - Missing arguments");
      } });

  server.on("/Bpm", HTTP_GET, []()
            {
      if (server.hasArg("BeatsPerMinute")) 
      {
        String inp1 = server.arg("BeatsPerMinute");        
        Serial.println("BeatsPerMinute: " + inp1);         
        BeatsPerMinute = inp1.toInt();                 
        
        
        if (BeatsPerMinute >= 1 && BeatsPerMinute <= 1000)
        {
            Serial.println("Received BeatsPerMinute: " + String(BeatsPerMinute));  
            server.send(200, "text/plain", "Bpm started");
            bpm = !bpm;
        }
        else
        {
            server.send(400, "text/plain", "Bad Request - Invalid BeatsPerMinute");
        }       
      }
      else
      {
          server.send(400, "text/plain", "Bad Request - Missing arguments");
      } });

  server.on("/Juggle", HTTP_GET, []()
            {      
    bool currentState = juggle;
    juggle = !juggle;
    String message = "juggle state: ";
    message += currentState ? "ON" : "OFF";
    Serial.println(message);
    server.send(200, "text/plain", message); });

  server.on("/Confetti", HTTP_GET, []()
            {      
    bool currentState = confetti;
    confetti = !confetti;
    String message = "confetti state: ";
    message += currentState ? "ON" : "OFF";
    Serial.println(message);
    server.send(200, "text/plain", message); });

  server.on("/restart", HTTP_GET, []()
            {
    ESP.restart();
    server.send(200, "text/plain", "ESP32 restarted"); });

  server.on("/serverIndex", HTTP_GET, []()
            {
    server.sendHeader("Connection", "close");
    //server.sendHeader("Content-Length", String(serverIndex.length())); // Add this line
    server.send(200, "text/html", serverIndex); });

  /*handling uploading firmware file */
  server.on(
      "/update", HTTP_POST, []()
      {
    server.sendHeader("Connection", "close");
    server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
    ESP.restart(); },
      []()
      {
        HTTPUpload &upload = server.upload();
        if (upload.status == UPLOAD_FILE_START)
        {
          Serial.printf("Update: %s\n", upload.filename.c_str());
          if (!Update.begin(UPDATE_SIZE_UNKNOWN))
          { // start with max available size
            Update.printError(Serial);
          }
        }
        else if (upload.status == UPLOAD_FILE_WRITE)
        {
          /* flashing firmware to ESP*/
          if (Update.write(upload.buf, upload.currentSize) != upload.currentSize)
          {
            Update.printError(Serial);
          }
        }
        else if (upload.status == UPLOAD_FILE_END)
        {
          if (Update.end(true))
          { // true to set the size to the current progress
            Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
          }
          else
          {
            Update.printError(Serial);
          }
        }
      });

  ledcWrite(1, 0); // Rood
  ledcWrite(2, 0); // WarmWhite
  ledcWrite(3, 0); // ColdWhite
  ledcWrite(5, 0); // Groen
  ledcWrite(6, 0); // Blauw
  ledcWrite(7, 0); // Rood

  // pointer |       name |        STACK | pionter | priority |  create |  core
  xTaskCreatePinnedToCore(codeForTask2, "RGB LED", 6144, NULL, 1, &Task2, 1);
  delay(100); // needed to start-up task2
  xTaskCreatePinnedToCore(codeForTask3, "RGB LED 2", 6144, NULL, 1, &Task3, 1);
  delay(100); // needed to start-up task3
  xTaskCreatePinnedToCore(TaskAirQuality, "air quality", 4096, NULL, 2, &Task4, 0);
  delay(100); // needed to start-up task4

  server.begin();
}

void loop()
{
  server.handleClient();
}