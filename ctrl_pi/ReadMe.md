# Ctrl_pi
```shell
$ sudo sh module_start.sh
$ sudo ./ctrl
```

# theory
1. pin설정
+ alternate function은 핀의 고유기능을 지정
+ PWM은 두 개의 채널이 있다. 0, 1
+ pin13 alt5 function 5, pin18 alt0 function이 핀을 PWM1, PWM0 채널로 기능을 변경
+ PWM 핀은 두 개의 펄스 폭 변조 출력을 제공합니다.
  - [pin mode setting](https://www.dummies.com/computers/raspberry-pi/raspberry-pi-projects-for-dummies-cheat-sheet/)

2. PWM의 2가지 모드 - N/M, M/S
+ mode setting
  - msen = 0 -> n/m mode
  - msen = 1 -> m/s mode

+ N/M - 0과 1이 섞인 비트열
```
본 모드 인 MSEN = 0 인 경우 전송 될 데이터는 N의 값 N으로 해석됩니다.
위에서 설명한 알고리즘. 데이터를 전송하는 데 사용되는 클럭 사이클 수 (범위)는
연산. 결과 듀티 사이클이 N / M이되도록이 범위 내에서 펄스가 전송됩니다. 채널이
데이터 레지스터가 사용되거나 버퍼가 사용되고 빈 상태가 아닌 한 지속적으로 출력됩니다.
```
+ M/S - 1과 0이 연속 둘이 섞이지 않음
```
MSEN = 1 인 경우 PWM 블록은 위에서 설명한 알고리즘을 사용하지 않고 대신 직렬 데이터를 전송합니다.
아래 그림과 같이 M / S 비율로 M은 전송할 데이터이고 S는 범위입니다. 이 모드
고주파 변조가 필요하지 않거나 부정적인 영향을 미치는 경우에 바람직하다. 채널
데이터 레지스터가 사용되거나 버퍼가 사용되고 비어 있지 않은 한 출력을 계속 보냅니다.
```

3. duty cycle and period
+ servo 90의 한 주기는 20ms인데 -90 ~ 90 degree는 0.5ms ~ 2.5ms 사이에서 발생
```
20ms -> 3072으로 매핑 (3072은 내가 바꾸기 나름)
20ms : 3072 = 0.5ms : x
x = 77
20ms : 3072 = 2.5ms : x
x = 384
```
4. raspberry pi PWM clock 변경
+ 기본 clock은 19.2MHz 이다.
+ clock 변경
  - 나눔 제수 구하기 공식 div = 19200000 / (range * frequency)
    + 이 프로젝트의 모터는 range 0 ~ 3072 이며 50Hz를 가진다.
    + 공식에 의하면 바꿀 주파수는 153600Hz가 된다(range * frequency)
    + 나눔 제수를 clock idiv register에 넣어야 한다. 
  - [clock div 구하기 공식](https://books.google.co.kr/books?id=1FUnCgAAQBAJ&pg=PA423&lpg=PA423&dq=%EB%9D%BC%EC%A6%88%EB%B2%A0%EB%A6%AC%ED%8C%8C%EC%9D%B4+clock+div&source=bl&ots=Y2gDh7iJ2L&sig=ACfU3U3Fqgs8gotAcHQ9q-fzOlHvPuJCng&hl=ko&sa=X&ved=2ahUKEwifivbU4OzpAhXRQN4KHe6kBfIQ6AEwCHoECAkQAQ#v=onepage&q=%EB%9D%BC%EC%A6%88%EB%B2%A0%EB%A6%AC%ED%8C%8C%EC%9D%B4%20clock%20div&f=false)   
 
+ 클럭 변경을 위해서는 BCMPASSWORD를 이용해야한다.
  - [BCM PASSWORD](https://www.scribd.com/doc/101830961/GPIO-Pads-Control2)
  - "중요한"주변 장치 레지스터 (클록 관리, 전원 관리, 리셋 / 워치 독 제어)에 대한 
  - 모든 쓰기는 0x5A로 처음 8 비트에 쓰거나 쓰기는 무시됩니다.
  - 즉, 레지스터를 수정하는 경우 1 비트가 변경 되더라도 쓰기 값은 0x5A000000으로 OR연산됩니다.
  - BCM2835 108쪽부터 해당 내용

+ 오실로스코프
  - 오실로스코프(영어: oscilloscope 이전 명칭 oscillograph)는 특정 시간 간격(대역)의 전압 변화를 볼 수 있는 장치이다. 
  - 주로 주기적으로 반복되는 전자 신호를 표시하는데 사용한다. 
  - 이 기기를 활용하면 시간에 따라 변화하는 신호를 주기적이고 반복적인 하나의 전압 형태로 파악할 수 있다.
