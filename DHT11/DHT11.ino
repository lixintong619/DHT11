#include <DHT.h>

// ----- 引脚定义 -----
#define DHTPIN 2         // DHT11 数据引脚接 D2
#define DHTTYPE DHT11    // DHT11 传感器
#define MQ2_PIN A0       // MQ2 信号引脚 (S) 接 A0
#define BUZZER_PIN 3     // 蜂鸣器控制引脚接 D3

// 3个独立 LED 引脚定义
#define LED_YELLOW 5     // 黄灯/蓝灯（预警）接 D5
#define LED_GREEN  6     // 绿灯（安全）接 D6
#define LED_RED    9     // 红灯（高警）接 D9

// ----- 报警阈值设置 -----
#define TEMP_HIGH_LIMIT 32.0  // 温度预警/报警上限 (℃)
#define MQ2_ALERT_LIMIT 300   // MQ2 气体浓度高警阈值 (模拟量 0-1023)

DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(9600);
  Serial.println(F("--- 智能室内环境监测系统（独立LED版）启动 ---"));

  dht.begin();
  
  // 初始化输出引脚
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_RED, OUTPUT);

  // 初始化状态：全关
  digitalWrite(BUZZER_PIN, LOW);
  setLEDs(LOW, LOW, LOW);
}

void loop() {
  // 1. 读取传感器数据
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();
  int mq2Value = analogRead(MQ2_PIN);

  // 检查 DHT 读取错误
  if (isnan(humidity) || isnan(temperature)) {
    Serial.println(F("[错误] 无法读取 DHT11，请检查 DHT11 Pin 2 与 10K 上拉电阻！"));
    delay(2000);
    return;
  }

  // 2. 串口打印监视数据
  Serial.print(F("温度: "));
  Serial.print(temperature);
  Serial.print(F(" °C | 湿度: "));
  Serial.print(humidity);
  Serial.print(F(" % | 气体浓度: "));
  Serial.println(mq2Value);

  // 3. 多级报警逻辑控制
  bool isGasAlert  = (mq2Value > MQ2_ALERT_LIMIT);     // 气体超标 (严重)
  bool isTempAlert = (temperature > TEMP_HIGH_LIMIT); // 温度过高 (预警)

  if (isGasAlert) {
    // 【最高警报】：气体超标！响蜂鸣器 + 亮红灯
    digitalWrite(BUZZER_PIN, HIGH);
    setLEDs(LOW, LOW, HIGH); // 仅亮红灯
    Serial.println(F(" -> [危险] 检测到烟雾/危险气体！蜂鸣器报警！"));

  } else if (isTempAlert) {
    // 【次级预警】：仅温度偏高！不响蜂鸣器 + 亮黄灯
    digitalWrite(BUZZER_PIN, LOW);
    setLEDs(LOW, HIGH, LOW); // 仅亮黄灯/蓝灯
    Serial.println(F(" -> [注意] 环境温度偏高！"));

  } else {
    // 【正常状态】：蜂鸣器关 + 亮绿灯
    digitalWrite(BUZZER_PIN, LOW);
    setLEDs(HIGH, LOW, LOW); // 仅亮绿灯
  }

  delay(2000); // 每 2 秒刷新一次
}

// 辅助函数：快速控制 3 个 LED 的亮灭 (绿, 黄, 红)
void setLEDs(uint8_t green, uint8_t yellow, uint8_t red) {
  digitalWrite(LED_GREEN, green);
  digitalWrite(LED_YELLOW, yellow);
  digitalWrite(LED_RED, red);
}