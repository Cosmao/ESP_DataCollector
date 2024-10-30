# Systemspecifikation

## Arkitektur

Systemet är uppbyggt av följande komponenter:

- **ESP32**: En mikrokontroller som använder en DHT11-sensor för att läsa temperatur och luftfuktighet.
- **MQTT Broker**: Centraliserar datakommunikationen mellan ESP32 och resten av systemet. Körs på en säker port (8883) och använder mTLS för kryptering och autentisering.
- **MQTT Consumer (Python-skript)**: En applikation som prenumererar på relevanta MQTT-ämnen, tar emot sensorvärden och matar in dem i databasen.
- **InfluxDB**: Databas för tidsseriedata som lagrar mätvärdena. Exponeras endast på port 8086.
- **Grafana**: Visualiserar data från InfluxDB och tillhandahåller en användargränssnitt för att se historiska och realtidsdata. Exponeras endast på port 3000.

Alla komponenter utöver ESP32 körs i enskilda Docker-containrar på en AWS ECS-kluster och kommunicerar över localhost.

## Kommunikationsflöde

1. ESP32 samlar in temperatur- och luftfuktighetsdata från DHT11-sensorn.
2. Data skickas från ESP32 till MQTT Broker via mTLS.
3. MQTT Consumer lyssnar på inkommande data från MQTT Broker och skickar dessa vidare till InfluxDB.
4. Grafana hämtar data från InfluxDB och visualiserar det för användare.

### Använda protokoll
- **mTLS** för krypterad dataöverföring mellan ESP32 och MQTT Broker.
- **MQTT** för dataöverföring mellan ESP32 och övriga systemkomponenter.
- **HTTPS** för framtida FOTA (Firmware Over-The-Air) uppdateringar.
  
## Säkerhetsåtgärder som redan finns

1. **mTLS (Mutual TLS)**: Säkerställer att kommunikationen mellan ESP32 och MQTT Broker är krypterad och autentiserad.
2. **Begränsade åtkomsttoken**: Endast de nödvändiga rättigheterna ges till varje komponent, exempelvis mellan Grafana och InfluxDB.
3. **Containerberoenden och loggning**: Alla komponenter körs som containrar i en isolerad miljö med loggning.
4. **Exponerade portar**: Endast nödvändiga portar är öppna mot omvärlden (8883 för MQTT, 8086 för InfluxDB, och 3000 för Grafana).
5. FOTA-stöd: Systemet har stöd för FOTA och använder Github-lagring för firmware-uppdateringar via HTTPS med giltiga certifikat.

## CRA-krav

För att uppfylla Cyber Resilience Act (CRA) behövs följande åtgärder som just nu inte är implementerade för snabbare utveckling.

- **CN-verifiering** för MQTT-certifikat kommer att införas för att öka säkerheten. 
- **Signering av binärer**: För att verifiera att firmware är autentisk och inte manipulerad.
- **Secure Boot**: Förhindra att osignerad kod körs på ESP32 genom att bränna eFUSE-bitar för att aktivera säkra uppstartskontroller.
- Rollback: Systemet kommer att markera felaktig firmware som oanvändbar och återgå till en tidigare fungerande version.

## Förbättringsområden

- Förbättrad säkerhet i AWS-miljön, just nu är det väldigt simpelt.
- Ringbuffer: Spara mätta värden och skicka dom batchat för att spara på batteri.
- Deep sleep: Gå ner och sov emellan mätningar, just nu är tempot högt för att säkerställa att hela flödet fungerar.
