#include <AmbSense.h>
#include <Componente_TesteConexao_controller.h>

TaskHandle_t tskLoop_core0;

Dispositivo objDispositivo;
Dispositivo_controller objDispositivo_controller = Dispositivo_controller();
Componente_TesteConexao_controller objComponente_TesteConexao_controller = Componente_TesteConexao_controller();

unsigned long startMillisTestaConexao = 60000;
unsigned long datInicioDispositivo = 0;

void GetDispositivo() {
  //Serial.println("Chamou GetDispositivo");
  Dispositivo objDispositivoAux = objDispositivo_controller.DispositivoComUltimaLeitura();
  std::vector<Sensor>* lstSensores = objDispositivoAux.Sensores();

  for (int idxSensores = 0; idxSensores < lstSensores->size(); idxSensores++) {
    if (*lstSensores->at(idxSensores).Codigo() == "PTM") {
      lstSensores->at(idxSensores).Leituras({ Leitura(String(objComponente_TesteConexao_controller.TempoMedio()), DataHora_utils::Agora()) });
    }

    if (*lstSensores->at(idxSensores).Codigo() == "PPF") {
      lstSensores->at(idxSensores).Leituras({ Leitura(String(objComponente_TesteConexao_controller.PercentFalha()), DataHora_utils::Agora()) });
    }
  }
  //Serial.println("Montou sensores com leitura");
  String strDispositivoJson = objDispositivoAux.ToJSON();

  //Serial.println(F("Vai enviar o response tamanho: "));
  //Serial.println(strDispositivoJson.length());
  WebServer_utils::EnviarWebServerResponse(200, "application/json; charset=utf-8", strDispositivoJson);
  //Serial.println(F("Response enviado"));
}

void LerSensores() {

  unsigned int intTempoTesteConexao = 60 * 1000;    // 5 segs
  unsigned int intTempoMedicaoSensores = 1 * 250;  // 1seg

  Leitura objLeitura;
  String strCodSensor = "";

  if (millis() - startMillisTestaConexao >= intTempoTesteConexao) {

    if (objComponente_TesteConexao_controller.RealizarTeste(3)) {
      startMillisTestaConexao = millis();
      //objComponente_TesteConexao_controller.ExibirResultadoTesteConexao();

      // PTM
      strCodSensor = "PTM"; 
      //Serial.println("Vai pegar a ultima leitura");
      //Serial.println("Criando a leitura atual");
      objLeitura = Leitura(String(objComponente_TesteConexao_controller.TempoMedio()), DataHora_utils::Agora());
      //Serial.println("Adicionar leitura");
      objDispositivo_controller.AdicionarLeitura(strCodSensor, objLeitura, 3);
   
      // PPF
      strCodSensor = "PPF"; 
      objLeitura = Leitura(String(objComponente_TesteConexao_controller.PercentFalha()), DataHora_utils::Agora());
      objDispositivo_controller.AdicionarLeitura(strCodSensor, objLeitura, 10);
      
      //Serial.println(F("No fim do processamento"));
      //Serial.println(Dispositivo_controller::DebugMemoriaLivre());
    }
  }
}
/******************************************************* Controles e Sensores *******************************************************/


void LigarLed() {
  objDispositivo_controller.LedPrincipal(true);
  objDispositivo_controller.AlterarControle("ONOFF", "true");
  WebServer_utils::EnviarWebServerResponse(200, "application/json; charset=utf-8", R"({"resultado":"AMBSENSE_OK", "mensagem":"Controle alterado com sucesso"})");
}

void DesligarLed() {
  objDispositivo_controller.LedPrincipal(false);
  objDispositivo_controller.AlterarControle("ONOFF", "false");
  WebServer_utils::EnviarWebServerResponse(200, "application/json; charset=utf-8", R"({"resultado":"AMBSENSE_OK", "mensagem":"Controle alterado com sucesso"})");
}

void ControlarLed(Controle *objControle){
  Serial.println(">>>>>>>>>>>>>>>>>>>>>> Chamou Callback ControlarLed");
  objDispositivo_controller.LedPrincipal(Texto_utils::toBoolean(*objControle->Valor()));
}

