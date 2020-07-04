


// --- MODO ---
void SetupFase()
{
  automaticFSM.IsFaseOk = IsFaseOk();
}

void ReadFase()
{
  boolean currVal = IsFaseOk();
  
  if (currVal != automaticFSM.IsFaseOk)
  {
    if (automaticFSM.IsFaseOk)
    {
      //muestro la pantalla mormal
      ShowMainView();
      Statistics_FaseError_End();
    }
    else
    {
      //muestro la pantalla de error
      ShowErrorFaseView();
      Statistics_FaseError_Begin();
    }

    automaticFSM.IsFaseOk = currVal;
    return;
  }

  //se podria hacer que titile el display
}

void ReadFase1()
{
  
}

void ReadFase2()
{
  
}

void ReadFase3()
{
  
}
boolean IsFaseOk()
{
  static unsigned long startTime = 0;
  static boolean state;
  static boolean isPressed;

  IsButtonPressed(FASE_ERROR_PIN, state, isPressed, startTime);

  return isPressed;
}
