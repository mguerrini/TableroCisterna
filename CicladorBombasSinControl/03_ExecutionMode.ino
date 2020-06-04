
// --- MODO ---
void SetupMode()
{
  _mode = AUTO;
  ReadExecutionMode();
}

void ReadExecutionMode()
{
  byte currMode = _mode;
  if (IS_CHANGE_MODE_PULSADOR)
  {
    //valido si cambio el modo
    if (IsChangeModeButtonPressed())
    {
      //si hay una bomba encendida...se apaga pero no se notifica si no se apago como un error porque puede ser que se haya prendido de manera externa.
      //paso a manual
      //pulsador...
      if (IsAutomaticMode())
        _mode = MANUAL;
      else
        _mode = AUTO;
    }
  }
  else
  {
    //valido si cambio el modo
    if (IsChangeModeButtonPressed())
      _mode = MANUAL;
    else
      _mode = AUTO;
  }

  if (currMode != _mode)
  {
    if (IsAutomaticMode())
      UpdateDisplayToAutoMode();
    else
      UpdateDisplayToManualMode();
  }
}



bool IsAutomaticMode()
{
  return _mode == AUTO;
}

void ChangeToAutomaticMode()
{
  _mode = AUTO;

  UpdateDisplayToAutoMode();
}

void ChangeToManualMode()
{
  _mode = MANUAL;

  UpdateDisplayToManualMode();
}