void TrocarLed() {

  Controle_controller objControle_controller = Controle_controller();

  objDispositivo_controller.TrocarStatusLedPrincipal();
  if (objDispositivo_controller.LedPrincipal()) {
    objDispositivo_controller.AlterarControle("ONOFF", "true");
    Serial.println("Trocou para true");
  } else {
    objDispositivo_controller.AlterarControle("ONOFF", "false");
    Serial.println("Trocou para false");
  }
  WebServer_utils::EnviarWebServerResponse(200, "application/json; charset=utf-8", R"({"resultado":"AMBSENSE_OK", "mensagem":"Controle alterado com sucesso"})");
}

void StatusLed() {
  String strResultado = "";
  if (objDispositivo_controller.LedPrincipal()) {
    strResultado = R"({"VALOR" : "ON" })";
  } else {
    strResultado = R"({"VALOR" : "OFF" })";
  }
  WebServer_utils::EnviarWebServerResponse(200, "application/json; charset=utf-8", strResultado);
}

/*************************************************************************************************************************************/

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  Serial.println(F("Criando dispositivo padrão"));

  Serial.println(F("Adicionando ref dispositivo"));
  Dispositivo_controller::RefDispositivo(&objDispositivo);

  // Se não carregar um dispositivo da memória carrega o dispositivo padrão
  
  objDispositivo = Dispositivo("AmbSense Teste AmbSense 2", "TSTAMB2");

  std::vector<Sensor> lstSensores = {
    Sensor("PPF", "Percentual de Falha"),
    Sensor("PTM", "Tempo Médio")
  };
  std::vector<Controle> lstControles = {
    Controle("ONOFF", "ONOFF", "Liga desliga led", "0", DataHora_utils::Agora(), "controle/trocar_led", ControlarLed)
  };
  std::vector<Programa> lstProgramas = {
    Programa("PG1", "Primeiro programa", true, 0, 1800, {}, {}, false, false, {}, "Trocar Led", "http://127.0.0.1/controle/trocar_led", {})
  };
  objDispositivo.Sensores(lstSensores);
  objDispositivo.Controles(lstControles);
  objDispositivo.Programas(lstProgramas);

  Serial.println(F("Inicializando dispositivo"));
  objDispositivo_controller.MaxLeiturasSensor(1000);
  objDispositivo_controller.Inicializar(objDispositivo);

  datInicioDispositivo = DataHora_utils::Agora();
  Serial.print("Dia da semana: ");
  Serial.println(DataHora_utils::PegarDiaSemana(datInicioDispositivo));

  objComponente_TesteConexao_controller.HostDestino("www.google.com");

  Serial.println(F("Criando request handlers"));
  objDispositivo_controller.CriarWebServerRequestHandler("/dispositivo", HTTP_GET, GetDispositivo);
  objDispositivo_controller.CriarWebServerRequestHandler("CONTROLE", "Ligar led", "/controle/ligar_led", HTTP_GET, LigarLed);
  objDispositivo_controller.CriarWebServerRequestHandler("CONTROLE", "Desligar led", "/controle/desligar_led", HTTP_GET, DesligarLed);
  objDispositivo_controller.CriarWebServerRequestHandler("CONTROLE", "Trocar led", "/controle/trocar_led", HTTP_GET, TrocarLed);
  objDispositivo_controller.CriarWebServerRequestHandler("SENSOR", "Status Led", "/sensor/status_led", HTTP_GET, StatusLed);

  Serial.println(F("Iniciando webserver"));
  objDispositivo_controller.IniciarWebServer();

  // Cria o tratamento do web server no core 0
  Serial.println(F("Criando loop 0"));
  xTaskCreatePinnedToCore(Loop_core0, "Loop_core0", 10000, NULL, 1, &tskLoop_core0, 0);

  Serial.println(Dispositivo_controller::DebugMemoriaLivre());
  
  Serial.println(DataHora_utils::ConverterDataEpochParaStr(datInicioDispositivo));
  Serial.println(*objDispositivo.IpLocal());
  Serial.println(F("--- FIM SETUP ---"));

}

void Loop_core0(void* pvParameters) {
  while (true) {
    //Serial.print(F("Loop 0 - "));
    objDispositivo_controller.ProcessarWebServerRequest();
    vTaskDelay(1);
  }
}

unsigned long lngTimerInfo = millis();
void loop() {

  if (millis() - lngTimerInfo > 10000) {
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
  //Serial.println("Fazer leituras");
  LerSensores();
  //Serial.println("Tratar alteracao");
  objDispositivo_controller.TratarAlteracaoDispositivo();
  //Serial.println("Processar os programas do dispositivo");
  objDispositivo_controller.ProcessarProgramas();
  
  vTaskDelay(1);
}
