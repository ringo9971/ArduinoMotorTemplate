// Arduino Mega/Due用
// モーター3個のオムニホイール用
// エンコーダーのパルスを2逓倍で読み取り, 
// FF+PI制御でモーターの速度制御をするサンプルプログラムです
// CytronMotorDriver ver:1.0.1

#include <CytronMotorDriver.h>

CytronMD motorA(PWM_PWM, 9, 6);
CytronMD motorB(PWM_PWM, 11, 10);
CytronMD motorC(PWM_PWM, 13, 12);

// エンコーダーに使うPin番号
// ENC_A1とENC_B1とENC_C1を割り込みに使用しているので, 割り込み可能なピンに繋いでください
// Dueだと3つのモーターでも4逓倍で読み取れます
const int ENC_A1 = 2;
const int ENC_A2 = 4;
const int ENC_B1 = 3;
const int ENC_B2 = 5;
const int ENC_C1 = 20;
const int ENC_C2 = 21;

// Pゲイン
// 左右の回転数が一定にならない場合は, 増加
// 回転数が不安定な場合は下げてください
const double Kp = 0.5;
// Iゲイン
// Pgain/(時定数)とするのが良いそうです
// 少しIゲインを入れると安定する
// やりすぎると不安定になる
const double Ki = 0.000;
// FFゲイン
// 初速が速い場合は増加
// 遅い場合は減少させてください
const double Kf = 0.0100;

// 左右の回転パルス数
int32_t cnt[3] = {0, 0, 0};
// 左右の回転目標値
double target[3] = {0.0, 0.0, 0.0};
// 偏差
int32_t err[3];
// 積分値
int32_t integral[3] = {0, 0, 0};

byte serial_data = 0;

void setup() {
  // シリアル通信を開始する
  Serial.begin(115200);

  // エンコーダーに使うPin
  pinMode(ENC_A1, INPUT);
  pinMode(ENC_A2, INPUT);
  pinMode(ENC_B1, INPUT);
  pinMode(ENC_B2, INPUT);
  pinMode(ENC_C1, INPUT);
  pinMode(ENC_C2, INPUT);

  //　割り込み
  attachInterrupt(digitalPinToInterrupt(ENC_A1), itrA, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENC_B1), itrB, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENC_C1), itrC, CHANGE);

  // 安心
  delay(100);
}

// 例
void loop() {
  if(Serial.available() > 0){
    serial_data = Serial.read();
    brake()
  }

  switch(serial_data){
    case 0:
      brake();
      break;
    case 1:
      forward(255);
      break;
    case 2:
      backward(255);
      break;
    case 3:
      ri_turn(100);
      break;
    case 4:
      le_turn(100);
      break;
    default:
      brake();
  } 
}

// 0~255のvel値に対応した速度で回転する
void forward(int val){
  setMoter((0, (int)(0.87*val), (int)(-0.878*val)));
}
void backward(int val){
  setMoter((0, (int)(-0.87*val), (int)(0.878*val)));
}
void right(int val){
  setMoter((-val, (int)(0.5*val), (int)(0.5*val)));
}
void left(int val){
  setMoter((val, (int)(-0.5*val), (int)(-0.5*val)));
}
void ri_turn(int val){
  setMoter((val, val, val));
}
void le_turn(int val){
  setMoter((-val, -val, -val));
}
void brake(){
  setMoter((0, 0, 0));
}

void initial(){
  for(int i = 0; i < 3; i++){
    cnt[i] = target[i];
  }
}

void setMoter(int vel[]){
  for(int i = 0; i < 3; i++){
    // 目標値の更新
    target[i] += vel[i]*Kf;
    // 偏差Ｅ
    err[i] = cnt[i]-target[i];
    //　積分項
    integral[i] += err[i];
    // FF+PI制御
    vel[i] = constrain(vel[i]-err[i]*Kp-integral[i]*Ki, -255, 255);
  }

  motorA.setSpeed(vel[0]);
  motorB.setSpeed(vel[1]);
  motorC.setSpeed(vel[2]);

  // 1kHz程度で回す
  delay(1);
}

void itrA(){
  cnt[0] += ((digitalRead(ENC_A1) == digitalRead(ENC_A2)) ? 1: -1);
}
void itrB(){
  cnt[1] += ((digitalRead(ENC_B1) == digitalRead(ENC_B2)) ? 1: -1);
}
void itrC(){
  cnt[2] += ((digitalRead(ENC_C1) == digitalRead(ENC_C2)) ? 1: -1);
}

