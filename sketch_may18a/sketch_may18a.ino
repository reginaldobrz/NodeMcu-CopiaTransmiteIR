#include <Arduino.h>
#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRsend.h>
#include <IRutils.h>

const uint16_t RECV_PIN = D5; // VS1838B OUT
const uint16_t SEND_PIN = D2; // LED IR emissor

const uint16_t BTN_CAPTURE = D6;
const uint16_t BTN_SEND = D7;

const uint16_t MAX_PULSES = 300;

IRrecv irrecv(RECV_PIN, 1024, 50, true);
IRsend irsend(SEND_PIN);

decode_results results;

uint16_t rawData[MAX_PULSES];
uint16_t rawLen = 0;

uint16_t frequency = 38;

void setup() {
  Serial.begin(115200);

  pinMode(BTN_CAPTURE, INPUT_PULLUP);
  pinMode(BTN_SEND, INPUT_PULLUP);

  irrecv.enableIRIn();
  irsend.begin();

  Serial.println("Sistema iniciado.");
  Serial.println("D6 = capturar sinal");
  Serial.println("D7 = enviar sinal");
}

void captureSignal() {
  Serial.println("Aguardando sinal IR...");

  rawLen = 0;

  unsigned long start = millis();

  while (millis() - start < 10000) {
    if (irrecv.decode(&results)) {
      rawLen = min((uint16_t)(results.rawlen - 1), MAX_PULSES);

      for (uint16_t i = 1; i < results.rawlen && i <= MAX_PULSES; i++) {
        rawData[i - 1] = results.rawbuf[i] * kRawTick;
      }

      Serial.println("Sinal capturado!");

      Serial.print("Protocolo: ");
      Serial.println(typeToString(results.decode_type));

      Serial.print("HEX: ");
      serialPrintUint64(results.value, HEX);
      Serial.println();

      Serial.print("Bits: ");
      Serial.println(results.bits);

      Serial.print("RAW LEN: ");
      Serial.println(rawLen);

      Serial.print("uint16_t rawData[] = {");
      for (uint16_t i = 0; i < rawLen; i++) {
        Serial.print(rawData[i]);
        if (i < rawLen - 1) Serial.print(", ");
      }
      Serial.println("};");

      irrecv.resume();
      return;
    }
  }

  Serial.println("Timeout: nenhum sinal detectado.");
}

void sendSignal() {
  if (rawLen == 0) {
    Serial.println("Nenhum sinal salvo.");
    return;
  }

  Serial.println("Enviando sinal...");

  irrecv.disableIRIn();

  for (int i = 0; i < 5; i++) {
    irsend.sendRaw(rawData, rawLen, frequency);

    Serial.print("Tentativa ");
    Serial.println(i + 1);

    delay(120);
  }

  irrecv.enableIRIn();

  Serial.println("Finalizado.");
}

void loop() {
  if (digitalRead(BTN_CAPTURE) == LOW) {
    delay(250);

    while (digitalRead(BTN_CAPTURE) == LOW) {
      delay(10);
    }

    captureSignal();
  }

  if (digitalRead(BTN_SEND) == LOW) {
    delay(250);

    while (digitalRead(BTN_SEND) == LOW) {
      delay(10);
    }

    sendSignal();
  }
}