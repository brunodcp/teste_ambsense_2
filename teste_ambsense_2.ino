#include <AmbSense.h>
#include <Componente_TesteConexao_controller.h>

TaskHandle_t tskLoop_core0;        

Dispositivo objDispositivo;
Dispositivo_controller objDispositivo_controller = Dispositivo_controller();
Componente_TesteConexao_controller objComponente_TesteConexao_controller = Componente_TesteConexao_controller();

unsigned long startMillisTestaConexao = 60000;
unsigned long datInicioDispositivo = 0;

void CarregarPrimeiraLeitura(){

  std::vector<Sensor>* lstSensores = objDispositivo.Sensores();
  Leitura objLeitura;

  objComponente_TesteConexao_controller.HostDestino("www.google.com");

  if (objComponente_TesteConexao_controller.RealizarTeste(3)) {
    for (int idxSensores=0; idxSensores < lstSensores->size(); idxSensores++){
      if(*lstSensores->at(idxSensores).Codigo() == "PTM"){
         lstSensores->at(idxSensores).Leituras()->push_back({Leitura(String(objComponente_TesteConexao_controller.TempoMedio()), DataHora_utils::Agora())});
      }

      if(*lstSensores->at(idxSensores).Codigo() == "PPF"){
         lstSensores->at(idxSensores).Leituras()->push_back({Leitura(String(objComponente_TesteConexao_controller.PercentFalha()), DataHora_utils::Agora())});
      }
    }        
    //objDispositivo.Sensores(lstSensores);
  }
}

void GetDispositivo(){
  Serial.println("Chamou GetDispositivo");
  Dispositivo objDispositivoAux = objDispositivo_controller.DispositivoComUltimaLeitura(); 
  std::vector<Sensor>* lstSensores = objDispositivoAux.Sensores();

  for (int idxSensores=0; idxSensores < lstSensores->size(); idxSensores++){
    if(*lstSensores->at(idxSensores).Codigo() == "PTM"){
       lstSensores->at(idxSensores).Leituras({Leitura(String(objComponente_TesteConexao_controller.TempoMedio()), DataHora_utils::Agora())});
    }

    if(*lstSensores->at(idxSensores).Codigo() == "PPF"){
       lstSensores->at(idxSensores).Leituras({Leitura(String(objComponente_TesteConexao_controller.PercentFalha()), DataHora_utils::Agora())});
    }
  }        
  Serial.println("Montou sensores com leitura");
  String strDispositivoJson = objDispositivoAux.ToJSON(); 
  
  Serial.println(F("Vai enviar o response tamanho: "));
  Serial.println(strDispositivoJson.length());
  WebServer_utils::EnviarWebServerResponse(200, "application/json; charset=utf-8", strDispositivoJson);
  Serial.println(F("Response enviado"));

}


void LerSensores() {
  
  unsigned int intTempoTesteConexao = 5 * 1000;  // 5 segs
  unsigned int intTempoMedicaoSensores = 1 * 250;  // 1seg

  Leitura objLeitura;
  Leitura objUltLeitura;
  if (millis() - startMillisTestaConexao >= intTempoTesteConexao) {
    
    if (objComponente_TesteConexao_controller.RealizarTeste(3)) {
      startMillisTestaConexao = millis();
      //objComponente_TesteConexao_controller.ExibirResultadoTesteConexao();
      
      // PTM
      objUltLeitura = objDispositivo_controller.UltimaLeitura("PTM");
      if (Texto_utils::isFloat(objUltLeitura.Valor()->c_str())){
        if (fabs(atof(objUltLeitura.Valor()->c_str()) - objComponente_TesteConexao_controller.TempoMedio()) >= 3 || true){
          objLeitura = Leitura(String(objComponente_TesteConexao_controller.TempoMedio()), DataHora_utils::Agora());
          objDispositivo_controller.AdicionarLeitura("PTM", objLeitura);

          Serial.print(F("Total de leituras: "));
          Serial.print(objDispositivo.Sensores()->at(0).Leituras()->size());
          Serial.print(F(" - "));
          Serial.print(objDispositivo.Sensores()->at(1).Leituras()->size());
          Serial.print(F(" Total: "));
          Serial.println(objDispositivo.Sensores()->at(0).Leituras()->size() + objDispositivo.Sensores()->at(1).Leituras()->size());

        }
      }

      // PPF
      objUltLeitura = objDispositivo_controller.UltimaLeitura("PPF");
      if (Texto_utils::isFloat(objUltLeitura.Valor()->c_str())){
        if (fabs(atof(objUltLeitura.Valor()->c_str()) - objComponente_TesteConexao_controller.PercentFalha()) >= 10 || true){
          objLeitura = Leitura(String(objComponente_TesteConexao_controller.PercentFalha()), DataHora_utils::Agora());
          objDispositivo_controller.AdicionarLeitura("PPF", objLeitura);

          Serial.print(F("Total de leituras: "));
          Serial.print(objDispositivo.Sensores()->at(0).Leituras()->size());
          Serial.print(F(" - "));
          Serial.print(objDispositivo.Sensores()->at(1).Leituras()->size());
          Serial.print(F(" Total: "));
          Serial.println(objDispositivo.Sensores()->at(0).Leituras()->size() + objDispositivo.Sensores()->at(1).Leituras()->size());

        }
      }
      
      //Serial.println(F("No fim do processamento"));
      //Serial.println(Dispositivo_controller::DebugMemoriaLivre());

    }
  }
}

