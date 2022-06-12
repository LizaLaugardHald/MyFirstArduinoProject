const int LED1 = 8; //Forsoegspersonen holder oeje med den LED, hvis den taender (RØD)
const int LED2 = 12; //LED til at indikere, hvis en forsoegsperson er faerdig og skal skiftes (BLÅ) og indikerer at spillet starter
const int knap1 = 7; //Forsoegspersonen skal trykke paa denne knap, hvis LED1 lyser
const int knap2 = 4; //Man trykker paa denne knap hvis spillet skal startes første gang og hvis der skal skiftes person
// Knap 3 genstarter programmet og sletter alt og er tilsluttet til "Reset" Arduino pin.


uint32_t startTime; 
uint32_t waitDuration;

int sensorVal1; 
int sensorVal2;


int i = 0; 
int j = 0;

double Sum = 0;

struct DataForEnPerson
{
  int Maalinger[4];
  double Middelvaerdi = 0; 
};

struct DataForEnPerson Personer[3];


enum STATES
{
  START,      // genererer en tilfældig forsinkelse
  DELAY,      // vente for forsinkelsen 
  TEST,       // vente på at en person trykker på en knap
  REPORT,     // resultater
  FINISHED,   // vente på at personen slipper knappen
};

STATES state = START; 


double UdregneMiddelvaerdi(int *x) { 
  for (int k = 0; k < 4; k++)
  {
    Sum += Personer[*x].Maalinger[k];
    if (k == 3) {
      Serial.println("Din totale sum er: ");
      Serial.println(Sum);
    }   
  }
  return (Sum / 4);
}

float getStdDev(int *Person) {
  long total = 0;
  for (int p = 0; p < 4; p++) {
    total = total + (Personer[*Person].Maalinger[p] - Personer[*Person].Middelvaerdi) * (Personer[*Person].Maalinger[p] - Personer[*Person].Middelvaerdi);
  }
  float variance = total / 4;
  float stdDev = sqrt(variance); 
  return stdDev;
}

double HurtigsteTid() {
  double HurtigstePerson = Personer[0].Middelvaerdi;
  for (int m = 0; m < 2; m++) {
    if (HurtigstePerson >= Personer[m + 1].Middelvaerdi) {
      HurtigstePerson = Personer[m + 1].Middelvaerdi;
    }
  }
  return (HurtigstePerson);
}

int HvemErHurtigst() {
  int HurtigstePersonNummer = 1;
  double HurtigstePerson = Personer[0].Middelvaerdi;
  for (int m = 0; m < 2; m++) {
    if (HurtigstePerson >= Personer[m + 1].Middelvaerdi) {
      HurtigstePerson = Personer[m + 1].Middelvaerdi;
      HurtigstePersonNummer = m + 2;
    }
  }
  return (HurtigstePersonNummer);
}

uint32_t TidsMaaling(uint32_t Starttid) {
  uint32_t Maaling = (millis() - Starttid);
  return (Maaling);
}

void setup()
{
  Serial.begin(9600);
  Serial.println("Velkommen");
  Serial.println("Din reaktionstid vil nu blive testet");
  Serial.println("Først kommer du til at se det blå lys, og når du er klar, skal du trykke på knap2 for at det blå lys slukker.");
  Serial.println("Når den røde LED tænder, skal du trykke på knap1 så hurtigt du kan");
  Serial.println("Det er snyd at holde knappen nede....");
  Serial.println("Når du har prøvet 4 gange, lyser det blå lys for at indikere personskift");
  Serial.println("Held og lykke");



  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(knap1, INPUT_PULLUP);
  pinMode(knap2, INPUT_PULLUP);

  digitalWrite(LED1, LOW);
  digitalWrite(LED2, HIGH);

  while (sensorVal2 == 0) {
    sensorVal2 = digitalRead(knap2);
  }
  sensorVal2 = 0;
  digitalWrite(LED2, LOW);


  randomSeed(analogRead(0));

}

