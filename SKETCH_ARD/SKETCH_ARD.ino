#include <SoftwareSerial.h>
#include <DFRobotDFPlayerMini.h>

SoftwareSerial mp3Serial(2, 3); // RX, TX
DFRobotDFPlayerMini myDFPlayer;

const int trigPin = 9;
const int echoPin = 10;

bool primoMessaggioRiprodotto = false;
bool portaAperta = false;
int conteggioSecondiMessaggi = 0;
unsigned long tempoUltimoMessaggio = 0;
unsigned long tempoChiusuraPorta = 0;
bool inAttesaChiusura = false;

const unsigned long intervalloMessaggio = 30000;   // 30 secondi
const unsigned long intervalloReset = 180000;      // 3 minuti

void setup() {
  Serial.begin(9600);
  mp3Serial.begin(9600);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  if (!myDFPlayer.begin(mp3Serial)) {
    Serial.println("Errore DFPlayer");
    while (true);
  }

  myDFPlayer.volume(30);
  Serial.println("Sistema pronto.");
}

void loop() {
  long duration, distance;

  // Misura distanza
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  duration = pulseIn(echoPin, HIGH);
  distance = duration * 0.0344 / 2;

  Serial.print("Distanza: ");
  Serial.println(distance);

  unsigned long ora = millis();

  // --- Porta Aperta ---
  if (distance > 20) {
    if (!portaAperta) {
      portaAperta = true;
      Serial.println("Porta aperta.");
    }

    if (!primoMessaggioRiprodotto) {
      myDFPlayer.play(1);  // Primo messaggio
      Serial.println("Riprodotto messaggio 1");
      primoMessaggioRiprodotto = true;
      tempoUltimoMessaggio = ora;
      conteggioSecondiMessaggi = 0;
    }

    // Riproduzione dei messaggi ripetuti ogni 30s (massimo 2 volte)
    if (primoMessaggioRiprodotto && conteggioSecondiMessaggi < 2 && ora - tempoUltimoMessaggio >= intervalloMessaggio) {
      myDFPlayer.play(2);  // Secondo messaggio
      conteggioSecondiMessaggi++;
      tempoUltimoMessaggio = ora;
      Serial.print("Riprodotto messaggio 2, volta n°");
      Serial.println(conteggioSecondiMessaggi);
    }

    // Se la porta è ancora aperta e ha raggiunto il massimo dei messaggi, aspetta chiusura
    if (conteggioSecondiMessaggi >= 2) {
      inAttesaChiusura = true;
    }
  }

  // --- Porta Chiusa ---
  else {
    if (portaAperta) {
      portaAperta = false;
      tempoChiusuraPorta = ora;
      Serial.println("Porta chiusa.");
    }

    // Se in attesa di chiusura e la porta resta chiusa per almeno 3 minuti, resetta
    if (inAttesaChiusura && (ora - tempoChiusuraPorta >= intervalloReset)) {
      Serial.println("Sistema resettato dopo 3 minuti di porta chiusa.");
      primoMessaggioRiprodotto = false;
      conteggioSecondiMessaggi = 0;
      inAttesaChiusura = false;
    }
  }

  delay(500);
}
