const int pinBuzzer = 3;

const unsigned int tonos[] = {31, 261, 277, 294, 311, 330, 349, 370, 392, 415, 440, 466, 494, 65535};
const int countTonos = 12;

void setup()
{
  Serial.begin(115200);
  pinMode(pinBuzzer, OUTPUT);
  pinMode(13, OUTPUT);
}

void loop()
{
  for (int i = 2020; i < 65535; i= i + 20)
  {
    Serial.print("TONE: ");
    Serial.println(i);
    tone(pinBuzzer, 440);
    delay(100);
    Serial.println("NO TONE-HIGH");
    noTone(pinBuzzer);
    digitalWrite(pinBuzzer, HIGH);
    delay(500);
  }


  /*
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
  */
}


void fadeOut()
{
    tone(pinBuzzer, 440,200);
    delay(200);
   for(int fadeValue = 255 ; fadeValue >= 0; fadeValue -=5) {
    analogWrite(ledPin, fadeValue);  
   }
}
