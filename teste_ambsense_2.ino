
//MODEL
#include <AmbSense.h>
#include <Componente_TesteConexao_controller.h>

TaskHandle_t tskLoop_core0;        

Dispositivo objDispositivo;
Dispositivo_controller objDispositivo_controller = Dispositivo_controller();
Componente_TesteConexao_controller objComponente_TesteConexao_controller = Componente_TesteConexao_controller();

unsigned long startMillisTestaConexao = 60000;
unsigned long datInicioDispositivo = 0;

void CarregarPrimeiraLeitura(){

  std::vector<Sensor> lstSensores = objDispositivo.Sensores();
  Leitura objLeitura;

  objComponente_TesteConexao_controller.HostDestino("www.google.com");

  if (objComponente_TesteConexao_controller.RealizarTeste(3)) {
    for (int idxSensores=0; idxSensores < lstSensores.size(); idxSensores++){
      if(lstSensores[idxSensores].Codigo() == "PTM"){
        lstSensores[idxSensores].Leituras({Leitura(String(objComponente_TesteConexao_controller.TempoMedio()), DataHora_utils::Agora())});
      }

      if(lstSensores[idxSensores].Codigo() == "PPF"){
        lstSensores[idxSensores].Leituras({Leitura(String(objComponente_TesteConexao_controller.PercentFalha()), DataHora_utils::Agora())});
      }
    }        
    objDispositivo.Sensores(lstSensores);
  }
}

void ConsultarTempoReal(){
  Dispositivo objDispositivoAux = objDispositivo; 
  std::vector<Sensor> lstSensores = objDispositivoAux.Sensores();

  for (int idxSensores=0; idxSensores < lstSensores.size(); idxSensores++){
    if(lstSensores[idxSensores].Codigo() == "PTM"){
      lstSensores[idxSensores].Leituras({Leitura(String(objComponente_TesteConexao_controller.TempoMedio()), DataHora_utils::Agora())});
    }

    if(lstSensores[idxSensores].Codigo() == "PPF"){
      lstSensores[idxSensores].Leituras({Leitura(String(objComponente_TesteConexao_controller.PercentFalha()), DataHora_utils::Agora())});
    }
  }        
  objDispositivoAux.Sensores(lstSensores);

  String strDispositivoJson = objDispositivoAux.ToJSON(); 
  
  Serial.println(F("Vai enviar o response tamanho: "));
  Serial.println(strDispositivoJson.length());
  WebServer_utils::EnviarWebServerResponse(200, "application/json; charset=utf-8", strDispositivoJson);
  Serial.println(F("Response enviado"));

}


void FazerLeituraSensores() {
  
  unsigned int intTempoTesteConexao = 5 * 1000;  // 5 segs
  unsigned int intTempoMedicaoSensores = 1 * 250;  // 1seg

  Leitura objLeitura;
  Leitura objUltLeitura;
  if (millis() - startMillisTestaConexao >= intTempoTesteConexao) {
    Serial.println(DataHora_utils::ConverterDataEpochParaStr(datInicioDispositivo));
    
    if (objComponente_TesteConexao_controller.RealizarTeste(3)) {
      startMillisTestaConexao = millis();
      objComponente_TesteConexao_controller.ExibirResultadoTesteConexao();
      
      // PTM
      objUltLeitura = objDispositivo_controller.UltimaLeitura("PTM");
      if (Texto_utils::isFloat(objUltLeitura.Valor().c_str())){
        if (fabs(atof(objUltLeitura.Valor().c_str()) - objComponente_TesteConexao_controller.TempoMedio()) >= 3){
          objLeitura = Leitura(String(objComponente_TesteConexao_controller.TempoMedio()), DataHora_utils::Agora());
          objDispositivo_controller.AdicionarLeitura("PTM", objLeitura);
        }
      }

      // PPF
      objUltLeitura = objDispositivo_controller.UltimaLeitura("PPF");
      if (Texto_utils::isFloat(objUltLeitura.Valor().c_str())){
        if (fabs(atof(objUltLeitura.Valor().c_str()) - objComponente_TesteConexao_controller.PercentFalha()) >= 10){
          objLeitura = Leitura(String(objComponente_TesteConexao_controller.PercentFalha()), DataHora_utils::Agora());
          objDispositivo_controller.AdicionarLeitura("PPF", objLeitura);
        }
      }

      Serial.print(F("Total de leituras: "));
      Serial.print(objDispositivo.Sensores()[0].Leituras().size());
      Serial.print(F(" - "));
      Serial.println(objDispositivo.Sensores()[1].Leituras().size());

      Serial.println(F("No fim do processamento"));
      Serial.println(Dispositivo_controller::DebugMemoriaLivre());

    }
  }
}

