#include <SoftwareSerial.h>       // Libreria per comunicare con dispositivi tramite porta seriale software
#include <DFRobotDFPlayerMini.h>  // Libreria per il controllo del modulo MP3 DFPlayer Mini

// Impostazione dei pin per il controllo del modulo MP3 (RX e TX)
SoftwareSerial mp3Serial(2, 3); // RX sul pin 2, TX sul pin 3
DFRobotDFPlayerMini myDFPlayer; // Oggetto per il controllo del modulo MP3

// Definizione dei pin per il sensore di distanza (ultrasuoni)
const int trigPin = 9;  // Pin Trigger del sensore
const int echoPin = 10; // Pin Echo del sensore

// Variabili di stato e gestione dei tempi
bool portaAperta = false;               // Stato della porta (aperta o chiusa)
int conteggioSecondiMessaggi = 0;       // Contatore per il numero di messaggi riprodotti
unsigned long tempoUltimoMessaggio = 0; // Timestamp dell'ultimo messaggio riprodotto
unsigned long tempoChiusuraPorta = 0;   // Timestamp della chiusura della porta
bool inAttesaChiusura = false;          // Flag che indica se si sta aspettando la chiusura della porta
unsigned long tempoUltimaAperturaPorta = 0; // Timestamp dell'ultima apertura della porta
bool primoMessaggioRiprodotto = false;  // Flag che indica se il primo messaggio è stato riprodotto

// Intervalli di tempo configurabili
const unsigned long intervalloMessaggio = 10000;   // 10 secondi tra i messaggi
const unsigned long intervalloReset = 180000;      // 3 minuti per il reset del sistema dopo che la porta è chiusa
const unsigned long intervalloTraAperture = 180000; // 3 minuti tra aperture successive della porta

void setup() {
  Serial.begin(9600);          // Inizializza la comunicazione seriale per il monitoraggio
  mp3Serial.begin(9600);       // Inizializza la comunicazione seriale per il DFPlayer Mini
  pinMode(trigPin, OUTPUT);    // Imposta il pin del Trigger del sensore come OUTPUT
  pinMode(echoPin, INPUT);     // Imposta il pin dell'Echo del sensore come INPUT

  // Inizializza il modulo DFPlayer Mini
  if (!myDFPlayer.begin(mp3Serial)) {  // Verifica se il modulo DFPlayer è stato correttamente inizializzato
    Serial.println("Errore DFPlayer"); // Se c'è un errore, stampa un messaggio di errore e blocca l'esecuzione
    while (true);                    // Entriamo in un loop infinito se il DFPlayer non è stato trovato
  }

  myDFPlayer.volume(30);           // Imposta il volume del modulo MP3 a 30
  Serial.println("Sistema pronto."); // Stampa un messaggio di avvio
}

void loop() {
  long duration, distance;

  // Misura la distanza utilizzando il sensore a ultrasuoni
  digitalWrite(trigPin, LOW);            // Imposta il pin del Trigger a LOW
  delayMicroseconds(2);                  // Aspetta per un breve intervallo
  digitalWrite(trigPin, HIGH);           // Imposta il pin del Trigger a HIGH per inviare il segnale
  delayMicroseconds(10);                 // Aspetta per un breve intervallo
  digitalWrite(trigPin, LOW);            // Imposta il pin del Trigger a LOW per terminare il segnale

  // Calcola la durata dell'eco e la distanza
  duration = pulseIn(echoPin, HIGH);     // Misura il tempo che l'eco impiega per tornare
  distance = duration * 0.0344 / 2;      // Calcola la distanza in centimetri (tempo * velocità del suono / 2)

  Serial.print("Distanza: ");
  Serial.println(distance);              // Stampa la distanza misurata sul monitor seriale

  unsigned long ora = millis();          // Ottiene il tempo corrente in millisecondi

  // --- Controllo della Porta Aperta ---
  if (distance > 20) {  // Se la distanza è maggiore di 20 cm, la porta è considerata aperta
    if (!portaAperta) { // Se la porta non è già aperta
      portaAperta = true;  // Imposta lo stato della porta come aperta
      Serial.println("Porta aperta.");  // Stampa sul monitor seriale che la porta è aperta

      // Verifica se sono passati più di 3 minuti dall'ultima apertura della porta
      if (ora - tempoUltimaAperturaPorta >= intervalloTraAperture) {
        primoMessaggioRiprodotto = false;  // Resetta il flag del primo messaggio
        conteggioSecondiMessaggi = 0;      // Resetta il contatore dei messaggi
        Serial.println("Ripristino stato dopo 3 minuti senza aperture."); // Ripristina lo stato
      }
      tempoUltimaAperturaPorta = ora;  // Memorizza il tempo dell'ultima apertura della porta
    }

    // Se è passato abbastanza tempo dall'ultimo messaggio e non sono stati ancora riprodotti due messaggi
    if (conteggioSecondiMessaggi < 2 && ora - tempoUltimoMessaggio >= intervalloMessaggio) {
      int messaggioCasuale = random(1, 6);  // Seleziona un messaggio casuale tra 1 e 5
      myDFPlayer.play(messaggioCasuale);    // Riproduci il messaggio selezionato
      Serial.print("Riprodotto messaggio n°");
      Serial.println(messaggioCasuale);    // Stampa il numero del messaggio riprodotto
      conteggioSecondiMessaggi++;          // Incrementa il contatore dei messaggi
      tempoUltimoMessaggio = ora;          // Memorizza il tempo dell'ultimo messaggio
    }

    // Se sono stati riprodotti due messaggi, entra in modalità di attesa per la chiusura della porta
    if (conteggioSecondiMessaggi >= 2) {
      inAttesaChiusura = true;
    }
  }

  // --- Controllo della Porta Chiusa ---
  else {
    if (portaAperta) {  // Se la porta era aperta e ora si sta chiudendo
      portaAperta = false;   // Imposta lo stato della porta come chiuso
      tempoChiusuraPorta = ora; // Memorizza il tempo della chiusura
      Serial.println("Porta chiusa.");  // Stampa sul monitor seriale che la porta è chiusa
    }

    // Se la porta è rimasta chiusa per più di 3 minuti, resetta il sistema
    if (inAttesaChiusura && (ora - tempoChiusuraPorta >= intervalloReset)) {
      Serial.println("Sistema resettato dopo 3 minuti di porta chiusa.");
      primoMessaggioRiprodotto = false;  // Resetta il flag del primo messaggio
      conteggioSecondiMessaggi = 0;      // Resetta il contatore dei messaggi
      inAttesaChiusura = false;          // Resetta lo stato di attesa chiusura
    }
  }

  delay(500);  // Attende mezzo secondo prima di ripetere la misurazione della distanza
}

