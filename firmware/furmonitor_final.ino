#include <SparkFun_Bio_Sensor_Hub_Library.h>
#include <Wire.h>
// Reset pin, MFIO pin
int resPin = 4;
int mfioPin = 5;
// Possible widths: 69, 118, 215, 411us
int width = 411;
// Possible samples: 50, 100, 200, 400, 800, 1000, 1600, 3200 samples/second
int samples = 400;
int pulseWidthVal;
int sampleVal;
// Takes address, reset pin, and MFIO pin.
SparkFun_Bio_Sensor_Hub bioHub(resPin, mfioPin);
bioData body;
const byte RATE_SIZE = 4;  // Increased buffer size for smoother averaging
byte rates[RATE_SIZE];
byte rateSpot = 0;
long lastBeat = 0;
float beatsPerMinute;
float beatAvg;  // Changed to float for precision
float smoothedIR = 0;
const long BEAT_THRESHOLD = 50000;
// Function to calculate AC & DC components using a moving average filter
void computeACDC(uint32_t rawValue, float &AC, float &DC, float &prevDC) {
    float alpha = 0.65;  // Smoothing factor (adjust for stability)
    DC = (alpha * prevDC) + ((1 - alpha) * rawValue);  // Low-pass filter for DC
    AC = rawValue - DC;  // AC component (pulsatile part)
    prevDC = DC;
}
// Function to compute SpO2 using the Ratio of Ratios method
float computeSpO2(float AC_red, float DC_red, float AC_ir, float DC_ir) {
    float R = (AC_red / DC_red) / (AC_ir / DC_ir);
    return (-45.06 * (R * R)) + (30.35 * R) + 94.85;  // Empirical SpO2 equation
}
float prevDC_red = 0, prevDC_ir = 0;
void setup() {
    Serial.begin(115200);
    Wire.begin();
    int result = bioHub.begin();
    if (result == 0)  // Zero errors!
        Serial.println("Sensor started!");
    Serial.println("Configuring Sensor....");
    int error = bioHub.configSensorBpm(MODE_ONE);
    if (error == 0) {
        Serial.println("Sensor configured.");
    } else {
        Serial.println("Error configuring sensor.");
        Serial.print("Error: ");
        Serial.println(error);
    }
    // Set pulse width.
    error = bioHub.setPulseWidth(width);
    if (error == 0) {
        Serial.println("Pulse Width Set.");
    } else {
        Serial.println("Could not set Pulse Width.");
        Serial.print("Error: ");
        Serial.println(error);
    }
    // Check that the pulse width was set.
    pulseWidthVal = bioHub.readPulseWidth();
    Serial.print("Pulse Width: ");
    Serial.println(pulseWidthVal);
    // Set sample rate per second.
    error = bioHub.setSampleRate(samples);
    if (error == 0) {
        Serial.println("Sample Rate Set.");
    } else {
        Serial.println("Could not set Sample Rate!");
        Serial.print("Error: ");
        Serial.println(error);
    }
    // Check sample rate.
    sampleVal = bioHub.readSampleRate();
    Serial.print("Sample rate is set to: ");
    Serial.println(sampleVal);

    Serial.println("Loading up the buffer with data....");
    delay(4000);
}
void loop() {
    body = bioHub.readSensorBpm();
    float beta = 0.7;
    smoothedIR = beta * smoothedIR + (1 - beta) * body.irLed;
    if (smoothedIR > BEAT_THRESHOLD && (millis() - lastBeat) > 600) {
        long delta = millis() - lastBeat;
        lastBeat = millis();
        beatsPerMinute = 60.0 / (delta / 1000.0);
        if (beatsPerMinute > 40 && beatsPerMinute < 180) {
            // Add new BPM reading to buffer
            rates[rateSpot++] = (byte)beatsPerMinute;
            rateSpot %= RATE_SIZE;  // Circular buffer to replace oldest value
            // Dynamically adjust beatAvg with decay
            beatAvg = 0;
            for (byte i = 0; i < RATE_SIZE; i++) {
                beatAvg += rates[i];
            }
            beatAvg /= RATE_SIZE;
        }
    }
    // **Dynamically Reset if Finger Removed**
    if (body.irLed < 10000 && body.redLed < 10000) {
        Serial.println("Finger removed! Resetting beatAvg.");
        beatAvg = 0;
        rateSpot = 0;
        memset(rates, 0, sizeof(rates));  // Fully clear buffer
    }
    // Compute AC/DC before modifying LED values
    float AC_red = 0, DC_red = 0, AC_ir = 0, DC_ir = 0;
    computeACDC(body.redLed, AC_red, DC_red, prevDC_red);
    computeACDC(body.irLed, AC_ir, DC_ir, prevDC_ir);
    // Adjust red LED value
    for (int i = 230000; i >= 0; i -= 10000) {
        if (body.redLed >= (i - 10000) && body.redLed <= i) {
            body.redLed = i - 5000;
            break;
        }
    }
    // Adjust IR LED value
    for (int i = 240000; i >= 0; i -= 10000) {
        if (body.irLed >= (i - 10000) && body.irLed <= i) {
            body.irLed = i - 5000;
            break;
        }
    }
    // Compute SpO2
    float spo2 = computeSpO2(AC_red, DC_red, AC_ir, DC_ir);
    Serial.print("Infrared LED counts: ");
    Serial.println(body.irLed);
    Serial.print("Red LED counts: ");
    Serial.println(body.redLed);
    Serial.print("Heartrate using library: ");
    Serial.println(body.heartRate);
    Serial.print("Heartrate using algorithm: ");
    Serial.println(beatAvg);
    Serial.print("Blood Oxygen using library: ");
    Serial.println(body.oxygen);
    Serial.print("Blood oxygen using algorithm: ");
    Serial.println(spo2);
    Serial.print("Confidence: ");
    Serial.println(body.confidence);
    Serial.print("Status: ");
    Serial.println(body.status);
    delay(250);
}
