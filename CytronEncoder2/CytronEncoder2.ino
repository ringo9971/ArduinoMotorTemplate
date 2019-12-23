// Arduino nano/uno用
// エンコーダーのパルスを2逓倍で読み取り, 
// FF+PI制御でモーターの速度制御をするサンプルプログラムです
// CytronMotorDriver ver:1.0.1

#include <CytronMotorDriver.h>

CytronMD ri_motor(PWM_PWM, 9, 6);
CytronMD le_motor(PWM_PWM, 11, 10);

// エンコーダーに使うPin番号
// ENC_R1とENC_L1を割り込みに使用しているので, 2番または3番ピンを使用してください
// 4逓倍で読み込みたいならDue等を使用し, プログラムを変更してください
const int ENC_L1 = 3;
const int ENC_L2 = 4;
const int ENC_R1 = 2;
const int ENC_R2 = 5;

// Pゲイン
// 左右の回転数が一定にならない場合は, 増加
// 回転数が不安定な場合は下げてください
const double Kp = 0.0;
// Iゲイン
// Pgain/(時定数)とするのが良いそうです
// 少しIゲインを入れると安定する
// やりすぎると不安定になる
const double Ki = 0.000;
// FFゲイン
// 初速が速い場合は増加
// 遅い場合は減少させてください
const double Kf = 0.0120;

// 左右の回転パルス数
int32_t ricnt = 0;
int32_t lecnt = 0;
// 左右の回転目標値
double ri_target = 0;
double le_target = 0;
// 偏差Ｅ
int32_t ri_err;
int32_t le_err;
//　積分項
int32_t ri_integral = 0;
int32_t le_integral = 0;

// Serialから読み込むデータ
byte serial_data = 0;

void setup() {
  // シリアル通信を開始する
  Serial.begin(19200);

  // エンコーダーに使うPin
  pinMode(ENC_R1, INPUT);
  pinMode(ENC_R2, INPUT);
  pinMode(ENC_L1, INPUT);
  pinMode(ENC_L2, INPUT);

  //　割り込み
  attachInterrupt(digitalPinToInterrupt(ENC_R1), ri_itr, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENC_L1), le_itr, CHANGE);

  // 安心
  delay(100);
}

// 例
void loop() {
  if(Serial.available() > 0){
    serial_data = Serial.read();
    brake();
  }

  switch(serial_data){
    case 0:
      brake();
      break;
    case 1:
      forward(250);
      break;
    case 2:
      backward(250);
      break;
    case 3:
      le_turn(100);
      break;
    case 4:
      ri_turn(100);
      break;
  }
}

void initial(){
  ricnt = ri_target;
  lecnt = le_target;
}

// 0~255のvel値に対応した速度で回転する
void forward(int vel){
  setMotor(vel, vel);
}
void backward(int vel){
  setMotor(-vel, -vel);
}
void ri_turn(int vel){
  setMotor(-vel, vel);
}
void le_turn(int vel){
  setMotor(vel, -vel);
}
void brake(){
  initial();
  setMotor(0, 0);
}

void setMotor(int ri_vel, int le_vel){
  // 目標値の更新
  ri_target += ri_vel*Kf;
  le_target += le_vel*Kf;
  // 偏差Ｅ
  ri_err = ricnt-ri_target;
  le_err = lecnt-le_target;
  //　積分項
  ri_integral += ri_err;
  le_integral += le_err;

  // FF+PI制御
  ri_vel = constrain(ri_vel-ri_err*Kp-ri_integral*Ki, -255, 255);
  le_vel = constrain(le_vel-le_err*Kp-le_integral*Ki, -255, 255);
  ri_motor.setSpeed(ri_vel);
  le_motor.setSpeed(le_vel);

  // 1kHz程度で回す
  delay(1);
}

// digitalRead(ENC_RR1)をPIND & _BV(2)
// の書き換えれば, digitalReadを高速化することができる
// このプログラムはどのピンにも対応できるように高速化はされていない
// 参考
// https://qiita.com/autumn-position/items/ac016c58190f77f66a13
// https://www.arduino.cc/en/Hacking/PinMapping168
void ri_itr(){
  ricnt -= ((digitalRead(ENC_R1) == digitalRead(ENC_R2)) ? 1: -1);
}
void le_itr(){
  lecnt += ((digitalRead(ENC_L1) == digitalRead(ENC_L2)) ? 1: -1);
}
