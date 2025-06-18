#include <SoftwareSerial.h>
#include <DFRobotDFPlayerMini.h>


SoftwareSerial mp3Serial(2, 3); // RX, TX per la comunicazione con DFPlayer Mini
DFRobotDFPlayerMini myDFPlayer; 

const int trigPin = 9;
const int echoPin = 10;
bool isPlaying = false;  // Variabile di stato per tracciare se un file è in riproduzione

void setup() {
  Serial.begin(9600);
  mp3Serial.begin(9600);  // Inizializza SoftwareSerial per la comunicazione con DFPlayer Mini
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  
  /*if (!myDFPlayer.begin(mp3Serial)) {
    Serial.println("Errore di comunicazione con DFPlayer Mini.");
    while (true);  // Blocca il programma se non riesce a connettersi
  }*/
  myDFPlayer.begin(mp3Serial);

  Serial.println("Sistema pronto.");
}

void loop() {
  long duration, distance;

  // Invia il pulso per misurare la distanza
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  duration = pulseIn(echoPin, HIGH);
  distance = duration * 0.0344 / 2;

  Serial.print("Distanza: ");
  Serial.println(distance);

  if (distance > 20) {  // Se la distanza è maggiore di 20 cm
    if (!isPlaying) {  // Verifica se non sta già riproducendo un file
      myDFPlayer.volume(30);  
      myDFPlayer.play(1);  // Riproduce il file 0001.mp3
      isPlaying = true;  // Imposta la variabile di stato a true quando inizia la riproduzione
      Serial.println("La porta è aperta.");
      delay(1000);  // Aggiungi un ritardo per dare tempo alla riproduzione
    }
  } else {
    isPlaying = false;  // Se la distanza è minore o uguale a 20 cm, resettiamo lo stato
    Serial.println("Distanza sicura");
  }

  delay(1000);  // Attende mezzo secondo prima di eseguire una nuova lettura
}
