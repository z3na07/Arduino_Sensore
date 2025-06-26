#include <SoftwareSerial.h>
#include <DFRobotDFPlayerMini.h>

SoftwareSerial mp3Serial(2, 3); // RX, TX
DFRobotDFPlayerMini myDFPlayer;

const int trigPin = 9;
const int echoPin = 10;

bool portaAperta = false;
int conteggioSecondiMessaggi = 0;
unsigned long tempoUltimoMessaggio = 0;
unsigned long tempoChiusuraPorta = 0;
bool inAttesaChiusura = false;
unsigned long tempoUltimaAperturaPorta = 0;
bool primoMessaggioRiprodotto = false; // Dichiarato qui

const unsigned long intervalloMessaggio = 10000;   // 10 secondi tra i messaggi
const unsigned long intervalloReset = 180000;      // 3 minuti
const unsigned long intervalloTraAperture = 180000; // 3 minuti tra aperture successive

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

      // Verifica se è passato più di 3 minuti dall'ultima apertura
      if (ora - tempoUltimaAperturaPorta >= intervalloTraAperture) {
        // Se è passato più di 3 minuti, resetta e riproduci il primo messaggio
        primoMessaggioRiprodotto = false;
        conteggioSecondiMessaggi = 0;
        Serial.println("Ripristino stato dopo 3 minuti senza aperture.");
      }
      tempoUltimaAperturaPorta = ora;
    }

    if (conteggioSecondiMessaggi < 2 && ora - tempoUltimoMessaggio >= intervalloMessaggio) {
      // Riproduzione messaggio casuale
      int messaggioCasuale = random(1, 6); // Scegli un numero tra 1 e 5
      myDFPlayer.play(messaggioCasuale);  // Riproduci messaggio casuale
      Serial.print("Riprodotto messaggio n°");
      Serial.println(messaggioCasuale);
      conteggioSecondiMessaggi++;
      tempoUltimoMessaggio = ora;
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
