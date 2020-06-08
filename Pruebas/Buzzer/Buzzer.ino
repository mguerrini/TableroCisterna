const int pinBuzzer = A3;

const unsigned int tonos[] = {31,261, 277, 294, 311, 330, 349, 370, 392, 415, 440, 466, 494, 65535};
const int countTonos = 12;

void setup()
{
  Serial.begin(115200);
  pinMode(pinBuzzer, OUTPUT);
  pinMode(13, OUTPUT);
}

void loop()
{
  digitalWrite(pinBuzzer, HIGH);

  for (int iTono = 0; iTono < countTonos; iTono++)
  {
    digitalWrite(13, HIGH);
    tone(pinBuzzer, tonos[iTono]);
    Serial.print(tonos[iTono]);
    Serial.println();
    delay(1000);

    digitalWrite(13, LOW);
    delay(1000);
//    noTone(pinBuzzer);
//    Serial.println("no tone");
//    delay(1000);
  }

  //noTone(pinBuzzer);
}
