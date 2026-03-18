/* Consola de videojuegos en Arduino
Daniel de Lizaur
Última modificación: 3 de mayo de 2019*/
 
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
 
LiquidCrystal_I2C lcd(0x27,20,4);  //instancia el LCD de 20x4
//pines de los botones
const int A = 13;
const int B = 12;
const int ARRIBA = 11;
const int ABAJO = 10;
const int IZQUIERDA = 9;
const int DERECHA = 8;
const int JOYSTICK_X = 1;
const int JOYSTICK_Y = 0;
 
//Estados de los botones
bool permitirA = true;
bool Apulsado = false;
bool permitirB = true;
bool Bpulsado = false;
bool permitirArriba = true;
bool arribaPulsado = false;
bool permitirAbajo = true;
bool abajoPulsado = false;
bool permitirIzquierda = true;
bool izquierdaPulsada = false;
bool permitirDerecha = true;
bool derechaPulsada = false;
 
int frameDelay = 100; //milisegundos entre cada fotograma;
 
String estado = "Menu";
 
//FUNCIONES GLOBALES
 
void setup()
{
  lcd.init(); // inicializa la pantalla LCD
  lcd.backlight(); //Activa la retroiluminación
  pinMode(A, INPUT);
  pinMode(B, INPUT);
  pinMode(ARRIBA, INPUT);
  pinMode(ABAJO, INPUT);
  pinMode(IZQUIERDA, INPUT);
  pinMode(DERECHA, INPUT);
  Serial.begin(9600);
  menuPrincipal();
}
 
//temporizadores
int timer = 0;
int prevTime = 0;
 
//este bucle se ejecuta un número fijo de veces por segundo
bool buscaminasIniciado = false;
bool serpienteIniciada = false;
bool carrerasIniciado = false;
void fixedLoop() {
  if (estado.equals("Menu")) {
    bucleMenu();  
  } else if (estado.equals("Buscaminas")) {
    if (!buscaminasIniciado) {
      iniciarBuscaminas();
      buscaminasIniciado = true;
    }
    bucleBuscaminas();
  } else if (estado.equals("Serpiente")) {
    if (!serpienteIniciada) {
      iniciarSerpiente();
      serpienteIniciada = true;
    }
    bucleSerpiente();
  } else if (estado.equals("Carreras")) {
    if (!carrerasIniciado) {
      iniciarCarreras();
      carrerasIniciado = true;
    }
    bucleCarreras();
  } else if (estado.equals("Debug")) {
    debug();
  }
}
 
//bucle principal
void loop()
{
  timer += millis() - prevTime;
  if (timer > frameDelay) {
    fixedLoop();
    timer = 0;
    //restablece el estado de los botones
    Apulsado = false;
    Bpulsado = false;
    arribaPulsado = false;
    abajoPulsado = false;
    izquierdaPulsada = false;
    derechaPulsada = false;
  }
  prevTime = millis();
  comprobarBotones();
  if (Apulsado && Bpulsado) {
    estado = "Debug";
    debug();
  }
}
 
//actualiza el estado de todos los botones
void comprobarBotones() {
  if (digitalRead(A) && permitirA) {
    Apulsado = true;
    permitirA = false;  
  }
  if (!digitalRead(A)) {
    permitirA = true;
  } //A
  if (digitalRead(B) && permitirB) {
    Bpulsado = true;
    permitirB = false;  
  }
  if (!digitalRead(B)) {
    permitirB = true;
  } //Arriba
  if ((digitalRead(ARRIBA) || analogRead(JOYSTICK_Y) > 700) && permitirArriba) {
    arribaPulsado = true;
    permitirArriba = false;  
  }
  if (!(digitalRead(ARRIBA) || analogRead(JOYSTICK_Y) > 700)) {
    permitirArriba = true;
  } //Abajo
  if ((digitalRead(ABAJO) || analogRead(JOYSTICK_Y) < 300) && permitirAbajo) {
    abajoPulsado = true;
    permitirAbajo = false;  
  }
  if (!(digitalRead(ABAJO) || analogRead(JOYSTICK_Y) < 300)) {
    permitirAbajo = true;
  } //Izquierda
  if ((digitalRead(IZQUIERDA) || analogRead(JOYSTICK_X) < 300) && permitirIzquierda) {
    izquierdaPulsada = true;
    permitirIzquierda = false;  
  }
  if (!(digitalRead(IZQUIERDA) || analogRead(JOYSTICK_X) > 300)) {
    permitirIzquierda = true;
  } //Derecha
  if ((digitalRead(DERECHA) || analogRead(JOYSTICK_X) > 700) && permitirDerecha) {
    derechaPulsada = true;
    permitirDerecha = false;  
  }
  if (!(digitalRead(DERECHA) || analogRead(JOYSTICK_X) > 700)) {
    permitirDerecha = true;
  }
}
 
