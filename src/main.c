#include <xc.h>

#include "../include/init.h"
#include "../include/common.h"

#define FLASH  PORTCbits.RC2
#define SWITCH PORTBbits.RB4
#define SIGNAL PORTBbits.RB3
#define _TMR0 84

unsigned int seconds           = _mmss(15, 45);
unsigned char is_running       = 0,
              is_init_state    = 1,
              f_display_switch = 0b0001;


/** ANn ポートからA/D変換した値を取得
 * > unsigned char ポート番号(AN0: 0, AN1: 1...)
 * < 0 ~ 1023 の 10bit データ
 */
// ``` unsigned int get_analog_value(unsigned char)
unsigned int
get_analog_value(unsigned char port) {
  unsigned int val = 0;

  ANSEL  = (1 << port);
  ADCON0 = 0b01000001 | ((port & 0b1111) << 2);
  ADCON1 = 0b10000000;
  __delay_us(20);

  ADCON0bits.GO_DONE = 1;

  while(ADCON0bits.GO_DONE);
  val = (ADRESH << 8) | ADRESL;
  PIR1bits.ADIF = 0;

  return val;
} // '''


/* # 割り込み関数
 * > void
 * < void
 */
// ``` void interrupt isr(void)
void interrupt isr(void) {
  INTCONbits.GIE = 0;

  // ダイナミック点灯
  // タイマー0 を使用して7seg の表示切り替え
  if(INTCONbits.T0IF) {
    unsigned char _min = seconds / 60,
                  _sec = seconds % 60;

    switch(f_display_switch) {
      case 0b0001:
        display_7seg(_sec % 10);
        break;

      case 0b0010:
        display_7seg(_sec / 10);
        break;

      case 0b0100:
        display_7seg(_min % 10);
        break;

      case 0b1000:
        display_7seg(_min / 10);
        break;
    }
    PORTC = (PORTC & 0b00001111) | (f_display_switch << 4);

    if(f_display_switch != 0b1000) f_display_switch <<= 1;
    else f_display_switch = 0b0001;

    TMR0 = _TMR0;
    INTCONbits.T0IF = 0b00;
  } else if(PIR1bits.TMR1IF) { // カウント
    if(is_running == 1) {
      FLASH ^= 1;

      if(--seconds == 0) { // カウントダウン終了
        is_running    = 0;
        is_init_state = 1;

        SIGNAL = 0;
        // NOTE: 音でも鳴らす?
        //PIE1bits.TMR1IE = 0b0;
      }
    } else {
      FLASH = 0;
    }

    TMR1L = 0x00;
    TMR1H = 0x80;
    PIR1bits.TMR1IF = 0b0;
  } else if(INTCONbits.RBIF) { // スイッチによるボタン処理
    if(SWITCH == 0) {
      // チャタリング防止は回路上で実装
      // NOTE: 5[ms] 遅延処理を行ったほうが安定するが,
      // 場合によっては7セグLED にチラツキが見られる.
      __delay_ms(5);

      if(is_running == 0) { // 開始
        SIGNAL        = 1;
        is_running    = 1;
        is_init_state = 0;
      } else { // 一時停止
        // NOTE: 一時停止中の出力は止めない
        //SIGNAL     = 0;
        is_running = 0;
      }
    }

    INTCONbits.RBIF = 0b0;
  }

  INTCONbits.GIE = 0b1;
} // '''


void
main(void) {
  TRISA  = 0b00000011;
  TRISB  = 0b00000000;
  TRISC  = 0b00000000;
  TRISD  = 0b00000000;

  PORTA  = 0b00000000;
  PORTB  = 0b00000000;
  PORTC  = 0b00000000;
  PORTD  = 0b00000000;

  OSCCON     = 0b01110101;
  //OSCTUNE    = 0b00001111;

  SSPCON = 0b00000000;

  // 割り込み
  INTCON = 0b11101000;
  PIE1   = 0b00000001;
  PIE2   = 0b00000000;

  // Timer0
  TMR0 = _TMR0;
  //OPTION_REG = 0b10000011;
  OPTION_REG = 0b00000011;

  // Timer1
  T1CON  = 0b10001011;
  TMR1L  = 0x00;
  TMR1H  = 0x80;

  // PORTB割り込み設定
  IOCB = 0b00000000;
  //WPUB = 0b11111111;

  OPTION_REGbits.nRBPU = 1; //
  WPUBbits.WPUB4       = 0; // RB4 プルアップ無効
  TRISBbits.TRISB4     = 1;
  IOCBbits.IOCB4       = 1;
  INTCONbits.RBIE      = 1;

  // アナログ入力
  ADCON0  = 0b01000000;
  ADCON1  = 0b10000000; // TODO: リファレンス電圧(3.3V想定)決定後, 変更
  ANSEL   = 0b00000011;
  ANSELH  = 0b00000000;
  CM1CON0 = 0b00000000;
  CM2CON0 = 0b00000000;
  CM2CON1 = 0b00000000;

  unsigned int an[2] = {0, 0};
  while(1) {
    if(is_running == 0 && is_init_state == 1) {
      // アナログ入力から設定時間を取得する.
#if 0
      an[0] = (int)((float)get_analog_value(AN0) * (60.0 / 1023.0)); // 分
      an[1] = (int)((float)get_analog_value(AN1) * (60.0 / 1023.0)); // 秒
#else
      an[0] = (int)((float)get_analog_value(AN0) * (59.0 / 1023.0)); // 分
      an[1] = (int)((float)get_analog_value(AN1) * (59.0 / 1023.0)); // 秒
#endif

      // 秒数変換
      seconds = _mmss(an[0], an[1]);
    }
    NOP();
  }
}
