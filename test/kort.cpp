#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <Update.h>

const char *host = "esp32";
const char *ssid = "Loading..."; //"JRB_Laptop";
const char *password = "driekeerraden123";
WebServer server(80);

// Voor Relais en LED strip
uint8_t r, g, b;
String c, c2;

// Digital LED strip
#include <FastLED.h>
#include <Arduino.h>
#define NUM_LEDS 300 // 30 LEDs per meter, 10 meters    +30 White LEDs per meter
#define DATA_PIN 23
#define CLOCK_PIN 13
//CRGBArray<NUM_LEDS> leds;
CRGB leds[NUM_LEDS];

bool waving, dot, dots, dots2 = false;
volatile int waveSpeed = 50;
volatile int delaySpeed = 500;
volatile int dotSpeed = 1;
volatile int dotsSpeed = 1;
volatile int dots2Speed = 1;


// Set initial fading speed
volatile int fadeSpeed, fadeSpeed2; // Default fading speed
bool isFading, isFading2 = false;


void codeForTask2(void *parameter)
    {
    Serial.print("Task2() running on core ");
    Serial.println(xPortGetCoreID());
    for (;;)
        {   
        /*      
        if(waving==true)
            { 
                Serial.println("inside waving.");
                static uint8_t hue;      
                for (int i = 0; i < NUM_LEDS / 2; i++)
                {                
                leds.fadeToBlackBy(40); // fade everything out    
                leds[i] = CHSV(hue++, 255, 255); // let's set an led value        
                leds(NUM_LEDS / 2, NUM_LEDS - 1) = leds(NUM_LEDS / 2 - 1, 0); // now, let's first 20 leds to the top 20 leds,
                //vTaskDelay(waveSpeed / portTICK_PERIOD_MS);
                vTaskDelay(waveSpeed);
                }
                //vTaskDelay(delaySpeed / portTICK_PERIOD_MS);
                vTaskDelay(delaySpeed);     
            }
        */

        if(dot==true){      
            for(int whiteLed = 0; whiteLed < NUM_LEDS; whiteLed = whiteLed + 1) {        
                leds[whiteLed] = CRGB::White; // Turn our current led on to white, then show the leds         
                FastLED.show(); // Show the leds (only one of which is set to white, from above)        
                delay(100);        
                leds[whiteLed] = CRGB::Black; // Turn our current led back to black for the next loop around
                vTaskDelay(dotSpeed / portTICK_PERIOD_MS);
            }
        }

        if(dots==true){      
            for(int whiteLed = 0; whiteLed < NUM_LEDS; whiteLed = whiteLed + 1) {        
                leds[whiteLed] = CRGB::White;      FastLED.show();
                leds[whiteLed+2] = CRGB::Red;      FastLED.show();    
                leds[whiteLed+4] = CRGB::Green;    FastLED.show();     
                leds[whiteLed+6] = CRGB::Blue;     FastLED.show();        
                delay(100);        
                leds[whiteLed] = CRGB::Black; 
                vTaskDelay(dotsSpeed / portTICK_PERIOD_MS);
            }
        }
        if(dots2==true){      
            for(int whiteLed = 0; whiteLed < NUM_LEDS; whiteLed = whiteLed + 1) {        
                leds[whiteLed] = CRGB::White;       FastLED.show();
                leds[whiteLed+2] = CRGB::Red;       FastLED.show();    
                leds[whiteLed+4] = CRGB::Green;     FastLED.show();     
                leds[whiteLed+6] = CRGB::Blue;      FastLED.show();       
                delay(100);        
                leds[whiteLed] = CRGB::Black; 
                vTaskDelay(dots2Speed / portTICK_PERIOD_MS);
            }
        }
        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
}

TaskHandle_t Task2, Task3, Task4;

void hexToRgb(const String &colorString)
    {   
    String color = colorString;  color.remove(0, 1);             // Remove the "#" symbol if present     
    uint32_t colorValue = strtoul(color.c_str(), NULL, 16);      // Convert the color to integer values

    // Extract R, G, and B components
    r = (colorValue >> 16) & 0xFF;    g = (colorValue >> 8) & 0xFF;    b = colorValue & 0xFF;

    Serial.print("Value Red: ");     Serial.println(r);
    Serial.print("Value Green: ");   Serial.println(g);
    Serial.print("Value Blue: ");    Serial.println(b);
    for (int i = 0; i < NUM_LEDS; i++)
    {
        leds[i].setRGB(r, g, b);
        delay(50);
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
}

// Setup function
void setup(void)
{
Serial.begin(115200);
delay(10);

Serial.println();
Serial.print("ESP Board MAC Address:  ");
Serial.println(WiFi.macAddress());

// Setup for digital LED strip
//FastLED.addLeds<WS2812, DATA_PIN, RGB>(leds, NUM_LEDS); // GRB ordering is typical  //Pulses red
FastLED.addLeds<WS2812B, DATA_PIN, RGB>(leds, NUM_LEDS);  // GRB ordering is typical //Pulses red

// Connect to WiFi network
Serial.print("Connecting to ");
Serial.println(ssid);
WiFi.begin(ssid, password);
Serial.println("");

server.on("/setColor", HTTP_GET, []()
        {
            if(server.hasArg("colorPicker")) {
            c = server.arg("colorPicker");
            Serial.println(c);
            hexToRgb(c);
            //hexToRgb_digital(c);            
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
            }
            // return fadeSpeed2
        });
server.on("/dot", HTTP_GET, []()
{
if (server.hasArg("dotSpeed"))
{
    String inp1 = server.arg("dotSpeed");
    Serial.println("dotSpeed: " + inp1);
    dotSpeed = inp1.toInt();
    if (dotSpeed >= 1 && dotSpeed <= 5000)
    {
    Serial.print("Received dotSpeed: ");
    Serial.println(dotSpeed);
    server.send(200, "text/plain", "dot started");
    dot = true;
    }
}
else
{      server.send(400, "text/plain", "Bad Request - Invalid fadeSpeed: " + String(dotSpeed));
}
});

server.on("/dots", HTTP_GET, []()
{
if (server.hasArg("dotsSpeed"))
{
    String inp1 = server.arg("dotsSpeed");
    Serial.println("dotsSpeed: " + inp1);
    dotsSpeed = inp1.toInt();
    if (dotsSpeed >= 1 && dotsSpeed <= 5000)
    {
    Serial.print("Received dotsSpeed: ");
    Serial.println(dotsSpeed);
    server.send(200, "text/plain", "dots started");
    dots = true;
    }
}
else
{      server.send(400, "text/plain", "Bad Request - Invalid fadeSpeed: " + String(dotsSpeed));
}
});

server.on("/dots2", HTTP_GET, []()
{
if (server.hasArg("dots2Speed"))
{
    String inp1 = server.arg("dots2Speed");
    Serial.println("dots2Speed: " + inp1);
    dots2Speed = inp1.toInt();
    if (dots2Speed >= 1 && dots2Speed <= 5000)
    {
    Serial.print("Received dots2Speed: ");
    Serial.println(dots2Speed);
    server.send(200, "text/plain", "dots2 started");
    dots2 = true;
    }
}
else
{      server.send(400, "text/plain", "Bad Request - Invalid fadeSpeed: " + String(dots2Speed));
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
    }  
});

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
    }  
});

server.on("/FadeStop", HTTP_GET, []()
        {
isFading = false;
isFading2 = false;
    ledcWrite(1, 0);
    ledcWrite(2, 0);
    ledcWrite(3, 0);
    ledcWrite(5, 0);
    ledcWrite(6, 0);
    ledcWrite(7, 0);

waving = false;
dot = false;
server.send(200, "text/plain", "Fading stopped"); 
});

server.on("/stop", HTTP_GET, []()
{
    FastLED.clear();
    //FastLED.show();
    waving = false;
    dot = false;
    dots = false;
    dots2 = false;

    Serial.println("Status waving: " + String(waving));
    Serial.println("Status dot: " + String(dot));
    Serial.println("Status dot: " + String(dots));
    Serial.println("Status dot: " + String(dots2));

    server.send(200, "text/plain", "stopped digital LED"); 
});

xTaskCreatePinnedToCore(codeForTask2, "RGB LED", 10000, NULL, 3, &Task2, 1);
delay(100); // needed to start-up task2
/* xTaskCreatePinnedToCore(codeForTask4, "air quality", 10000, NULL, 2, &Task4, 0);
delay(100); // needed to start-up task4 */

server.begin();
}

void loop()
{
server.handleClient();
}