//Menu principal
void menuPrincipal() {
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("^. Buscaminas");
  lcd.setCursor(0,1);
  lcd.print("v. Serpiente");
  lcd.setCursor(0,2);
  lcd.print("<. Asteroides");/*
  lcd.setCursor(0,3);
  lcd.print(">. Simon"); */
}
 
void bucleMenu() {
  if (arribaPulsado) {
    estado = "Buscaminas";
  } if (abajoPulsado) {
    estado = "Serpiente";
  } if (izquierdaPulsada) {
    estado = "Carreras";
  }
}
 
void debug() {
  lcd.setCursor(0,0);
  lcd.print(analogRead(JOYSTICK_X));
  lcd.setCursor(0,1);
  lcd.print(analogRead(JOYSTICK_Y));
  lcd.setCursor(0,2);
  lcd.print(digitalRead(12));
  lcd.setCursor(0,3);
  lcd.print(digitalRead(13));
}
 
void volverAlMenu() {
  delay(2000);
  lcd.clear();
  menuPrincipal();
  buscaminasIniciado = false;
  serpienteIniciada = false;
  carrerasIniciado = false;
  estado = "Menu";
}
 
//////////////BUSCAMINAS/////////////////
 
int minas[20][4]; //guarda la situación de las minas y el número de minas alrededor de las casillas vacías
bool despejado[20][4];
bool banderas[20][4];
const int MINA = 100;
int cursorX;
int cursorY;
bool victoria = false;
const int numeroDeMinas = 10;
 
void colocarMinas(int numeroDeMinas) {
  int minasColocadas = 0;
  while (minasColocadas < numeroDeMinas) {
    int minaX = random(0, 20);
    int minaY = random(0, 4);
    if (minas[minaX][minaY] != MINA) { //Si no hay una mina en ese espacio
       minas[minaX][minaY] = MINA;
       minasColocadas++;
    }
  }
}
 
void calcularNumeros() { //cuenta el número de minas alrededor de cada casilla vacía y lo guarda en minas
  for (int x = 0; x < 20; x++) {
    for (int y = 0; y < 4; y++) {
      int vecinos = 0;
      if (minas[x][y] != MINA) {
         if (minas[x+1][y] == MINA && x != 19) {
          vecinos++;
         }
         if (minas[x-1][y] == MINA && x != 0) {
          vecinos++;
         }
         if (minas[x][y+1] == MINA && y != 3) {
          vecinos++;
         }
         if (minas[x][y-1] == MINA && y != 0) {
          vecinos++;
         }
         if (minas[x+1][y+1] == MINA && !(x == 19 || y == 3)) {
          vecinos++;
         }
         if (minas[x-1][y+1] == MINA && !(x == 0 || y == 3)) {
          vecinos++;
         }
         if (minas[x+1][y-1] == MINA && !(x == 19 || y == 0)) {
          vecinos++;
         }
         if (minas[x-1][y-1] == MINA && !(x == 0 || y == 0)) {
          vecinos++;
         }
         minas[x][y] = vecinos;
      }
    }
  }
}
 
void imprimirMinas() { //cuando el jugador pierde, mostrar todas las minas
  //lcd.print("Imprimiendo");
  for (int x = 0; x < 20; x++) {
    for (int y = 0; y < 4; y++) {
      lcd.setCursor(x,y);
      if (minas[x][y] == MINA) {
        lcd.print("X");  
      } else {
        lcd.print(minas[x][y]);
      }
    }
  }
}
 
bool comprobarVictoria() {
  int despejados = 0;
  int _banderas = 0;
  for (int x = 0; x < 20; x++) {
    for (int y = 0; y < 4; y++) {
      if (banderas[x][y] && minas[x][y] == MINA) {
        _banderas++;
      } else if (despejado[x][y]){
        despejados++;
      }
    }
  }
  return (despejados+_banderas) == 80; //Numero de casillas total
}
 
void despejar(int x, int y) { //Algoritmo floodfill, para despejar las celdas automáticamente
 lcd.setCursor(x, y);
 if (minas[x][y] != MINA) {
  lcd.print(minas[x][y]);
 }
 despejado[x][y] = true;
 if (minas[x][y] != MINA && minas[x][y] == 0) {
  for (int xoff = -1; xoff <= 1; xoff++) {
    for (int yoff = -1; yoff <= 1; yoff++) {
      if (x+xoff > -1 && x+xoff < 20 && y+yoff > -1 && y+yoff < 4) {
        if (minas[x+xoff][y+yoff] != MINA && !despejado[x+xoff][y+yoff] && !banderas[x+xoff][y+yoff]) {
          despejar(x+xoff, y+yoff);
        }
      }
    }
  }
 }
}
 