void TratarAlteracaoDispositivo(){
  
  Dispositivo objDispositivoNovo;
  std::vector<Controle> lstControles;
  std::vector<Controle> lstControlesNovo;

  if (Dispositivo_controller::NovoDispositivoJson() != ""){
    Serial.println("Existe uma alteração no dispositivo!");
    Serial.println(Dispositivo_controller::NovoDispositivoJson());
    objDispositivoNovo = objDispositivo_controller.CarregarDispositivoJson(Dispositivo_controller::NovoDispositivoJson());
    lstControlesNovo = objDispositivoNovo.Controles();
    lstControles = objDispositivo.Controles();
    for (int idxControlesNovo=0;idxControlesNovo < lstControlesNovo.size(); idxControlesNovo++){
      for (int idxControles=0;idxControles < lstControles.size(); idxControles++){
        if (lstControlesNovo[idxControlesNovo].Codigo() == lstControles[idxControles].Codigo()){
          
          Serial.print("Executa ação no sensor:");
          Serial.println(lstControlesNovo[idxControlesNovo].Codigo());
          Serial.print(" Data atual : ");
          Serial.print(DataHora_utils::ConverterDataEpochParaStr(lstControles[idxControles].AlteradoEm()));
          Serial.print(" Data novo : ");
          Serial.print(lstControlesNovo[idxControlesNovo].AlteradoEm());
          Serial.print(" - ");
          Serial.println(DataHora_utils::ConverterDataEpochParaStr(lstControlesNovo[idxControlesNovo].AlteradoEm()));
            

          if (lstControlesNovo[idxControlesNovo].AlteradoEm() > lstControles[idxControles].AlteradoEm() ){
            
            Serial.print("Valor atual : ");
            Serial.println(lstControles[idxControles].Valor());
            Serial.print("Valor novo : ");
            Serial.println(lstControlesNovo[idxControlesNovo].Valor());
            
            lstControles[idxControles].Valor(lstControlesNovo[idxControlesNovo].Valor());
            lstControles[idxControles].AlteradoEm(DataHora_utils::Agora());

          }
          break;
        }
      }  
      objDispositivo.Controles(lstControles);
    }
    Dispositivo_controller::NovoDispositivoJson("");
  }
}


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  Serial.println(F("Criando dispositivo padrão"));
  objDispositivo = Dispositivo("AmbSense Teste AmbSense 2", "TSTAMB2");
  std::vector<Sensor> lstSensores = {
    Sensor("PPF", "Percentual de Falha"),
    Sensor("PTM", "Tempo Médio")
  };
  std::vector<Controle> lstControles = {
    Controle("LED", "ON/OFF", "Liga desliga led", "OFF", DataHora_utils::Agora(),"")
  };
  objDispositivo.Sensores(lstSensores);
  objDispositivo.Controles(lstControles);
  Serial.println(F("Adicionando ref dispositivo"));
  Dispositivo_controller::RefDispositivo(&objDispositivo);

  Serial.println(F("Inicializando dispositivo"));
  objDispositivo_controller.MaxLeiturasSensor(1000);
  objDispositivo_controller.Inicializar();

  datInicioDispositivo = DataHora_utils::Agora();
  
  Serial.println(F("Criando request handlers"));
  objDispositivo_controller.CriarWebServerRequestHandler("/consultar_tempo_real", HTTP_GET, ConsultarTempoReal);
  
  Serial.println(F("Iniciando webserver"));
  objDispositivo_controller.IniciarWebServer();
  
  // Cria o tratamento do web server no core 0
  Serial.println(F("Criando loop 0"));
  xTaskCreatePinnedToCore(Loop_core0,"Loop_core0",10000,NULL,1,&tskLoop_core0,0);

  CarregarPrimeiraLeitura();

  Serial.println(Dispositivo_controller::DebugMemoriaLivre());

  Serial.println(F("FIM SETUP"));
}


void Loop_core0(void* pvParameters){
  while (true) {
    //Serial.println(F("Loop 0"));
    objDispositivo_controller.ProcessarWebServerRequest();
    vTaskDelay(100);
  }
}


void loop() {
  //Serial.println("Processar a consulta do dispositivo");
  objDispositivo_controller.ProcessarConsultaDispositivo();
  //Serial.println("Tratar alteracao");
  TratarAlteracaoDispositivo();
  //Serial.println("Fazer leituras");
  FazerLeituraSensores();
  //Serial.println("delay");
  delay(100);

}