void TratarAlteracaoDispositivo(){
  
  Dispositivo objDispositivoNovo;
  std::vector<Controle>* lstControles;
  std::vector<Controle>* lstControlesNovo;

  if (Dispositivo_controller::NovoDispositivoJson() != ""){
    Serial.println("Existe uma alteração no dispositivo!");
    Serial.println(Dispositivo_controller::NovoDispositivoJson());
    objDispositivoNovo = objDispositivo_controller.CarregarJson(Dispositivo_controller::NovoDispositivoJson());
    lstControlesNovo = objDispositivoNovo.Controles();
    lstControles = objDispositivo.Controles();
    for (int idxControlesNovo=0;idxControlesNovo < lstControlesNovo->size(); idxControlesNovo++){
      for (int idxControles=0;idxControles < lstControles->size(); idxControles++){
        if (*lstControlesNovo->at(idxControlesNovo).Codigo() == *lstControles->at(idxControles).Codigo()){
          
          Serial.print("Executa ação no sensor:");
          Serial.println(*lstControlesNovo->at(idxControlesNovo).Codigo());
          Serial.print(" Data atual : ");
          Serial.print(DataHora_utils::ConverterDataEpochParaStr(*lstControles->at(idxControles).AlteradoEm()));
          Serial.print(" Data novo : ");
          Serial.print(*lstControlesNovo->at(idxControlesNovo).AlteradoEm());
          Serial.print(" - ");
          Serial.println(DataHora_utils::ConverterDataEpochParaStr(*lstControlesNovo->at(idxControlesNovo).AlteradoEm()));
            

          if (*lstControlesNovo->at(idxControlesNovo).AlteradoEm() > *lstControles->at(idxControles).AlteradoEm() ){
            
            Serial.print("Valor atual : ");
            Serial.println(*lstControles->at(idxControles).Valor());
            Serial.print("Valor novo : ");
            Serial.println(*lstControlesNovo->at(idxControlesNovo).Valor());
            
            lstControles->at(idxControles).Valor(*lstControlesNovo->at(idxControlesNovo).Valor());
            lstControles->at(idxControles).AlteradoEm(DataHora_utils::Agora());
            
            if (*lstControlesNovo->at(idxControlesNovo).Tipo() == "ONOFF"){
              objDispositivo_controller.LedPrincipal(Texto_utils::toBoolean(*lstControlesNovo->at(idxControlesNovo).Valor()));
            }
          }
          break;
        }
      }  
      //objDispositivo.Controles(lstControles);
    }
    Dispositivo_controller::NovoDispositivoJson("");
    Dispositivo_controller::SalvarDispositivo();
  }
}


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  Serial.println(F("Criando dispositivo padrão"));
  
  Serial.println(F("Adicionando ref dispositivo"));
  Dispositivo_controller::RefDispositivo(&objDispositivo);

  // Se não carregar um dispositivo da memória carrega o dispositivo padrão
  if (!objDispositivo_controller.CarregarDispositivo()){
    objDispositivo = Dispositivo("AmbSense Teste AmbSense 2", "TSTAMB2");

    std::vector<Sensor> lstSensores = {
      Sensor("PPF", "Percentual de Falha"),
      Sensor("PTM", "Tempo Médio")
    };
    std::vector<Controle> lstControles = {
      Controle("ONOFF", "ONOFF", "Liga desliga led", "0", DataHora_utils::Agora(),"controle/trocar_led")
    };
    std::vector<Programa> lstProgramas = {
      Programa("PG1", "Primeiro programa", true, 0, 1800, {}, {}, false, false, "", "Trocar Led", "http://127.0.0.1/controle/trocar_led", {})
    };
    objDispositivo.Sensores(lstSensores);
    objDispositivo.Controles(lstControles);
    objDispositivo.Programas(lstProgramas);
  }

  Serial.println(F("Inicializando dispositivo"));
  objDispositivo_controller.MaxLeiturasSensor(1000);
  objDispositivo_controller.Inicializar();

  Serial.print(F("Total de leituras: "));
  Serial.print(objDispositivo.Sensores()->at(0).Leituras()->size());
  Serial.print(F(" - "));
  Serial.print(objDispositivo.Sensores()->at(1).Leituras()->size());
  Serial.print(F(" Total: "));
  Serial.println(objDispositivo.Sensores()->at(0).Leituras()->size() + objDispositivo.Sensores()->at(1).Leituras()->size());


  datInicioDispositivo = DataHora_utils::Agora();
  Serial.print("Dia da semana: ");
  Serial.println(DataHora_utils::PegarDiaSemana(datInicioDispositivo));

  Serial.println(F("Criando request handlers"));
  objDispositivo_controller.CriarWebServerRequestHandler("/dispositivo", HTTP_GET, GetDispositivo);
  objDispositivo_controller.CriarWebServerRequestHandler("CONTROLE", "Ligar led","/controle/ligar_led", HTTP_GET, LigarLed);
  objDispositivo_controller.CriarWebServerRequestHandler("CONTROLE", "Desligar led","/controle/desligar_led", HTTP_GET, DesligarLed);
  objDispositivo_controller.CriarWebServerRequestHandler("CONTROLE", "Trocar led","/controle/trocar_led", HTTP_GET, TrocarLed);
  objDispositivo_controller.CriarWebServerRequestHandler("SENSOR", "Status Led","/sensor/status_led", HTTP_GET, StatusLed);
  
  Serial.println(F("Iniciando webserver"));
  objDispositivo_controller.IniciarWebServer();
  
  // Cria o tratamento do web server no core 0
  Serial.println(F("Criando loop 0"));
  xTaskCreatePinnedToCore(Loop_core0,"Loop_core0",10000,NULL,1,&tskLoop_core0,0);

  CarregarPrimeiraLeitura();

  Serial.println(Dispositivo_controller::DebugMemoriaLivre());
  objDispositivo_controller.LedPrincipal(0);
  
  Serial.println(DataHora_utils::ConverterDataEpochParaStr(datInicioDispositivo));
  Serial.println(*objDispositivo.IpLocal());
  Serial.println(F("--- FIM SETUP ---"));
}

