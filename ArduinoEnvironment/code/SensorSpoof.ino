const int triggerPin = 9;
const int echoPin = 10;   

volatile unsigned long lastPulseTime = 0;
volatile unsigned long pulseInterval = 0;
volatile bool timingFinished = false; 
unsigned long timingStart = 0; 

void setup() {
  pinMode(triggerPin, OUTPUT);
  pinMode(echoPin, INPUT);
  
  Serial.begin(9600);

  // Set up an interrupt to detect pulses
  attachInterrupt(digitalPinToInterrupt(echoPin), pulseDetected, CHANGE);

  timingStart = millis();
}

void loop() {
  if (!timingFinished && millis() - timingStart >= 5000) {
    timingFinished = true; 
  }

  if (timingFinished) {
    if (pulseInterval > 0) {

      float distance = (pulseInterval / 2.0) * 0.0343;

      Serial.print("Distance: ");
      Serial.print(distance);
      Serial.println(" cm");

      pulseInterval = 0;
    }


    sendPulse();  
    delay(100);
  }
}

void pulseDetected() {
  unsigned long currentTime = micros();
  static unsigned long startTime = 0;
  bool pulseState = digitalRead(echoPin);

  if (pulseState == HIGH) {

    startTime = currentTime;
  } else if (startTime > 0) {
    // Pulse ended, calculate the interval
    pulseInterval = currentTime - startTime;
    startTime = 0;
  }
}

void sendPulse() {
  digitalWrite(triggerPin, LOW);
  delayMicroseconds(2);

  digitalWrite(triggerPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(triggerPin, LOW);
}
