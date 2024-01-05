const int RELAY = 13;

const int numConnections = 4;

const int ledPins[numConnections] = {14,27,26,4};

const int connections[numConnections][2] = {{21,19},{18,12},{17,16},{25,15}};

void setup() {
  Serial.begin(115200);

  for (int i=0; i<numConnections; i++) {
    pinMode(connections[i][0], INPUT_PULLUP);
    pinMode(connections[i][1], INPUT_PULLUP);
  }

  for (int i=0; i<numConnections; i++) {
    pinMode(ledPins[i], OUTPUT);
  }

  pinMode(RELAY, OUTPUT);

  Serial.println("Setup Complete");
}

bool check(int OUT_PIN, int IN_PIN) {
  pinMode(OUT_PIN, OUTPUT); // set one pin to output
  pinMode(IN_PIN, INPUT_PULLUP); // set one pin ready to read
  digitalWrite(OUT_PIN, LOW); // set one pin to low
  bool connected = !digitalRead(IN_PIN); // read if pins are connected
  pinMode(OUT_PIN,INPUT_PULLUP); // return to original pin mode

  return connected;
}

bool checkall() {
  bool AllCorrect = true;
  Serial.print("Correct: ");
  for (int i=0; i<numConnections; i++) {  // loops thru all pins and checks the proper connections
    int pin1 = connections[i][0];
    int pin2 = connections[i][1];
    bool state = check(pin1,pin2);
    bool state2 = check(pin2,pin1);
    if (state && state2) { // set LEDs
      digitalWrite(ledPins[i], HIGH);
      Serial.print(i);
    } else {
      digitalWrite(ledPins[i], LOW);
      AllCorrect = false;
    }
  }
  Serial.print("\n");
  return AllCorrect;
}

bool toggle = false; // learned the hard way that these maglocks like to be on for a split second
void loop() {
  bool completed = checkall();
  if (completed && !toggle) {
    toggle = true;
    Serial.println("COMPLETE.");
    digitalWrite(RELAY,HIGH);
    delay(1000);
    digitalWrite(RELAY,LOW);
  } else if (!completed && toggle ) {
    toggle = false;
    digitalWrite(RELAY,LOW);
  } else {
    digitalWrite(RELAY,LOW);
  }
  delay(200);
}
