#include <avr/io.h>
#define F_CPU 16000000UL
#include <util/delay.h>
#include <avr/interrupt.h>
#include "kt_m128.h"

// 핀 매핑 및 기본 설정
#define TRIG 6     // 초음파 센서 Trig 핀 (PE6)
#define ECHO 7     // 초음파 센서 Echo 핀 (PE7)
#define SOUND_VELOCITY 340UL // 소리 속도 (단위: m/s)

// DC 모터 속도 제어 함수
void dc_motor_control(int speed) {
    /*
    두 DC 모터의 방향과 속도를 제어하는 함수
    speed: PWM duty 비율 설정으로 모터 속도 조정
    */

    // 모터 1과 2의 정회전 설정
    PORTD |= (1 << PD2);  // 모터 1 IN1 = HIGH
    PORTD &= ~(1 << PD1); // 모터 1 IN2 = LOW
    PORTD |= (1 << PD7);  // 모터 2 IN3 = HIGH
    PORTD &= ~(1 << PD6); // 모터 2 IN4 = LOW

    // PWM을 이용한 모터 속도 조절
    for (int i = 0; i < 100; i++) {
        PORTD |= (1 << PD0);  // 모터 1 ENABLE = HIGH
        PORTD |= (1 << PD5);  // 모터 2 ENABLE = HIGH
        _delay_us(speed);     // duty 비율에 따라 속도 조절
        PORTD &= ~(1 << PD0); // 모터 1 ENABLE = LOW
        PORTD &= ~(1 << PD5); // 모터 2 ENABLE = LOW
        _delay_us(20000 - speed);  // 주기 조절을 통한 속도 조절
    }
}
// 긴 경고음을 발생 시키는 부저 함수 - distance < 150의 경우
void buzz1(void) {
    for (int i = 0; i < 50; i++) {
        PORTB = 0x10; // 버저 ON
        _delay_ms(1);
        PORTB = 0x00; // 버저 OFF
        _delay_ms(1);
    }
}
// 짧은 경고음을 발생 시키는 부저 함수 - distance < 450의 경우
void buzz2(void) {
    for (int i = 0; i < 10; i++) {
        PORTB = 0x10; // 버저 ON
        _delay_ms(1);
        PORTB = 0x00; // 버저 OFF
        _delay_ms(1);
    }
}

int main(void) {             //초음파 센서를 사용하여 장애물 감지 및 DC 모터,LED,부저 제어
    init_kt_m128();
    unsigned int distance;
    int i;
    // MCU 및 USART 초기화 설정
    MCUCR = 0x00;
    XMCRB = 0x00;
    // USART 설정: 비동기 통신, 8비트 데이터, 1비트 정지, 115200bps
    UCSR1A = 0x00;
    UCSR1B = 0x18;
    UCSR1C = 0x06;
    UBRR1H = 0x00;
    UBRR1L = 8;

    // 출력 핀 초기화(PB핀 - 부저, PE핀 - 초음파센서, PC핀 - LED, PD핀 - 모터)
    DDRB = 0x10; // 부저 출력
    DDRE = ((DDRE | (1 << TRIG)) & ~(1 << ECHO)); // TRIG = 출력, ECHO = 입력
    DDRD = (1 << PD2) | (1 << PD1) | (1 << PD0) | (1 << PD7) | (1 << PD6) | (1 << PD5); // 모터 제어 핀 설정
    DDRC |= (1 << PC0) | (1 << PC1); // LED 핀 설정
    PORTC &= ~((1 << PC0) | (1 << PC1)); // LED 초기화 (OFF)

    while (1) {
        // 초음파 센서를 이용하여 거리 측정
        TCCR1B = 0x03; // 타이머 1 시작, 64분주 설정 (클록 주기 = 4us)

        PORTE &= ~(1 << TRIG); // Trig LOW
        _delay_us(10);
        PORTE |= (1 << TRIG); // Trig HIGH (거리 측정 신호 발생)
        _delay_us(10);
        PORTE &= ~(1 << TRIG); // Trig LOW

        // Echo 신호 대기 및 거리 계산
        while (!(PINE & (1 << ECHO))) ; // Echo HIGH 대기
        TCNT1 = 0x0000; // 타이머 값 초기화

        while (PINE & (1 << ECHO)) ; // Echo LOW 대기
        TCCR1B = 0x00; // 타이머 정지

        // 거리 계산 (단위: mm)
        distance = (unsigned int)(SOUND_VELOCITY * (TCNT1 * 4 / 2) / 1000);

        // 장애물 거리 기반 제어
        if (distance < 150) {
            // 150mm => 15cm 이내 장애물: 모터 정지, 긴 경고음, LED 점멸
            PORTD &= ~((1 << PD0) | (1 << PD5)); // 모터 비활성화(정지상태)
            for (i = 0; i < 10; i++) {
                for (int j = 0; j < 10; j++) {
                    PORTC |= (1 << PC1); // LED1 ON
                    PORTC &= ~(1 << PC0); // LED2 OFF
                    PORTB = 0x10; // 부저 ON
                    _delay_ms(1);
                    PORTB = 0x00; // 부저 OFF
                    _delay_ms(1);
                }
                _delay_ms(100);
            }
        } else if (distance < 450) {
            // 45cm 이내 장애물: 짧은 경고음, LED1 ON, 모터 느리게 동작
            buzz2(); //부저 함수(150 < distance < 450의 경우)
            PORTC |= (1 << PC0); // LED1 ON
            PORTC &= ~(1 << PC1); // LED2 OFF
            dc_motor_control(2500); // 없는 상태에 비해 느린 속도 설정
        } else {
            // 장애물 없음: LED OFF, 모터 빠르게 동작
            PORTC &= ~((1 << PC0) | (1 << PC1)); // LED OFF
            dc_motor_control(4000); // 빠른 속도 설정
        }
    }

    return 0;
}
