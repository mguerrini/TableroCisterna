
void AutomaticLoop()
{
  //valido si cambio el modo
  if (IsChangeModeButtonPressed())
  {
    //si hay una bomba encendida...se apaga pero no se notifica si no se apago como un error porque puede ser que se haya prendido de manera externa.
    ChangeToManualMode();
    return;
  }

  //ejecuto la maquina de estados de las bombas
  BombaStateMachine(&bomba1);
  BombaStateMachine(&bomba2);

  ReadCisternaSensors();
  ReadTanqueSensors();

  switch (_automaticModeState)
  {
    case AUTO_IDLE:
      //verifico los niveles
      if (IsTanqueEmpty() && IsCisternaFull())
        ChangeAutomaticState(AUTO_SELECTING_BOMBA);
      break;

    case AUTO_SELECTING_BOMBA:
      Automatic_SelectBomba();
      break;

    case AUTO_NOT_AVAILABLES_BOMBAS:
      break;

    case AUTO_WORKING:
      //detecta tanque lelno y cambia de estado a AUTO_TANQUE_FULL o CISTERNA_VACIA y pasa a IDLE

      //verifico que exista una bomba funcionando....sino, vuelvo a selecting bomba.
      break;

    case AUTO_TANQUE_FULL:
      //REQUEST OFF
      break;

    case AUTO_STOPPING_BOMBA:
      //Apago la bomba
      break;
  }
}




void ChangeAutomaticState(AutomaticModeStates state)
{
  _automaticModeState = state;
}

void Automatic_SelectBomba()
{
  //valido si existe una bomba disponible
  if (!IsBombaAvailable(BOMBA1) && !IsBombaAvailable(BOMBA2))
  {
    UpdateActiveBombaDisplay(NONE);
    ChangeAutomaticState(AUTO_NOT_AVAILABLES_BOMBAS);
    return;
  }

  //me fijo si estan las dos en estado de error
  bool updateActiveBomba = false;

  /*
    if (_bombaUses >= BOMBA_USES_MAX)
    {
      //cambio de bomba
      if (_bombaActiva == BOMBA1)
        _bombaActiva = BOMBA2;
      else
        _bombaActiva = BOMBA1;

      updateActiveBomba = true;
    }
    else
      _bombaUses++;
  */
}