void iniciarBuscaminas() {
  lcd.clear();
  lcd.cursor();
  randomSeed(millis());
  colocarMinas(numeroDeMinas);
  calcularNumeros();
  //imprimirMinas();
}
 
void bucleBuscaminas() {
  //movimiento del cursor
  if (arribaPulsado && cursorY != 0) {
    cursorY--;
    lcd.setCursor(cursorX, cursorY);
  }
  if (abajoPulsado && cursorY != 3) {
    cursorY++;
    lcd.setCursor(cursorX, cursorY);
  }
  if (izquierdaPulsada && cursorX != 0) {
    cursorX--;
    lcd.setCursor(cursorX, cursorY);
  }
  if (derechaPulsada && cursorX != 19) {
    cursorX++;
    lcd.setCursor(cursorX, cursorY);
  }
 
  if (Apulsado) { //despejar casillla
    if (minas[cursorX][cursorY] == MINA) { //Derrota
      imprimirMinas();
      delay(1000);
      lcd.clear();
      lcd.setCursor(0, 1);
      lcd.print("Has perdido");
      volverAlMenu();
    } else {
      despejar(cursorX, cursorY);
      lcd.setCursor(cursorX, cursorY);
    }
  }
  if (Bpulsado) { //colocar/quitar banderas
    if (banderas[cursorX][cursorY]) {
      banderas[cursorX][cursorY] = false;
      lcd.print(" ");
      lcd.setCursor(cursorX, cursorY);
    } else {
      banderas[cursorX][cursorY] = true;
      lcd.print("P");
      lcd.setCursor(cursorX, cursorY);
    }
  }
  if (Apulsado || Bpulsado || arribaPulsado || abajoPulsado || izquierdaPulsada || derechaPulsada) {
    victoria = comprobarVictoria();
  }
  if (victoria) {
    lcd.clear();
    lcd.print("Has ganado!");
    volverAlMenu();
  }
}
 
/////////SERPIENTE///////////////
const int arriba = 0;
const int abajo = 1;
const int izquierda = 2;
const int derecha = 3;
int direccion = derecha;
int posX = 0;
int posY = 0;
int comidaX;
int comidaY;
int longitud = 1;
int colaX[80];
int colaY[80];
 
void calcularPosicion() {
  if (direccion == arriba) {
    posY--;
    if (posY < 0) {
      posY = 3;  
    }
  } else if (direccion == abajo) {
    posY++;
    if (posY > 3) {
      posY = 0;
    }
  } else if (direccion == izquierda) {
    posX--;
    if (posX < 0) {
      posX = 19;
    }
  } else if (direccion == derecha) {
    posX++;
    if (posX > 19) {
      posX = 0;
    }
  }
}
 
void dibujarSerpiente() {
  lcd.clear();
  lcd.setCursor(comidaX, comidaY);
  lcd.print("X");
  for (int i = 0; i <= longitud-1; i++) {
    lcd.setCursor(colaX[i], colaY[i]);
    lcd.print("O");
  }
}
 
void iniciarSerpiente() {
  lcd.clear();
  lcd.noCursor();
  frameDelay = 300;
  colocarComida();
}
 
void serpienteGameOver() {
  lcd.clear();
  lcd.setCursor(0,1);
  lcd.print("GAME OVER");
  lcd.setCursor(0,2);
  lcd.print("Puntos: ");
  lcd.print(longitud);
  volverAlMenu();
}
 
void colocarComida() {
  randomSeed(millis());
  comidaX = random(0,19);
  comidaY = random(0,3);
}
 
void cambiarDireccionSerpiente() {
  if (arribaPulsado) {
    direccion = arriba;
  } else if (abajoPulsado) {
    direccion = abajo;
  } else if (izquierdaPulsada) {
    direccion = izquierda;
  } else if (derechaPulsada) {
    direccion = derecha;
  }
}
 
bool comprobarDerrota() {
  for (int i = 1; i <= longitud-1; i++) {
    if ((colaX[i] == posX && colaY[i] == posY) && i!=0) {
      return true;
    }
  }
  return false;
}
 
