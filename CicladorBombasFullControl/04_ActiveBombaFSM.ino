void ChangingActiveStateMachine()
{
  switch (_changeActiveMachineState)
  {
    case CHANGE_ACTIVE_IDLE:
    //Verifico si hay un cambio => CHANGING_REQUEST
    case CHANGE_ACTIVE_CHANGING_REQUEST:
    //Si la bomba activa esta en ON => CHANGING_FROM_ON
    //Si la bomba activa esta en OFF => CHANGING_FROM_OFF
    //Sino (no se deberia dar) => CHANGING_FROM_OFF

    case CHANGE_ACTIVE_CHANGING_FROM_ON:
    //detengo la bomba => AUTO_STOPPING_BOMBA

    case CHANGE_ACTIVE_CHANGING_FROM_OFF:
    //actualizo el display y vuelvo IDLE

    case CHANGE_ACTIVE_STOPPING:
      //espero a que la bomba se detenga....o un tiempo prudencial. Luego actualizo el display
      break;
  }
}



//--- Change Active Bombas ---

//Devuelve un booleano si se produjo el cambio de bomba
byte CheckChangeActiveBomba()
{
  /*
  if (IsChangeActiveBombaButtonPressed())
  {
    if (_bombaActiva == BOMBA1)
    {
      if (IsBombaAvailable(BOMBA2))
      {
        bomba2.IsActive = true;
        bomba2.Uses = 0;
        bomba1.IsActive = false;
        UpdateActiveBombaDisplay(BOMBA2);

        return BOMBA1;
      }
      else
        Serial.println("La bomba 2 no esta disponible. No es posible cambiar.");
    }
    else
    {
      if (IsBombaAvailable(BOMBA1))
      {
        bomba2.IsActive = false;
        bomba1.IsActive = true;
        bomba1.Uses = 0;
        UpdateActiveBombaDisplay(BOMBA1);

        return BOMBA2;
      }
      else
        Serial.println("La bomba 2 no esta disponible. No es posible cambiar.");
    }
  }
*/
  return NONE;
}
