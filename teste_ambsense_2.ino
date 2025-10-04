
//MODEL
#include <AmbSense.h>
#include <Componente_TesteConexao_controller.h>

TaskHandle_t tskLoop_core0;
        

Dispositivo objDispositivo;
Dispositivo_controller objDispositivo_controller = Dispositivo_controller();

unsigned long startMillisTestaConexao = 0;

void FazerLeituraSensores() {
  Componente_TesteConexao_controller objComponente_TesteConexao_controller = Componente_TesteConexao_controller();

  int intTempoTesteConexao = 60 * 1000;  // 1 min
  int intTempoMedicaoSensores = 1 * 250;  // 1seg

  Leitura objLeitura;

  if (millis() - startMillisTestaConexao >= intTempoTesteConexao) {
    
    if (objComponente_TesteConexao_controller.RealizarTeste(3)) {
      startMillisTestaConexao = millis();
      objComponente_TesteConexao_controller.ExibirResultadoTesteConexao();
      // PTM
      objLeitura = Leitura(
        String(objComponente_TesteConexao_controller.TempoMedio()), 
        "", objDispositivo_controller.IPLocal(), DataHora_utils::Agora()
        );
      //objComponente_TesteConexao_controller.AdicionarLeitura("PTM", objLeitura);

      // PPF
      objLeitura = Leitura(
        String(objComponente_TesteConexao_controller.PercentFalha()), 
        "", objDispositivo_controller.IPLocal(), DataHora_utils::Agora()
        );
      //objComponente_TesteConexao_controller.AdicionarLeitura("PPF", objLeitura);

    }
  }
/*

    std::vector<Sensor>lstSensores = objDispositivo.Sensores();
    for (int idxSensor=1; idxSensor < lstSensores.size(); idxSensor++){
      if (lstSensores[idxSensor].Codigo() == "PTM"){
      }
      else if (lstSensores[idxSensor].Codigo() == "PPF"){
          
          //arrPingPercFalha[idxLeituraSensores] = Componente_TesteConexao_controller.PercentFalha();
        
          //break;
      }
    }

  }

*/
}

void ConsultarDispositivo() {
  String strDispositivoJson = "";
  strDispositivoJson = objDispositivo_controller.ToJSON(objDispositivo);
  objDispositivo_controller.EnviarWebServerResponse(200, "application/json; charset=utf-8", strDispositivoJson);
  Serial.println("Response enviado");
}

void AlterarDispositivo() {


}

void ReiniciarDispositivo() {
  String strResponseHTTP = "Executando restart...";
  objDispositivo_controller.EnviarWebServerResponse(200, "application/json; charset=utf-8", strResponseHTTP);
  delay(1000);
  ESP.restart();
}



void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println(F("Criando dispositivo"));
  objDispositivo = Dispositivo("AmbSense Teste AmbSense 2", "TSTAMB2");
  Serial.println(F("Criando lista de sensores"));
  std::vector<Sensor> lstSensores = {
    Sensor("PPF", "Percentual de Falha"),
    Sensor("PTM", "Tempo Médio")
  };
  Serial.println(F("Adicionando sensores"));
  objDispositivo.Sensores(lstSensores);

  Serial.println(F("Adicionando ref controller"));
  objDispositivo_controller.RefDispositivo_controller(&objDispositivo_controller);
  
  Serial.println(F("Adicionando ref dispositivo"));
  objDispositivo_controller.RefDispositivo(&objDispositivo);
  
  Serial.println(F("Iniocializando dispositivo"));
  objDispositivo = objDispositivo_controller.Inicializar();
  
  Serial.println(F("Criando request handlers"));

  objDispositivo_controller.CriarWebServerRequestHandler("/consultar", HTTP_GET, ConsultarDispositivo);
  objDispositivo_controller.CriarWebServerRequestHandler("/alterar", HTTP_POST, AlterarDispositivo);
  objDispositivo_controller.CriarWebServerRequestHandler("/reiniciar", HTTP_GET, ReiniciarDispositivo);
  
  Serial.println(F("Iniciando webserver"));
  objDispositivo_controller.IniciarWebServer();
  
  // Cria o tratamento do web server no core 0
  Serial.println(F("Criando loop 0"));
  xTaskCreatePinnedToCore(Loop_core0,"Loop_core0",10000,NULL,1,&tskLoop_core0,0);

Serial.println(F("FIM SETUP"));
}

void Loop_core0(void* pvParameters){
  while (true) {
    Serial.println(F("Loop 0"));
    objDispositivo_controller.ProcessarWebServerRequest();
    vTaskDelay(100);
  }
}


void loop() {
  Serial.println(F("Loop 1"));
  // put your main code here, to run repeatedly:
  FazerLeituraSensores();
  delay(500);

}
