
//MODEL
#include <AmbSense.h>
#include <Componente_TesteConexao_controller.h>

TaskHandle_t tskLoop_core0;
        

Dispositivo objDispositivo;
Dispositivo_controller objDispositivo_controller = Dispositivo_controller();

unsigned long startMillisTestaConexao = 60000;
unsigned long datInicioDispositivo = 0;




void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  Serial.println(F("Criando dispositivo padrão"));
  objDispositivo = Dispositivo("AmbSense Teste AmbSense 2", "TSTAMB2");
  std::vector<Sensor> lstSensores = {
    Sensor("PPF", "Percentual de Falha"),
    Sensor("PTM", "Tempo Médio")
  };
  objDispositivo.Sensores(lstSensores);
  Serial.println(F("Adicionando ref dispositivo"));
  Dispositivo_controller::RefDispositivo(&objDispositivo);

  Serial.println(F("Inicializando dispositivo"));
  objDispositivo_controller.MaxLeiturasSensor(500);
  objDispositivo_controller.Inicializar();

  datInicioDispositivo = DataHora_utils::Agora();
  
  Serial.println(F("Criando request handlers"));

  //objDispositivo_controller.CriarWebServerRequestHandler("/consultar", HTTP_GET, ConsultarDispositivo);
  //objDispositivo_controller.CriarWebServerRequestHandler("/alterar", HTTP_POST, AlterarDispositivo);
  //objDispositivo_controller.CriarWebServerRequestHandler("/reiniciar", HTTP_GET, ReiniciarDispositivo);
  
  Serial.println(F("Iniciando webserver"));
  objDispositivo_controller.IniciarWebServer();
  
  // Cria o tratamento do web server no core 0
  Serial.println(F("Criando loop 0"));
  xTaskCreatePinnedToCore(Loop_core0,"Loop_core0",10000,NULL,1,&tskLoop_core0,0);

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



void FazerLeituraSensores() {
  Componente_TesteConexao_controller objComponente_TesteConexao_controller = Componente_TesteConexao_controller("www.google.com");
  
  unsigned int intTempoTesteConexao = 5 * 1000;  // 1 min
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
        if (fabs(atof(objUltLeitura.Valor().c_str()) - objComponente_TesteConexao_controller.TempoMedio()) > 3){
          objLeitura = Leitura(String(objComponente_TesteConexao_controller.TempoMedio()), DataHora_utils::Agora());
          objDispositivo_controller.AdicionarLeitura("PTM", objLeitura);
        }
      }

      // PPF
      objUltLeitura = objDispositivo_controller.UltimaLeitura("PPF");
      if (Texto_utils::isFloat(objUltLeitura.Valor().c_str())){
        if (fabs(atof(objUltLeitura.Valor().c_str()) - objComponente_TesteConexao_controller.PercentFalha()) > 10){
          objLeitura = Leitura(String(objComponente_TesteConexao_controller.PercentFalha()), DataHora_utils::Agora());
          objDispositivo_controller.AdicionarLeitura("PPF", objLeitura);
        }
      }
      
      Serial.print(F("Total de leituras: "));
      Serial.println(objDispositivo.Sensores()[0].Leituras().size());

      Serial.println(F("No fim do processamento"));
      Serial.println(Dispositivo_controller::DebugMemoriaLivre());

    }
  }
}

void TratarAlteracaoDispositivo(){
  
  if (Dispositivo_controller::NovoDispositivoJson() != ""){
    Serial.println("Existe uma alteração no dispositivo!");
    Dispositivo_controller::NovoDispositivoJson("");
  }
  
}


void loop() {
  // put your main code here, to run repeatedly:
  //Serial.println("Tratar alteracao");
  TratarAlteracaoDispositivo();
  //Serial.println("Fazer leituras");
  FazerLeituraSensores();
  //Serial.println("delay");
  delay(500);

}