void Loop_core0(void* pvParameters){
  while (true) {
    //Serial.print(F("Loop 0 - "));
    objDispositivo_controller.ProcessarWebServerRequest();
    
    vTaskDelay(100);
    
  }
}

void LigarLed(){
  objDispositivo_controller.LedPrincipal(true);
  objDispositivo_controller.AlterarControle("ONOFF", "true");
  WebServer_utils::EnviarWebServerResponse(200, "application/json; charset=utf-8",  R"({"resultado":"AMBSENSE_OK", "mensagem":"Controle alterado com sucesso"})");
  
}

void DesligarLed(){
  objDispositivo_controller.LedPrincipal(false);
  objDispositivo_controller.AlterarControle("ONOFF", "false");
  WebServer_utils::EnviarWebServerResponse(200, "application/json; charset=utf-8",  R"({"resultado":"AMBSENSE_OK", "mensagem":"Controle alterado com sucesso"})");
}


void TrocarLed(){
  objDispositivo_controller.TrocarStatusLedPrincipal();
  if (objDispositivo_controller.LedPrincipal()){
    objDispositivo_controller.AlterarControle("ONOFF", "true");
    Serial.println("Trocou para true");
  } else {
    objDispositivo_controller.AlterarControle("ONOFF", "false");
    Serial.println("Trocou para false");
  }
  Dispositivo_controller::SalvarDispositivo();
  WebServer_utils::EnviarWebServerResponse(200, "application/json; charset=utf-8",  R"({"resultado":"AMBSENSE_OK", "mensagem":"Controle alterado com sucesso"})");
}

void StatusLed(){
  String strResultado = "";
  if(objDispositivo_controller.LedPrincipal()){
    strResultado = R"({"VALOR" : "ON" })";
  } else {
    strResultado = R"({"VALOR" : "OFF" })";
  }
  WebServer_utils::EnviarWebServerResponse(200, "application/json; charset=utf-8", strResultado);
}


unsigned long lngTimerInfo = millis();
void loop() {

  if (millis() - lngTimerInfo > 10000 ) {
    lngTimerInfo = millis();
    Serial.print("*************************** Hora definida: ");
    Serial.print(DataHora_utils::DataHoraDefinida());
    Serial.print(" - Inicio: ");
    Serial.print(DataHora_utils::ConverterDataEpochParaStr(datInicioDispositivo));
    Serial.print(" - Agora: ");
    Serial.print(DataHora_utils::Agora(0));
    Serial.print(" - IP: ");
    Serial.print(*objDispositivo.IpLocal());
    Serial.println();
  }
  
  //Serial.println("Processar os programas do dispositivo");
  objDispositivo_controller.ProcessarProgramas();
  //Serial.println("Tratar alteracao");
  TratarAlteracaoDispositivo();
  //Serial.println("Fazer leituras");
  LerSensores();
  //Serial.println("delay");
  //delay(100);
  vTaskDelay(10);

}
