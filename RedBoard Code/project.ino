// This #include statement was automatically added by the Particle IDE.
#include <HttpClient.h>

// This #include statement was automatically added by the Particle IDE.
#include <SparkFunMMA8452Q.h>


#include <application.h>

// for math operations
#include <cmath>

// input variables
const int BUTTON_PIN = D6;
const int TOUCH_PIN = D4;
bool buttonState;
int buttonInput;
int touchState;
unsigned long readTimer;

HttpClient http;

// Headers currently need to be set at init, useful for API keys, etc
http_header_t headers[] = {
  // {"Content-Type", "application/json"},
  // {"Accept", "application/json"},
  {"Accept", "*/*"},
  {NULL, NULL} // NOTE: Always terminate headers will NULL
};

http_request_t request;
http_response_t response;

unsigned int timeRan;
bool running;
MMA8452Q accel;
float avgAccelerations [3]; // [x,y,z]
unsigned int numOfSamples = 1;

// calculates rolling average
float calculateAvg(float avg, float newSample){ // https://stackoverflow.com/questions/12636613/how-to-calculate-moving-average-without-keeping-the-count-and-data-total
    // we may want to abs(newSample)
    avg = avg - (avg / numOfSamples);
    avg = avg + (newSample / numOfSamples);
    return avg;
}

// sends an HTTP GET request to hostname:port/path
void apiDemo(String hostname="3.19.56.20", unsigned int port=5000, String path=""){
    
    Serial.println();
    Serial.println("Application>\tStart of Loop.");
    
    // Request patha nd body can be set at runtime or at setup
    request.hostname = hostname;
    request.port = port;
    request.path = path;
    
    // The library also supports sending a body with your request:
    //request.body = "{\"key\":\"value\"}";
    
    // GET request
    http.get(request, response, headers);
    Serial.print("Application>\tResponse status: ");
    Serial.println(response.status);
    
    Serial.print("Application>\tResponse Body: ");
    Serial.println(response.body);
}

// wrapper function around apiDemo for insert
// will only work if web server is running(ask Kurtis to turn it on)
// ie 3.19.56.20:5000/insert/?velocity=velocity&time=timeRan
void uploadData(float velocity, unsigned int timeRan){
    apiDemo("3.138.112.130", 5000, "/insert/?velocity=" + String(velocity) + "&time=" + String(timeRan));
}

// unit conversion
float convertGsToSI(float value){
    return (value * 9.8);
}
float convertMillisToSeconds(float value){
    return (value / 1000);
}

void setup() {
    // init devices
    Serial.begin(9600);
    accel.begin(SCALE_2G, ODR_1);
    pinMode(BUTTON_PIN, INPUT);
    pinMode(TOUCH_PIN, INPUT);
    avgAccelerations[0] = 0.0;
    avgAccelerations[1] = 0.0;
    avgAccelerations[2] = 0.0;
    // init variables
    buttonInput = 0;
    buttonState = false;
    touchState = 0;
    running = false;
    timeRan = 0;
}

void loop() {
    // read the state of each button
    touchState = digitalRead(TOUCH_PIN);
    buttonInput = digitalRead(BUTTON_PIN);
    //Serial.printlnf("Accel.available() = %d", accel.available());
    
    if(buttonInput == HIGH/* start/stop button pressed */){ // could be implemented w/ flags instead
        // Calculate time between pressing button for start and pressing button for stop
        timeRan = millis() - timeRan; // this calculation might be finicky
        readTimer = millis();
        
        delay(1000); // handles long presses causing multiple occurences -- hot fix
        
        // set bool running = true
        running = !running; // indicates start/stop
    }
    
    if(running && millis() >= readTimer){ // user currently running & 2000ms have passed since last read
    
        accel.read(); // MUST BE CALLED to tell accelerometer to read acceleration
        // get current acceleration on each axe and average it
        avgAccelerations[0] = calculateAvg(avgAccelerations[0], accel.cx);
        avgAccelerations[1] = calculateAvg(avgAccelerations[1], accel.cy);
        avgAccelerations[2] = calculateAvg(avgAccelerations[2], accel.cz);
        
        // increment timer
        readTimer += 2000;
        
        // increment numOfSamples
        numOfSamples++;
        
    }
    else if(touchState == 1/* Touch sensor tapped */){ // run code to send HTTP request
    
        // calculate vector average
        for(unsigned int i = 0; i < 3; i++){ // square each axe
            avgAccelerations[i] = pow(avgAccelerations[i], 2);
        }
        float finalAcceleration = sqrt(avgAccelerations[0] + avgAccelerations[1] + avgAccelerations[2]); // sqrt their sum
        
        // unit conversion
        finalAcceleration = convertGsToSI(finalAcceleration); // from G's to m/s^2
        timeRan = convertMillisToSeconds(timeRan); // from millis to seconds
        
        // convert acceleration to velocity
        float finalVelocity = finalAcceleration * timeRan; // using kinematic equation, assuming v_0 = 0
        // pass it to API/HTTP ie the web server
        uploadData(finalVelocity, timeRan);
        numOfSamples = 1;
        delay(1000);
    }
}