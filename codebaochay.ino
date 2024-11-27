#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>
#include <DHT.h>

// Cấu hình cho Firebase
#define FIREBASE_HOST "https://canbaobaochay-default-rtdb.firebaseio.com/"  // Thay bằng URL Firebase
#define FIREBASE_AUTH "d4t0S6PpHqKe66UutjPvhq6VdIF3yKMQjf77CD84"        // Thay bằng Secret Firebase
// Thông tin kết nối WiFi
#define WIFI_SSID "Eoi"                      // Tên WiFi của bạn
#define WIFI_PASSWORD "88888888"              // Mật khẩu WiFi của bạn

// Khai báo chân cảm biến và DHT
#define DHTPIN D4        // DHT11 được nối với chân D4 của ESP8266
#define DHTTYPE DHT11    // Loại cảm biến là DHT11

DHT dht(DHTPIN, DHTTYPE);

// Khai báo chân cho các cảm biến khác
#define MQ9PIN D2        // MQ9 nối với chân D2 của ESP (sử dụng chân digital thay vì analog)
#define FIRE_SENSOR_PIN D5  // Cảm biến lửa nối với chân D5
#define BUZZERPIN D6     // Còi nối với chân D6

FirebaseData firebaseData;
FirebaseConfig firebaseConfig;

void setup() {
  Serial.begin(115200);
  
  // Kết nối WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected.");

  // Cấu hình Firebase
  firebaseConfig.host = FIREBASE_HOST;
  firebaseConfig.signer.tokens.legacy_token = FIREBASE_AUTH;

  // Kết nối Firebase
  Firebase.begin(&firebaseConfig, nullptr);
  Firebase.reconnectWiFi(true);

  // Khởi tạo cảm biến
  dht.begin();
  pinMode(MQ9PIN, INPUT);
  pinMode(FIRE_SENSOR_PIN, INPUT);
  pinMode(BUZZERPIN, OUTPUT);
}

void loop() {
  // Đọc dữ liệu từ DHT11
  float nhietdo = dht.readTemperature();
  float doam = dht.readHumidity();
  
  // Đọc tín hiệu từ MQ9 (sử dụng digitalRead thay vì analogRead)
  int gasValue = digitalRead(MQ9PIN);
  
  // Đọc dữ liệu từ cảm biến lửa (1 khi không phát hiện lửa, 0 khi phát hiện lửa)
  int fireValue = digitalRead(FIRE_SENSOR_PIN);

  // Xác định trạng thái còi
  bool coiCanhBao = (gasValue == 0 || fireValue == 0);  // 1 là phát hiện lửa hoặc phát hiện khí gas

  if (coiCanhBao) {
    digitalWrite(BUZZERPIN, HIGH); // bat còi
  } else {
    digitalWrite(BUZZERPIN, LOW);  // tat còi
  }

  // Ghi dữ liệu lên Firebase
  if (Firebase.setFloat(firebaseData, "/Nhietdo", nhietdo)) {
    Serial.println("Ghi nhiệt độ thành công");
  } else {
    Serial.println("Ghi nhiệt độ thất bại: " + firebaseData.errorReason());
  }

  if (Firebase.setFloat(firebaseData, "/Doam", doam)) {
    Serial.println("Ghi độ ẩm thành công");
  } else {
    Serial.println("Ghi độ ẩm thất bại: " + firebaseData.errorReason());
  }

  if (Firebase.setInt(firebaseData, "/gas", gasValue)) {
    Serial.println("Ghi khí gas thành công");
  } else {
    Serial.println("Ghi khí gas thất bại: " + firebaseData.errorReason());
  }

  if (Firebase.setInt(firebaseData, "/cambienlua", fireValue)) {
    Serial.println("Ghi cảm biến lửa thành công");
  } else {
    Serial.println("Ghi cảm biến lửa thất bại: " + firebaseData.errorReason());
  }

  if (Firebase.setString(firebaseData, "/coi", coiCanhBao ? "Cảnh báo" : "Bình thường")) {
    Serial.println("Ghi trạng thái còi thành công");
  } else {
    Serial.println("Ghi trạng thái còi thất bại: " + firebaseData.errorReason());
  }

  // Hiển thị giá trị trên Serial Monitor
  Serial.print("Nhiet do: "); Serial.println(nhietdo);
  Serial.print("Do am: "); Serial.println(doam);
  Serial.print("Gas: "); Serial.println(gasValue);
  Serial.print("Lửa: "); Serial.println(fireValue);
  Serial.print("Coi: "); Serial.println(coiCanhBao ? "Cảnh báo" : "Bình thường");

  // Delay 3 giây trước khi đọc lại
  delay(3000);
}
