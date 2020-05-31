
// --- MODO ---
void ReadExecutionMode()
{
    //valido si cambio el modo
  if (IsChangeModeButtonPressed())
  {
    //si hay una bomba encendida...se apaga pero no se notifica si no se apago como un error porque puede ser que se haya prendido de manera externa.
    //paso a manual
    
  }
  else
  {
    //paso a automatico
  }
}

void SetupMode()
{
  _mode = AUTO;
}

bool IsAutomaticMode()
{
  return _mode == AUTO;
}

void ChangeToAutomaticMode()
{
  _mode = AUTO;

  UpdateDisplayToAuto();
}

void ChangeToManualMode()
{
  _mode = MANUAL;

  UpdateDisplayToManual();
}