void loop()
{

  while (i < 3) { //For personer

    while (j < 4) { //For målinger
      switch (state)
      {
        case START:
          // vær sikker på at rød LED ikke er tændt!
          digitalWrite(LED1, LOW);
          // random delay / tilfældig forsinkelse
          waitDuration = random(500, 20000);
          // sætter starttiden for forsinkelsen
          startTime = millis();
          // skifte til næste skridt
          state = DELAY;
          break;
        case DELAY:
          sensorVal1 = digitalRead(knap1);
          if (sensorVal1 == 1)
          {
            Serial.println("Du snyder!"); // Hvis en person snyder
            // tilbage til start
            delay(1000); 
            state = START;
          }
          // Hvis random delay er færdig (tilfældig forsinkelse)
          if (millis() - startTime >= waitDuration)
          {
            // tænd LED
            digitalWrite(LED1, HIGH);
            // start time indikerer hvornår reaction time måling bliver startet
            startTime = millis();
            // skifte til næste skridt
            state = TEST;
          }
          delay(10); //Tid til at statene kan blive sat.
          break;
        case TEST:
          // venter på at en knap bliver trykket ned
          sensorVal1 = digitalRead(knap1);
          if (sensorVal1 == 1)
          {
            // skifte til næste skirdt
            state = REPORT;
          }
          delay(100); 
          break;
        case REPORT:
          // LED skal slukke
          digitalWrite(LED1, LOW);
          // informerer personen om målingerne
          Personer[i].Maalinger[j] = TidsMaaling(startTime);
          Serial.println(Personer[i].Maalinger[j]);
          // skifte til næste skridt
          state = FINISHED;
          delay(100); 
          break;
        case FINISHED:
          // vente lidt for debounce og flash purposes
          delay(100);
          // skifte LED for at indikere at brugeren burde slippe knappen 
          digitalWrite(LED1, LOW);
          // hvis knappen er slippet/
          sensorVal1 = digitalRead(knap1);
          if (sensorVal1 == 0)
          {
            // tilbage til start
            state = START;
            // vente lidt for debounce og flash purposes
            delay(100);

            j++; //Det skrives for at vi kan være sikker på at det hele er afsluttet og en succesfuld test er udført. Ellers risikerer man for mange eller for lidt j.
            if (j == 4) {

              j = 0; //her sætter vi j lige med 0 for at være klar til den næste person


              Personer[i].Middelvaerdi = UdregneMiddelvaerdi(&i);
              float std = getStdDev(&i); //I denne scope bruger vi i. 
              Serial.println("Dit gennemsnit er: ");
              Serial.println(Personer[i].Middelvaerdi);
              Serial.println("Din varians er: ");
              float VisVarians = std * std;
              Serial.println(VisVarians);
              Serial.println("Din Standard deviation er: "); 
              Serial.println(std);              
              Sum = 0;
              i++;
              digitalWrite(LED2, HIGH);
              while (sensorVal2 == 0) {
                sensorVal2 = digitalRead(knap2);
                if (i == 3) { //Når spillet har været igennem alle 3 personers tur.
                  break; //lykken bliver pauseret og blå led tænder ikke og spillet går videre til at kåre den hurtigste person.
                }
              }
              sensorVal2 = 0;
              digitalWrite(LED2, LOW);
              double BedsteTid = 0;
              int BedstePerson = 0;

              if (i == 3) {
                BedsteTid = HurtigsteTid();
                BedstePerson = HvemErHurtigst();


                Serial.println("Hurtigste tid: ");
                Serial.println(BedsteTid);
                Serial.println("Den hurtigste person er: ");
                Serial.println(BedstePerson);

                int n = 0;
                while (n == 0) { //Løkken kører imens den er 0 og når den bliver andet end 0, så stopper den.

                }
              }
            }
            delay(100); //Det sætter vi for at programmet ikke skriver "Der skete en fejl" når loopet er kørt igennem, når personen trykker på knappen
            break;
          }
        default:
          Serial.println("Der skete en fejl.");
          break;

      }
    }
  }
}