void bucleSerpiente() {
  cambiarDireccionSerpiente();
  calcularPosicion();
  if (comprobarDerrota()) {
    serpienteGameOver();
  }
  for (int i = 0; i <= longitud-1; i++) {
    colaX[i] = colaX[i+1];
    colaY[i] = colaY[i+1];
  }
  colaX[longitud-1] = posX;
  colaY[longitud-1] = posY;
  //si la serpiente retrocede
  if ((direccion == abajo && arribaPulsado) || (direccion == arriba && abajoPulsado) || (direccion == derecha && izquierdaPulsada) || (direccion == izquierda && derechaPulsada)) {
    serpienteGameOver();
  }
  if (posX == comidaX && posY == comidaY) { //si come
    longitud++;
    colaX[longitud-1] = posX;
    colaY[longitud-1] = posY;
    colocarComida();
  }
  dibujarSerpiente();
}
 
void imprimirArray(int lista[]) {
  for (int i = 0; i < 19; i++) {
    Serial.print(lista[i]);
    Serial.print(", ");
  }
  Serial.println("");
}
 
////////////////////ASTERIOIDES////////////////////////
#include <ListLib.h> //librería de listas
 
List<int> enemigosX(10);
List<int> enemigosY(10);
List<int> disparosX(20);
List<int> disparosY(20);
 
int tiempoEnemigo = 2; //llamadas a bucleCarreras restantes hasta generar otro meteorito
int puntos = 0;
int cooldown = 7; // llamadas a bucleCarreras entre disparos
int vidas = 5;
 
int jugadorY = 0;
 
void iniciarCarreras() {
  lcd.clear();
  randomSeed(millis());
  byte JUGADOR[8] {
    B11000,
    B10100,
    B10010,
    B10001,
    B10010,
    B10100,
    B11000,
  };
  lcd.createChar(0, JUGADOR);
  frameDelay = 200; // cambiar los FPS a 5
  }
 
void bucleCarreras() {
  //movimiento del jugador
  if (arribaPulsado && jugadorY != 0) {
    jugadorY--;
  }
  if (abajoPulsado && jugadorY != 3) {
    jugadorY++;
  }
  //disparos
  cooldown--;
  if (Apulsado && cooldown <= 0) {
    disparar();
    cooldown = 2;
  }
  //generar enemigos
  tiempoEnemigo--;
  if (tiempoEnemigo < 0) {
    generarMeteorito();
    tiempoEnemigo = 3;
  }
  if (vidas == 0) {
    gameOverCarreras();
  }
  actualizarDisparos();
  comprobarColisiones();
  actualizarMeteoritos();
  dibujarCarreras();
}
 
void disparar() {
  disparosX.Add(0);
  disparosY.Add(jugadorY);
}
 
void generarMeteorito() {
  enemigosX.Add(19);
  enemigosY.Add(random(0,4));
}
 
void actualizarDisparos() {
  for (int i = 0; i < disparosX.Count(); i++) {
    disparosX[i]++;
    if (disparosX[i] > 19) {
      disparosX.Remove(i);
      disparosY.Remove(i);
    }
  }
}
 
void comprobarColisiones() {
  //colisión entre disparo y enemigo
  for (int i = 0; i < enemigosX.Count(); i++) {
    for (int j = 0; j < disparosX.Count(); j++) {
      if ((enemigosX[i] == disparosX[j] && enemigosY[i] == disparosY[j]) || (enemigosX[i] == disparosX[j] -1 && enemigosY[i] == disparosY[j])) {
        enemigosX.Remove(i);
        enemigosY.Remove(i);
        disparosX.Remove(j);
        disparosY.Remove(j);
        puntos++;
      }
    }
  }
  //colsision entre enemigo y jugador
  for (int i = 0; i < enemigosX.Count(); i++) {
    if (enemigosX[i] == 0 && enemigosY[i] == jugadorY) {
      gameOverCarreras();
    }
  }
}
 
void dibujarCarreras() {
  lcd.clear();
  lcd.setCursor(0, jugadorY);
  lcd.write(byte(0));
  for (int i = 0; i < disparosX.Count(); i++) {
    lcd.setCursor(disparosX[i], disparosY[i]);
    lcd.print("-");
  }
  for (int i = 0; i < enemigosX.Count(); i++) {
    lcd.setCursor(enemigosX[i], enemigosY[i]);
    lcd.print("O");
  }
  lcd.setCursor(19,0);
  lcd.print(vidas);
}
 
void actualizarMeteoritos() {
  for (int i = 0; i < enemigosX.Count(); i++) {
    enemigosX[i]--;
    if (enemigosX[i] < 0) {
      vidas--;
      enemigosX.Remove(i);
      enemigosY.Remove(i);
    }
  }
}
 
void gameOverCarreras() {
  lcd.clear();
  lcd.setCursor(0,1);
  lcd.print("Has perdido");
  lcd.setCursor(0,2);
  lcd.print("Puntos: ");
  lcd.print(puntos);
  volverAlMenu();
}
