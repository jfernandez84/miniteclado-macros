#include <Keyboard.h>
#include <Wire.h>
#include <U8g2lib.h>

const char* PASS_BIOS = "CLAVE_BIOS";
const char* PASS_WIN  = "CLAVE_WINDOWS";

const uint8_t PIN_A = 4, PIN_B = 5, PIN_SW = 6;
const uint8_t CHAR_DELAY = 20;   // ms entre teclas; sube si la BIOS pierde caracteres
const unsigned LARGO_MS = 600;   // pulsación larga
const unsigned DOBLE_MS = 300;   // ventana para el doble clic

U8G2_SSD1306_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

// Nunca se muestra una contraseña, solo el nombre de la macro.
const char* NOMBRES[] = {
  "Enter to BIOS (Del)",
  "BIOS Password",
  "Navegate BIOS",
  "Write 7 + Enter",
  "Esc",
  "F10 (save)",
  "fs1:",
  "startup.nsh",
  "Windows Password",
  "Info"
};
const uint8_t N = sizeof(NOMBRES) / sizeof(NOMBRES[0]);
const uint8_t ID_NAVEGAR = 2;
const uint8_t ID_INFO = N - 1;   // siempre el último de la lista
const uint8_t FILAS = 5;

enum Modo { MACROS, NAVEGAR, INFO };
enum Evento { NINGUNO, CLIC, DOBLE, LARGO };

Modo modo = MACROS;
uint8_t sel = 0;
bool ejeVertical = true;

// --- Botón: clic / doble clic / largo sin bloquear ---
bool btnEstado = false, btnLargo = false;
uint8_t btnClics = 0;
unsigned long btnT = 0;

void resetBoton() {
  btnEstado = false; btnLargo = false; btnClics = 0; btnT = millis();
}

Evento leerBoton() {
  unsigned long ahora = millis();
  bool down = digitalRead(PIN_SW) == LOW;
  if (down != btnEstado && ahora - btnT > 25) {   // antirrebote
    btnEstado = down;
    btnT = ahora;
    if (down)            btnLargo = false;
    else if (!btnLargo)  btnClics++;
    else                 btnClics = 0;
  }
  if (btnEstado && !btnLargo && ahora - btnT > LARGO_MS) {
    btnLargo = true;
    btnClics = 0;
    return LARGO;
  }
  if (!btnEstado && btnClics > 0 && ahora - btnT > DOBLE_MS) {
    Evento e = (btnClics >= 2) ? DOBLE : CLIC;
    btnClics = 0;
    return e;
  }
  return NINGUNO;
}

// --- Pantallas ---
void dibujarMacros() {
  uint8_t top = 0;
  if (sel >= 2) top = sel - 2;
  if (top > N - FILAS) top = N - FILAS;
  u8g2.firstPage();
  do {
    u8g2.setFont(u8g2_font_6x12_tf);
    for (uint8_t f = 0; f < FILAS; f++) {
      uint8_t i = top + f;
      uint8_t y = f * 12;
      if (i == sel) {
        u8g2.setDrawColor(1);
        u8g2.drawBox(0, y, 128, 12);
        u8g2.setDrawColor(0);
      } else {
        u8g2.setDrawColor(1);
      }
      u8g2.drawStr(4, y + 10, NOMBRES[i]);
    }
    u8g2.setDrawColor(1);
  } while (u8g2.nextPage());
}

void dibujarNavegar() {
  u8g2.firstPage();
  do {
    u8g2.setFont(u8g2_font_6x12_tf);
    u8g2.setDrawColor(1);
    u8g2.drawBox(0, 0, 128, 12);
    u8g2.setDrawColor(0);
    u8g2.drawStr(4, 10, "NAVEGATE BIOS");
    u8g2.setDrawColor(1);
    u8g2.drawStr(4, 24, "Rotate:");
    u8g2.drawStr(40, 24, ejeVertical ? "UP/DOWN" : "LFT/RHT");
    u8g2.drawStr(4, 38, "1 clic: Enter");
    u8g2.drawStr(4, 50, "2 clics: change axis");
    u8g2.drawStr(4, 62, "Long: exit");
  } while (u8g2.nextPage());
}

