void BombaStateMachine(Bomba* bomba)
{
  //Detectar deshabilitacion de la bomba
  //si la alarma esta encendida la apago
  //Si esta HabilitaCambio el estado: Deshabilitada
  //si la bomba esta ON => hay que apagarla, paso a AUTO_STOPPING:
  //si sta en OFF => no hago nada

  //leo los sensores y acciones
  ReadBombaSensors(bomba);

  ReadBombaEnabledButton(bomba);

  //esta el termico saltado...pasa a error directamente si estaba habilitada
  if (!bomba->IsTermicoOk && IsBombaEnabled(bomba))
  {
    //si está encendida..la apago
    if (IsBombaOn(bomba->Number))
      //pasa a apagada...porque esta en error
      bomba->MachineState = BOMBA_TURNING_OFF;
    else
      bomba->MachineState = BOMBA_ERROR_TERMICO;
  }

  //me fijo si hay un pedido de deshabilitación...tiene prioridad...total el error tambien se va a atender.
  if (bomba->RequestDisable)
  {
    //si está encendida..la apago
    if (IsBombaError(bomba->Number))
      bomba->MachineState = BOMBA_DISABLING_FROM_ERROR;
    else if (IsBombaOn(bomba->Number))
      //pasa a apagada...porque esta en error
      bomba->MachineState = BOMBA_DISABLING_FROM_ON;
    else if (IsBombaOff(bomba->Number))
      bomba->MachineState = BOMBA_DISABLING_FROM_OFF;
  }


  switch (bomba->MachineState)
  {
    case BOMBA_OFF:
      if (bomba->RequestDisable)
      {
        bomba->MachineState = BOMBA_DISABLING_FROM_OFF;
        return;
      }

      if (bomba->RequestOn)
      {
        Bomba_RequestON(bomba);
      }
      break;

    case BOMBA_TURNING_ON:


    case BOMBA_ON:
      if (bomba->RequestOff)
      {
        bomba->Timer = millis();
        bomba->MachineState = BOMBA_TURNING_OFF;
        //se podria poner el display en apagando
      }

    case BOMBA_TURNING_OFF:
      //tengo que apagar la bomba y esperar un tiempo prudencial...para que deje de girar y para que llegue el evento del contactor
      //sino llega......alarma, contactor pegado.
      if (bomba->Timer > BOMBA_TURNING_OFF_TIME)
      {
        if (bomba->IsContactorClosed)
        {
          //ERROR TOTAL...NO ES POSIBLE USAR EL TABLERO.....(PUEDE SER TAMBIEN QUE SEA PRENDIDO POR FUERA)
          Serial.println("Bomba apagada pero contactor Encendido!");
          bomba->MachineState = BOMBA_ERROR_CONTACTOR_CERRADO;
          if (bomba->RequestDisable)
            bomba->IsEnabled = false; 
          return;
        }
        else
        {
          //limpio el request
          if (bomba->RequestDisable)
            bomba->MachineState = BOMBA_DISABLING_FROM_OFF;
          else
            bomba->MachineState = BOMBA_OFF;
        }
      }
      break;

    case BOMBA_ERROR_CONTACTOR_CERRADO:
      if (bomba->State != ERROR_CONTACTOR_CERRADO)
      {
        ClearReques(bomba);
        bomba->State = ERROR_CONTACTOR_CERRADO;
        UpdateDisplayBomba(bomba);
        return;
      }

      //se recupera cuando el contactor se abre o se deshabilita
      if (!bomba->IsContactorClosed)
      {
        
      }
      
      //chequeo para ver si se normalizo
      if (bomba->RequestDisabled)
      {
        
      }

      break;

    case BOMBA_ERROR_CONTACTOR_ABIERTO:
      //no se activo
      if (bomba->State != BOMBA_ERROR_CONTACTOR_ABIERTO)
      {
        ClearReques(bomba);
        bomba->State = BOMBA_ERROR_CONTACTOR_ABIERTO;
        UpdateDisplayBomba(bomba);
        return;
      }

     //chequeo para ver si se normalizo
      if (bomba->RequestDisabled)
      {
        
      }
      break;

    case BOMBA_ERROR_TERMICO:
      //cambio el estado de la bomba a error
      //verifico el error y cambio el estado
      if (bomba->State != BOMBA_ERROR_TERMICO)
      {
        ClearReques(bomba);
        bomba->State = BOMBA_ERROR_TERMICO;
        UpdateDisplayBomba(bomba);
        return;
      }

      if (bomba->IsTermicoOk)
      {
        //me fijo si está o no habilitada
      }

      if (bomba->RequestDisabled)
      {
        
      }
      
      break;

    case BOMBA_DISABLING_FROM_ON:
      bomba->Timer = millis(); //arranco el contador de apagado
      bomba->MachineState = BOMBA_TURNING_OFF;
      //se podria poner el display en apagando
      break;

    case BOMBA_DISABLING_FROM_OFF:
      bomba->MachineState = BOMBA_DISABLED;
      break;

    case BOMBA_DISABLING_FROM_ERROR:
      bomba->MachineState = BOMBA_DISABLED;
      //se puede poner un estado 
      break;

    case BOMBA_DISABLED:
      if (bomba->State != DISABLED)
      {
        bomba->IsEnabled = false;
        UpdateBombaDisplay(bomba);
        return;  
      }
      
      if (bomba->RequestEnable)
      {
        bomba->RequestEnable = false;
        bomba->MachineState = BOMBA_ENABLING;
        return;
      }
      break;

    case BOMBA_ENABLING:
      break;
  }
}

void PrintToSerialMachineState(Bomba* bomba, BombaMachineStates from, BombaMachineStates to)
{

}

void CleanRequest(Bomba* bomba)
{
  bomba->RequestOn = false;
  bomba->RequestOff = false;
  bomba->RequestEnable = false;
  bomba->RequestDisable = false;
}

void ReadBombaEnabledButton(Bomba* bomba)
{
  //me fijo si el boton
  bool isEnablingButtonPressed = IsBombaEnableButtonPressed(bomba->Number);
  if (isEnablingButtonPressed)
  {
    if ( (bomba->RequestDisable && bomba->IsEnabled) || (bomba->RequestEnable && !bomba->IsEnabled))
      return; //hay un evento en procesos

    if (bomba->IsEnabled)
    {
      bomba->RequestDisable = true;
      bomba->RequestEnable = false;
    }
    else
    {
      bomba->RequestDisable = false;
      bomba->RequestEnable = true;
    }
  }
}

void Bomba_RequestON(Bomba* bomba)
{
  if (!bomba->IsTermicoOk)
  {
    Serial.println("Error de térmico en pedido de encendido");
    bomba->MachineState = BOMBA_ERROR_CONTACTOR;
  }
  else
  {
    if (bomba->State == OFF)
    {
      bomba->MachineState = BOMBA_TURNING_ON;
    }
    else if (bomba->State == ON)
    {
      //no deberia llegar nunca
      bomba->MachineState = BOMBA_ON;
    }
    else
    {
      //esta en error
      if (bomba->State == ERROR_CONTACTOR)
        bomba->MachineState = BOMBA_ERROR_CONTACTOR;
      else
        bomba->MachineState = BOMBA_ERROR_TERMICO;
    }
  }

  //limpio el pedido
  bomba->RequestOn = false;
}
