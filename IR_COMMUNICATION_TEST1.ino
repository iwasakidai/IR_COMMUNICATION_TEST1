// https://qiita.com/pontaman007/items/97f00a6eba5d0b649f91
// (Arduino)ESP32モジュールを用いた赤外線リモコン受信プログラム
/*  作成日；2020/3/4
    ファイル名；02_IR_COMMUNICATION_TEST1
    プログラム内容；赤外線リモコンからの情報をシリアルモニターへ出力する
    　　　　　　　　出力内容；メーカーコード、受信コード、総ビット数、オンオフ時間
    使用部品；ESP32、IRM-3638
    ESP32へのピン接続；(17:IRM_Vout)
 */

//各ライブラリ
#include <Arduino.h>
#include <IRrecv.h>
#include <IRremoteESP8266.h>
#include <IRac.h>
#include <IRtext.h>
#include <IRutils.h>

const uint16_t kRecvPin = 22; //赤外線受信ピン
const uint16_t kCaptureBufferSize = 1024; //受信データ格納バッファ数(赤外線ON/OFFの時間を格納)

String maker_code = ""; //メーカーコード挿入変数
String recv_code = ""; //受信コード挿入変数
uint16_t recv_bits; //受信コード総ビット数挿入変数
uint16_t recv_onoff_bits; //ONOFF回数挿入変数
uint16_t recv_state_bits; //A/C受信コード挿入変数
uint32_t recv_address; //受信アドレス挿入変数
uint32_t recv_command; //受信コマンド挿入変数
uint64_t recv_data; //受信データ挿入変数

//kTimeout = 赤外線データの終了を判断するmsの値(kTimeoutミリ秒間受信がなければ終了と判断する)
#if DECODE_AC //エアコンの赤外線リモコンの場合
const uint8_t kTimeout = 50;  //50ms
#else   //その他の赤外線リモコン機器の場合
const uint8_t kTimeout = 15;  //15ms
#endif

const uint16_t kMinUnknownSize = 12;  //不明な赤外線受信をカットするための変数

IRrecv irrecv(kRecvPin, kCaptureBufferSize, kTimeout, true);  //IRrecv(赤外線受信ピン、受信データ格納バッファサイズ、データ終了判断時間、値保存機能(true推奨))
decode_results results;  //受信結果を保存するクラスのインスタンスを生成


/*  初期設定  */
void setup() {
  
  Serial.begin(115200); //シリアル通信レート
  
#if DECODE_HASH
  irrecv.setUnknownThreshold(kMinUnknownSize);  //不明な赤外線受信データは除外する
#endif

  irrecv.enableIRIn();  //赤外線受信開始
  
}
/*  初期設定  */


/*  メイン本文  */
void loop() {

  if (irrecv.decode(&results)) {  //赤外線受信があれば

    maker_code = typeToString(results.decode_type,false); //メーカーコード取得
    recv_code = resultToHexidecimal(&results);  //赤外線受信データを16進数で取得
    recv_bits = results.bits; //受信データの総Bit数を取得
    recv_onoff_bits = getCorrectedRawLength(&results);  //受信データのON/OFF切り替え回数を取得

    Serial.println("Maker : " + maker_code); //メーカーコード出力
    Serial.println("Code : " + recv_code);  //赤外線受信データを16進数で表示
    Serial.println("Bits : " + String(recv_bits)); //受信データの総Bit数を表示

    Serial.print("ON/OFF Data : rawData[" + String(recv_onoff_bits) + "] = {");  //受信データのON/OFF切り替え回数を表示
    for(int i=1; i < recv_onoff_bits + 1; i++){ //受信データのON/OFF切り替え時間を表示
      Serial.print(String(results.rawbuf[i] * kRawTick) + ", ");
    }
    Serial.println("}");

    if (hasACState(results.decode_type)) {  //メーカーコードがACだった場合

      recv_state_bits = recv_bits / 8;
      Serial.print("State Data : state[" + String(recv_state_bits) + "] = {");  //赤外線受信データの分割ビット数を表示
      for(int i = 0; i < recv_state_bits; i++) {  //赤外線受信データの分割したものを16進数で表示
        Serial.print("0x");
        if (results.state[i] < 0x10) {
          Serial.print("0");
        }
        Serial.print(uint64ToString(results.state[i], 16));
        if (i < recv_state_bits - 1) {
          Serial.print(kCommaSpaceStr);
        }
      }
      Serial.println("}");
      
    }else{  //メーカーコードがAC以外だった場合
      
      if (results.address > 0 || results.command > 0) { //アドレスとコマンドを16進数で表示

        recv_address = results.address; //アドレスを取得
        recv_command = results.command; //コマンドを取得
        
        Serial.println("Address : 0x" + uint64ToString(recv_address, 16));
        Serial.println("Command : 0x" + uint64ToString(recv_command, 16));

      }

      recv_data = results.value;  //データを取得

      //データを16進数で表示
      Serial.print("Data : 0x");
      serialPrintUint64(recv_data, HEX);
      Serial.println("");
      
    }
    
    Serial.println();

  }

}
/*  メイン本文  */
