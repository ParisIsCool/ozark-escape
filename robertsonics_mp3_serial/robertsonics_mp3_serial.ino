#define AUDIO_TX 17
#define AUDIO_RX 16

void setup() {
  Serial.begin(38400); 
  //player.begin(38400,SERIAL_8N1,AUDIO_TX,AUDIO_RX);
  Serial2.begin(38400, SERIAL_8N1, AUDIO_RX, AUDIO_TX);
  Serial.println("Setup Successful.");
}
void loop() {
  if (Serial.available()) { // if there is data comming
    String command = Serial.readStringUntil('\n'); // read string until newline character
    //Serial.println(command);
    Serial2.println(command);
  }
}