void dibujarInfo() {
  u8g2.firstPage();
  do {
    u8g2.setFont(u8g2_font_6x12_tf);
    u8g2.setDrawColor(1);
    u8g2.drawBox(0, 0, 128, 12);
    u8g2.setDrawColor(0);
    u8g2.drawStr(4, 10, "Designed by");
    u8g2.setDrawColor(1);
    u8g2.drawStr(4, 24, "Jorge Fernandez");
    u8g2.drawStr(4, 38, "2026 - Room 3B");
    u8g2.drawStr(4, 50, "R.H Electronics");
    u8g2.drawStr(4, 62, "Lumenis");
  } while (u8g2.nextPage());
}

void pantallaEnviando(const char* nombre) {
  u8g2.firstPage();
  do {
    u8g2.setFont(u8g2_font_7x14B_tf);
    u8g2.drawStr(4, 24, "Sending...");
    u8g2.setFont(u8g2_font_6x12_tf);
    u8g2.drawStr(4, 46, nombre);
  } while (u8g2.nextPage());
}

// --- Teclado ---
void escribir(const char* s, bool enter = true) {
  for (const char* p = s; *p; p++) {
    Keyboard.write(*p);
    delay(CHAR_DELAY);
  }
  if (enter) Keyboard.write(KEY_RETURN);
}

// Repite Delete durante ms; un clic del encoder lo cancela.
void spamDelete(unsigned long ms) {
  unsigned long fin = millis() + ms;
  while (millis() < fin && digitalRead(PIN_SW) == HIGH) {
    Keyboard.write(KEY_DELETE);
    delay(50);
  }
}

void ejecutar(uint8_t i) {
  pantallaEnviando(NOMBRES[i]);
  switch (i) {
    case 0: spamDelete(6000); break;
    case 1: escribir(PASS_BIOS); break;
    case 3: escribir("7"); break;
    case 4: Keyboard.write(KEY_ESC); delay(150); break;
    case 5: Keyboard.write(KEY_F10); delay(150); break;
    case 6: escribir("fs1:"); break;
    case 7: escribir("startup.nsh"); break;
    case 8: escribir(PASS_WIN); break;
  }
  while (digitalRead(PIN_SW) == LOW) delay(10);  // esperar a soltar
  delay(150);
  resetBoton();
  dibujarMacros();
}

// Decodificador de encoder por tabla de estados; devuelve -1, 0 o +1 por muesca.
int8_t leerEncoder() {
  static const int8_t TABLA[16] = {0,-1,1,0, 1,0,0,-1, -1,0,0,1, 0,1,-1,0};
  static uint8_t prev = 0;
  static int8_t acum = 0;
  prev = ((prev << 2) | (digitalRead(PIN_A) << 1) | digitalRead(PIN_B)) & 0x0F;
  acum += TABLA[prev];
  if (acum >= 4)  { acum = 0; return 1; }
  if (acum <= -4) { acum = 0; return -1; }
  return 0;
}

void setup() {
  pinMode(PIN_A, INPUT_PULLUP);
  pinMode(PIN_B, INPUT_PULLUP);
  pinMode(PIN_SW, INPUT_PULLUP);
  u8g2.begin();
  Keyboard.begin();
  dibujarMacros();
}

void loop() {
  int8_t d = leerEncoder();
  Evento e = leerBoton();

  if (modo == MACROS) {
    if (d != 0) {
      sel = (sel + N + d) % N;
      dibujarMacros();
    }
    if (e == CLIC || e == DOBLE) {
      if (sel == ID_NAVEGAR) {
        modo = NAVEGAR;
        ejeVertical = true;
        dibujarNavegar();
      } else if (sel == ID_INFO) {
        modo = INFO;
        dibujarInfo();
      } else {
        ejecutar(sel);
      }
    }
  } else if (modo == INFO) {
    if (e != NINGUNO) {  // cualquier clic o pulsación larga vuelve al menú
      modo = MACROS;
      dibujarMacros();
    }
  } else {  // NAVEGAR
    if (d != 0) {
      if (ejeVertical) Keyboard.write(d > 0 ? KEY_DOWN_ARROW : KEY_UP_ARROW);
      else             Keyboard.write(d > 0 ? KEY_RIGHT_ARROW : KEY_LEFT_ARROW);
    }
    if (e == CLIC) {
      Keyboard.write(KEY_RETURN);
    } else if (e == DOBLE) {
      ejeVertical = !ejeVertical;
      dibujarNavegar();
    } else if (e == LARGO) {
      modo = MACROS;
      dibujarMacros();
    }
  }
}