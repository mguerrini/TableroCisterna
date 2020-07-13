// ***** BOTONERA *****
//#define BTN_PRESSED_TIME 20 //20 milisegundos de boton presionado, para evitar rebote

//funciones con referencias
//boolean IsButtonPressed(int pin, boolean &state, boolean &isPressed, unsigned long &startTime);
//boolean IsButtonPressedWithTimeRange(int pin, boolean &state, boolean &isPressed, unsigned long &startTime, unsigned long minTime, unsigned long maxTime);


// ****************************************************************** //
//                              READ
// ****************************************************************** //

//Cada vez que se presiona el botón devuelve true y luego false. Una sola vez retorna TRUE, aunque siga presionado.
//Mientras se mantenga presionado isPressed es va a ser TRUE
boolean IsButtonPressed(int pin, boolean &state, boolean &isPressed, unsigned long &startTime)
{
  //Serial.println(F("IsButtonPressed - Start"));
  unsigned long currMillis = millis();

  //cambia el estado, registro el inicio del cambio
  boolean currState = digitalRead(pin);

  if (currState != state)
  {
    //Serial.println(F("IsButtonPressed - changed state"));
    state = !state;
    startTime = currMillis;
    isPressed = false; //porque recien se inicio el timer y tiene que pasar el limite
  }


  //state == LOW -> Esta presionado
  if (state == LOW && !isPressed) {
    unsigned long delta = deltaMillis(currMillis, startTime);
    boolean output = delta > BTN_PRESSED_TIME;

    //paso BTN_PRESSED_TIME presionado -> lo considero presionado
    if (output)
    {
      //registro el boton presionado
      isPressed = true;
    }

    //output determina que se presiono y se resetea.....vuelve a no presionado, sin importar que siga apretado
    return output;
  }
  else
  {
    return false;
  }
}

boolean IsButtonPressedWithTimeRange(int pin, boolean &state, boolean &isPressed, unsigned long &startTime, unsigned long minTime, unsigned long maxTime)
{
  if (minTime <= 0 && maxTime <= 0)
    return IsButtonPressed(pin, state, isPressed, startTime);

  //si tiene tiempo maximo => se verifica si se presiono cuando hay cambio de estado y la variable isPressed no aplica
  unsigned long delta1 = 0;
  boolean changeState = false;
  unsigned long currMillis = millis();

  //cambia el estado, registro el inicio del cambio
  if (digitalRead(pin) != state)
  {
    state = !state;
    delta1 = deltaMillis(currMillis, startTime);
    startTime = currMillis;

    isPressed = false;
    changeState = true;
  }


  //si maxTime > 0 => tiene cota superior...lo verfico cuando se dejo de presionar cuanto tiempo duro la presion. No puede mantener presionado.
  if (maxTime > 0)
  {
    minTime = minTime + BTN_PRESSED_TIME;
    maxTime = maxTime + BTN_PRESSED_TIME;

    if (state == HIGH && changeState) //no esta presionado
    {
      /*
            Serial.print("        - Min: ");
            Serial.print(minTime);
            Serial.print(" - Delta: ");
            Serial.print(delta1);
            Serial.print(" - Max: ");
            Serial.println(maxTime);
      */
      //valido que el tiempo de presion este en el rango
      if (minTime <= delta1 && delta1 <= maxTime )
      {
        /*
                Serial.print("Pressed - Min: ");
                Serial.print(minTime);
                Serial.print(" - Delta: ");
                Serial.print(delta1);
                Serial.print(" - Max: ");
                Serial.println(maxTime);
        */
        return true;
      }
      else
        return false;
    }
    else
    {
      //esta apretado o no cambio el estado
      return false;
    }
  }

  //solo tiene configurado minTime....es igual a antes pero con un delta mayor
  //state == LOW -> Esta presionado
  if (state == LOW && !isPressed) {
    unsigned long delta2 = deltaMillis(currMillis, startTime);

    minTime = minTime + BTN_PRESSED_TIME;
    boolean output = (delta2 > minTime);
    /*
        Serial.print("        - Min: ");
        Serial.print(minTime);
        Serial.print(" - Delta: ");
        Serial.println(delta2);
    */
    //paso BTN_PRESSED_TIME presionado -> lo considero presionado
    if (output)
    {
      //registro el boton presionado
      isPressed = true;
      /*
            Serial.print("Pressed - Min: ");
            Serial.print(minTime);
            Serial.print(" - Delta: ");
            Serial.println(delta2);
      */
    }

    //output determina que se presiono y se resetea.....vuelve a no presionado, sin importar que siga apretado
    return output;
  }
  else
  {
    return false;
  }
}
