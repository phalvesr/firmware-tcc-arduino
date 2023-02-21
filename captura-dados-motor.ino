#define ENTRADA_SENSOR_TENSAO   A1
#define ENTRADA_SENSOR_CORRENTE A0
#define DEBUG_PIN 7

const int QUANTIDADE_LEITURAS_FILTRO_MEDIA_MOVEL = 32;

unsigned int timeCounter = 0;
bool enviaInformacao = false;

ISR (TIMER2_OVF_vect) { 
  timeCounter++;

  // timeCounter = 25? (Ou seja, já passou 100 milisegundo?)
  if (timeCounter == 25) {
    timeCounter = 0;

    enviaInformacao = true;
  }

  TCNT2 = 6;
}

// Declaracao de funcoes
void setupTimerDois();
void setupPins();
void setupSerial();
void enviaInformacaoTensaoCorrente(unsigned int leituraTensao, unsigned int leituraCorrente);
int retornaMediaMovel(int * arr, int size);

// Rotina principal
void setup() {
  setupTimerDois();
  setupPins();
  setupSerial();
}

void setupTimerDois() {
  // Desabilita as entradas dos comparadores (Pg. 126 Datasheet)
  TCCR2A = 0;

  // 0000 0110, configura prescaler de 256
  // Desabilita as saídas dos comparadores (Pg. 129 Datasheet)
  TCCR2B = 6;

  // Inicia a contagem a partir do 6 (Pg. 130 Datasheet)
  TCNT2 = 6;

  // Habilita a interrupção do TMR2 por Overflow (Pg. 131 Datasheet)
  TIMSK2 = 1;
}

void setupPins() {
  pinMode(ENTRADA_SENSOR_TENSAO, INPUT);
  pinMode(ENTRADA_SENSOR_CORRENTE, INPUT);
}

void setupSerial() {
  Serial.begin(9600);
}

// Declaracao funçoes rotina principal
bool passouUmSegundo(int contadorCentenasMilissegundos);
void executaAcoesACadaCemMilissegundos();

int leiturasTensao[QUANTIDADE_LEITURAS_FILTRO_MEDIA_MOVEL];
int leiturasCorrente[QUANTIDADE_LEITURAS_FILTRO_MEDIA_MOVEL];

void loop() {
  static int contadorCentenasMilissegundos = 0;
  if (enviaInformacao) {
    enviaInformacao = false;
    contadorCentenasMilissegundos++;

    executaAcoesACadaCemMilissegundos();
  }


  if (passouUmSegundo(contadorCentenasMilissegundos)) {
    contadorCentenasMilissegundos = 0;

    int leituraTensao = retornaMediaMovel(leiturasTensao, QUANTIDADE_LEITURAS_FILTRO_MEDIA_MOVEL);
    int leituraCorrente = retornaMediaMovel(leiturasCorrente, QUANTIDADE_LEITURAS_FILTRO_MEDIA_MOVEL);

    enviaInformacaoTensaoCorrente(leituraTensao, leituraCorrente);
  }

}

bool passouUmSegundo(int contadorCentenasMilissegundos) {
  return contadorCentenasMilissegundos == 10;
}

void executaAcoesACadaCemMilissegundos() {
  static int indiceTensao = 0;
  static int indiceCorrente = 0;

  adicionaLeituraEmArray(leiturasTensao, QUANTIDADE_LEITURAS_FILTRO_MEDIA_MOVEL, indiceTensao, ENTRADA_SENSOR_TENSAO);
  adicionaLeituraEmArray(leiturasCorrente, QUANTIDADE_LEITURAS_FILTRO_MEDIA_MOVEL, indiceCorrente, ENTRADA_SENSOR_CORRENTE);
}

void adicionaLeituraEmArray(int *arr, int size, int& index, int port) {


  if (index >= size) {
    index = 0;
  }

  arr[index] = analogRead(port);
  index++;
}

int getLeituraCorrente() {

  return analogRead(ENTRADA_SENSOR_CORRENTE);
}

int retornaMediaMovel(int * arr, int size) {

  long sum = 0;
  for (int i = 0; i < size; i++) {
    sum += arr[i];
  }
  
  return (int)(sum / QUANTIDADE_LEITURAS_FILTRO_MEDIA_MOVEL);
}

void enviaInformacaoTensaoCorrente(int leituraTensao, int leituraCorrente) {

  // Informacao enviada no formato por linha: V:<leitura_tensao>;I:<leitura_corrente>;
  Serial.print('V');
  Serial.print(':');
  Serial.print(leituraTensao);
  Serial.print(';');
  Serial.print('I');
  Serial.print(':');
  Serial.print(leituraCorrente);
  Serial.println(';');
}
