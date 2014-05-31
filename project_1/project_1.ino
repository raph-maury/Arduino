#include <SoftwareSerial.h>
#include <SD.h>

File myFile;
const int chipSelect = 10;
SoftwareSerial gprsSerial(7, 8);

char messageRead[300];

//Variable de temps
unsigned long SendSmsPeriod=40000;
unsigned long RefreshDataPeriod=20000;

unsigned long NextGetDataTime=0;
unsigned long SendSmsTime=0;


void setup()
{
  gprsSerial.begin(19200);
  Serial.begin(19200);
  
  //fonction SIM900power
  SIM900power();
  Serial.println("Config SIM900 en cours");
  
  //delai de 20s secondes
  delay(20000);
  
  pinMode(10, OUTPUT);  
}


////////////////LOOP/////////////////////
void loop()
{
   refresh();
   sendSMS();
}


//focntion pour allumer la carte gsm
void SIM900power()
{
  
  digitalWrite(9, HIGH);
  delay(1000);
  digitalWrite(9, LOW);
  delay(5000);
}

void toSerial()
{
  while(gprsSerial.available()!=0)
  {
    Serial.write(gprsSerial.read());
  }
}

void enregistrement()
{
  while(gprsSerial.available())
  {
    myFile.write(gprsSerial.read());
  }
}

void sendSMS()
{
  gprsSerial.print("AT+CMGF=1\r");                                                        // AT command to send SMS message
  delay(100);
  gprsSerial.println("AT + CMGS = \"numero_°\"");                                     // recipient's mobile number, in international format
  delay(100);
  
  Serial.println("sending sms ");
  Serial.println(messageRead);
  gprsSerial.println(messageRead);        // message to send
  delay(100);
  gprsSerial.println((char)26);                       // End AT command with a ^Z, ASCII code 26
  delay(100); 
  gprsSerial.println();
  delay(5000);                                     // give module time to send SMS
  
  Serial.println("sent sms ");
  SIM900power();                                   // turn off module
}

void refresh(){
  
 if (!SD.begin(chipSelect)) {
    Serial.println("Echec initialisation ou carte inexistant");
    // don't do anything more:
    return;
  }
  Serial.println("Initialisation SD reussie !");
  
  Serial.println("Config check");
  gprsSerial.flush();
  Serial.flush();
  
  //Création du fichier
  myFile = SD.open("test.txt", FILE_WRITE);

  // attach or detach from GPRS service 
  gprsSerial.println("AT+CGATT?");
  delay(100);
  toSerial();


  // bearer settings
  gprsSerial.println("AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"");
  delay(2000);
  toSerial();

  // bearer settings
  gprsSerial.println("AT+SAPBR=3,1,\"APN\",\"wapsfr\""); //Élément a changer si l'opérateur varie 
  delay(2000);
  toSerial();

  // bearer settings
  gprsSerial.println("AT+SAPBR=1,1");
  delay(2000);
  toSerial();
  
   // initialize http service
   gprsSerial.println("AT+HTTPINIT");
   delay(2000); 
   toSerial();

   //lien vers le fichier txt
   gprsSerial.println("AT+HTTPPARA=\"URL\",\"http://@serveur/fichier.txt\"");
   delay(2000);
   toSerial();

   // set http action type 0 = GET, 1 = POST, 2 = HEAD
   gprsSerial.println("AT+HTTPACTION=0");
   delay(6000);
   toSerial();

   // read server response
   gprsSerial.println("AT+HTTPREAD"); 
   delay(1000);
   enregistrement();

   gprsSerial.println("");
   gprsSerial.println("AT+HTTPTERM");
   toSerial();
   delay(300);

   gprsSerial.println("");
   delay(10000);
   
   myFile.close();
   
   //Ouvre le fichier
  myFile = SD.open("test.txt");
  int mi = 0;
  // Si le fichier est invalide
  if (myFile) {
    while (myFile.available()) {
      // ajouter tes caractères dans ton tableau
      messageRead[mi++] += ((char)myFile.read());
      Serial.write(messageRead[mi-1]);
      messageRead[mi] += (char)'\0';
      
    }
    
    myFile.close();
  }  
  // Si il ne s'ouvre pas alors affiche erreur
  else {
    Serial.println("Erreur d'ouverture");
  } 
   
   
   Serial.println("done."); 
}
