#include "HX711.h"
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SimpleRotary.h>
#include <Adafruit_NeoPixel.h>
#define calibration_factor -463000.00

#define som D0

#define NUM_LEDS 30            // number of leds in strip
#define LED_PIN D8             // pin for led strip

Adafruit_NeoPixel pixels(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);
LiquidCrystal_I2C lcd(0x27, 20, 4); // Default address of most PCF8574 modules, change according
// Pin A, Pin B, Button Pin

#define LOADCELL_DOUT_PIN  D3
#define LOADCELL_SCK_PIN  D4
HX711 scale;
SimpleRotary rotary(D7, D6, D5);
#define NUMCATEG 5
#define MAXPRODS 31
#define NUMMENUS 7
int cor = 0;
int intLuz = 250;

String produto[][MAXPRODS + 1] = {{"Parafusos",
    "2.5x12",
    "2.9x13",
    "3x16",
    "3x20",
    "3x25" ,
    "3x30",
    "3x40",
    "3.5x12",
    "3.5x16",
    "4x20",
    "4x25",
    "4x30",
    "4x35",
    "4x40",
    "4x50",
    "4x60",
    "4,5x20",
    "4,5x25",
    "5x30",
    "5x40",
    "5x50",
    "5x60",
    "5x70",
    "6x30",
    "6x40",
    "6x50",
    "6x60",
    "6x70",
    "6x80",
    "6x100",
    "6x120"},
    {"P.Autoenrosc", "6x30", "6x40", "6x50", "6x60", "6x70", "6.3x19", "6.3x25", "6.3x70", "6.3x80", "6.3x100","6.3x110", "6.3x120", "6.3x130"},
    {"Porcas", "M4", "M5", "M6", "M8", "M10", "M12", "M14", "M16", "M18", "M20"},
    {"Anilhas", "M5", "M6", "M8", "M10", "M12", "M14", "M16" },
    {"Pregos","Cavilha 4", "Cavilha 5", "Cavilha 6", "Cavilha 7","Cavilha 8" "Galiota 10", "Telhado 9", "Fasq 3x15", "Fasq 4x14", "Setia" }
  };
  long prodPeso[][MAXPRODS + 1] = {{290, 520, 690, 670,720,899,1120,715,690,960,1010,1444,2320,2120,2700,3500,1500,1180,1500,1900
  , 2150, 5632, 33310,55,5555,2834,2222,6982,6987,2572},
    {105,700,1300,2700,5500,7400,8700,8006,5000,2642,4126,7133},
    {40, 800, 2000,4200,10000,14600,20600,27700,40400,53400},
    {100,700,1300,2700,5500,7400,8700},
    {300,800,1200,2964,3504,6700,9000}
  };  // Peso em miligramas
  int quantProd[NUMCATEG];
  String menu[NUMMENUS] = {"Mudar de medida ", "Reset Tara", "Taxa atualizacao", "Cor dos Leds", "Brilho do Leds", "Informacoes", "Voltar"};
  int cores[8][3] = {{0, 0, 0}, {1, 0, 0}, {0, 1, 0}, {0, 0, 1}, {1, 1, 0}, {0, 1, 1}, {1, 0, 1}, {1, 1, 1}};
  String nomeCor[8] = {"Desligado", "Vermelho", "  Verde", "   Azul", " Amarelo", "   Ciao", " Magenta", " Branco" };
  int prodAtual = -1;
  int catAtual = 0;
  int ecra = 0;
  int opcaoMenu = 0;
  int alterado = 1;
  int unid = 0; // 0-Gr, 1-Kg
  int brilho = 1;
  int vol = 50;


  unsigned long tempo;
  int atualiza = 500;

  int ctr = 0;
  byte lastDir = 0;



  void setup()
  {
    pixels.begin();
    pinMode(som, OUTPUT);
    lcd.init();
    // this stop the library(LCD_I2C) from calling Wire.begin()
    lcd.backlight();
    lcd.clear();
    lcd.setCursor(4, 0);
    lcd.print("Lorex Smart");
    lcd.setCursor(6, 1);
    lcd.print("Balance");
    delay(25);
    rotary.setTrigger(HIGH);
    //rotary.setErrorDelay(20);
    Serial.begin(9600);
    scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
    scale.set_scale(calibration_factor); //This value is obtained by using the SparkFun_HX711_Calibration sketch
    scale.tare(); //Assuming there is no weight on the scale at start up, reset the scale to 0
    tempo = millis();
    atualizQuantProd();

    pixels.clear(); // Set all pixel colors to 'off'

    // The first NeoPixel in a strand is #0, second is 1, all the way up
    // to the count of pixels minus one.
    for (int i = 0; i < NUM_LEDS; i++)
      pixels.setPixelColor(i, pixels.Color(2, 2, 60));
    pixels.show();



    delay(2000);
  }

  void loop()
  {
    int rDir = rotary.rotate();
    int rBtn = rotary.push();
    int rLBtn = rotary.pushLong(2000);
    if (ecra == 0) {
      if (millis() - tempo > atualiza) {
        ecra0();
        tempo = millis();
      }
      if (rDir == 1) { // CW
        prodAtual++;
        if (prodAtual >= quantProd[catAtual])
          prodAtual = 0;
        rDir = 0;
        alterado = 1;
        //lastDir = rDir;
      }
      if (rDir == 2) { // CCW
        prodAtual--;
        if (prodAtual < 0)
          prodAtual = quantProd[catAtual] - 1;
        rDir = 0;
        alterado = 1;
        //lastDir = rDir;
      }
      if (rLBtn == 1) {
        tone(som, 800, 20);
        ecra = 1;
        rLBtn = 0;
        alterado = 1;
      }
      if (rBtn == 1) {
        tone(som, 800, 20);
        catAtual++;
        if (catAtual >= NUMCATEG)
          catAtual = 0;
        prodAtual = 0;
        rBtn = 0;
        alterado = 1;
      }
    }

    if (ecra == 1) { // Menu
      if (alterado == 1) {
        ecra1();
        alterado = 0;
      }
      if (rDir == 1) { // CW
        opcaoMenu++;
        if (opcaoMenu >= NUMMENUS)
          opcaoMenu = 0;
        rDir = 0;
        alterado = 1;
      }
      if (rDir == 2) { // CCW
        opcaoMenu--;
        if (opcaoMenu < 0)
          opcaoMenu = NUMMENUS - 1;
        rDir = 0;
        alterado = 1;
      }
      if (rBtn == 1) {
        tone(som, 800, 20);
        switch (opcaoMenu) {
          case 0:
            ecra = 2;
            break;
          case 1:
            ecra = 3;
            break;
          case 2:
            ecra = 4;
            break;
          case 3:
            ecra = 5;
            break;
          case 4:
            ecra = 6;
            break;
          case 5:
            ecra = 7;
            break;
          case 6:
            ecra = 8;
            break;
        }
        rBtn = 0;
        alterado = 1;
      }
      if (rLBtn == 1) {
        tone(som, 800, 20);
        ecra = 0;
        rLBtn = 0;
        alterado = 1;
      }

    }

    if (ecra == 2) { // Unidades
      Serial.println("ecra 3");
      if (rDir == 1 || rDir == 2) {
        if (unid == 0)
          unid = 1;
        else unid = 0;
        rDir = 0;
        alterado = 1;
      }
      if (rLBtn == 1) {
        ecra = 0;
        rLBtn = 0;
        alterado = 1;
      }
      if (rBtn == 1) {
        ecra = 0;
        rBtn = 0;
        alterado = 1;
      }
      if (alterado == 1) {
        ecra2();
      }
    }

    if (ecra == 3) { // tara
      scale.tare();
      alterado = 1;
      ecra = 0;

    }

    if (ecra == 4) { // Taxa de atualizacao
      Serial.println("ecra 4");
      if (alterado == 1) {
        ecra4();
      }
      if (rDir == 1) {
        atualiza += 100;
        if (atualiza > 1000) atualiza = 1000;
        rDir = 0;
        alterado = 1;
      }
      if (rDir == 2) {
        atualiza -= 100;
        if (atualiza < 100) atualiza = 100;
        rDir = 0;
        alterado = 1;
      }
      if (rLBtn == 1) {
        ecra = 0;
        rLBtn = 0;
      }
      if (rBtn == 1) {
        ecra = 0;
        rBtn = 0;
      }
    }

    if (ecra == 5) { // Leds
      Serial.println("ecra 5");
      if (alterado == 1) {
        ecra5();
      }
      if (rDir == 1) {
        cor++;
        if (cor > 7) cor = 0;
        rDir = 0;
        alterado = 1;
      }
      if (rDir == 2) {
        cor;
        if (cor < 0) cor = 7;
        rDir = 0;
        alterado = 1;
      }
      if (rLBtn == 1) {
        ecra = 0;
        rLBtn = 0;
      }
      if (rBtn == 1) {
        ecra = 0;
        rBtn = 0;
      }

    }

    if (ecra == 6) { // Brilho

      Serial.println("ecra 6");
      if (alterado == 1) {
        ecra6();
      }
      if (rDir == 1) {
        intLuz += 16;
        if (intLuz > 255) intLuz = 255;
        rDir = 0;
        alterado = 1;
      }
      if (rDir == 2) {
        intLuz -= 16;
        if (intLuz < 16) intLuz = 16;
        rDir = 0;
        alterado = 1;
      }
      if (rLBtn == 1) {
        ecra = 0;
        rLBtn = 0;
      }
      if (rBtn == 1) {
        ecra = 0;
        rBtn = 0;
      }
    }
    if (ecra == 7) {
      Serial.println("ecra 7");
      if (alterado == 1) {
        ecra7();
      }
      if (rLBtn == 1) {
        ecra = 0;
        rLBtn = 0;
      }
      if (rBtn == 1) {
        ecra = 0;
        rBtn = 0;
      }
    }

    if (ecra == 8) {
      Serial.println("ecra 8");
      alterado = 1;
      ecra = 0;
      if (rLBtn == 1) {
        ecra = 0;
        rLBtn = 0;
      }
      if (rBtn == 1) {
        ecra = 0;
        rBtn = 0;
      }
    }

  }

  void atualizQuantProd() {
    for (int i = 0; i < NUMCATEG; i++) {
      int max = 0;
      for (int j = 0; j <= MAXPRODS && max == 0; j++) {
        if (prodPeso[i][j] == 0) {
          max = j;
        }
        quantProd[i] = max;
      }
    }
  }

  void ecra0() {
    lcd.clear();
    lcd.setCursor(6, 0);
    lcd.print("Leitura:");
    // lcd.setCursor(6, 4);
    int peso = lerPeso();

    if (prodAtual != -1) {
      lcd.setCursor(0, 2);
      lcd.print(produto[catAtual][0]); //print da categoria
      lcd.setCursor(0, 3);
      lcd.print(produto[catAtual][prodAtual + 1]); //print da subcategoria
      lcd.setCursor(6, 1);
      if (peso == -10)
        lcd.print("Erro");
      else {
        if (unid == 0) {
          lcd.print(peso);
          lcd.setCursor(9, 1);
          lcd.print(" Gramas ");
        } else {
          lcd.setCursor(9, 1);
          lcd.print(" Kg ");
        }
        lcd.setCursor(10, 3);
        lcd.print("Quant:");
        lcd.print(peso / (prodPeso[catAtual][prodAtual] / 1000.0), 0);
        lcd.print("   ");
      }
    } else {
      lcd.setCursor(6, 1);
      if (peso == -10)
        lcd.print("Erro");
      else if (unid == 0) {
        lcd.print(peso /1000.0);
        lcd.setCursor(10, 1);
        lcd.print(" Kg");
      } else {
        lcd.print(peso);
        lcd.setCursor(10, 1);
        lcd.print(" Gr");
      }
    }
  }

  void ecra1() {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("--------Menu--------->");
    lcd.setCursor(3, 2);
    lcd.print(menu[opcaoMenu]);
    lcd.print("   ");
    alterado = 0;
    delay(50);
  }

  void ecra2() {
    lcd.clear();
    lcd.setCursor(6, 0);
    lcd.print("Unidades");
    lcd.setCursor(0, 2);
    if (unid == 0)
      lcd.print("     <Gr>    Kg    ");
    else
      lcd.print("      Gr    <Kg>   ");
    alterado = 0;
    delay(50);
  }

  void ecra3() {
    /* lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Brilho");
      lcd.setCursor(0, 1);
      if (brilho == 1)
        lcd.print("  <ON>    OFF   ");
      else
        lcd.print("   ON    <OFF>  ");
      alterado = 0;
      delay(50); */

  }

  void ecra4() {
    lcd.clear();
    lcd.setCursor(2, 0);
    lcd.print("Taxa Atualizacao");
    lcd.setCursor(7, 2);
    lcd.print(atualiza);
    lcd.print(" ms ");
    alterado = 0;
    delay(50);
  }

  void ecra5() {
    pixels.clear(); // Set all pixel colors to 'off'

    // The first NeoPixel in a strand is #0, second is 1, all the way up
    // to the count of pixels minus one.
    for (int i = 0; i < NUM_LEDS; i++)
      pixels.setPixelColor(i, pixels.Color(cores[cor][0]*intLuz, cores[cor][1]*intLuz, cores[cor][2]*intLuz));
    pixels.show();
    /*lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Volume");
      lcd.setCursor(3, 1);
      for (int i = 0; i < vol / 10; i++)
      lcd.print("X");
      // lcd.print(atualiza);
      // lcd.print(" ms ");
      alterado = 0;
      delay(50);*/
    lcd.clear();
    lcd.setCursor(5, 0);
    lcd.print("Cor dos Leds");
    lcd.setCursor(5, 2);
    lcd.print(nomeCor[cor]);
    //lcd.print(" ms ");
    alterado = 0;
    delay(50);
  }

  void ecra6() {
    pixels.clear();
    for (int i = 0; i < NUM_LEDS; i++)
      pixels.setPixelColor(i, pixels.Color(cores[cor][0]*intLuz, cores[cor][1]*intLuz, cores[cor][2]*intLuz));
    pixels.show();
    lcd.clear();
    lcd.setCursor(1, 0);
    lcd.print("Intensidade da Luz");
    lcd.setCursor(8, 2);
    lcd.print(intLuz / 16);
    //lcd.print(" ms ");
    alterado = 0;
    delay(50);

  }
  void ecra7() {
    lcd.clear();
    lcd.setCursor(4, 0);
    lcd.print("Informacoes");
    lcd.setCursor(2, 1);
    lcd.print(" Realizado por  ");
    lcd.setCursor(1, 2);
    lcd.print(" Francisco Ruivo   ");
    lcd.setCursor(5, 3);
    lcd.print(" 2020/2021   ");
    alterado = 0;
    delay(50);
  }




  int lerPeso() {
    int peso = -1000 * scale.get_units();
    Serial.println(peso);
    if (peso >= -2 && peso <= 0)
      return 0;
    else if (peso < -2)
      return -10;
    else
      return peso;
  }

  /*Menus
    - Mundar de medida
    - Reset Tara
    - Taxa de atualização
    - DefeniçoesLeds
    - Brilho
    - Voltar
    - Info

     0 1 2 3 4 5 6 7 8 9 10 11 13 14 15 16 17 18 19 20
     0 -----------------
     1
     2
     4 */
