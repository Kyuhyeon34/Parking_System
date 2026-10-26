# 차량 감지 기반 스마트 주차 관제 솔루션

STM32F411 Nucleo + Qt5 주차장 차단기 시뮬레이션.
`QT-Parking-system_Test` 와 동작·구성이 동일한 버전이며, Qt5 환경에서 빌드/실행할 수 있도록
카메라 모듈(`qt-parking/cameraview.h/.cpp`)만 Qt5 Multimedia API로 새로 작성했습니다.
그 외 펌웨어와 Qt 앱 로직은 전부 동일합니다.

초음파로 차량을 감지하면 Qt가 번호판 인식 다이얼로그를 띄우고, 입차/출차를 확정하면 차단기 모터가 5초 시퀀스로 동작합니다.

> 번호판 인식은 아직 붙지 않았습니다. 지금은 `[TEST]` 버튼으로 입차/출차를 직접 지정합니다.

---

## 아키텍처

```
인터럽트 ──▶ Handler ──▶ state (전역) ──▶ StateMachine ──▶ Controller
             플래그 확인      상태 변경        상태 보고 지시        기능만 수행
```

| 상태 | 의미 | LED |
|---|---|---|
| `US_ON` | 초음파 스캔 중 | 🟢 초록 |
| `US_OFF` | 인식 / 입차 / 출차 다이얼로그 진행 중 | 🔴 빨강 |
| `MOTOR` | 차단기 5초 시퀀스 진행 중 | 🔴 빨강 |

처리 중에는 새 차가 어차피 들어올 수 없으므로 초음파 트리거를 멈춥니다.

---

## 배선

### MCU 핀 할당

| 기능 | 핀 | 모드 | Nucleo 커넥터 | 연결 대상 |
|---|---|---|---|---|
| USART2_TX | PA2 | AF7 | CN10-35 | **ST-LINK VCP (배선 불필요)** |
| USART2_RX | PA3 | AF7 | CN9-1 | 〃 |
| 모터 1 | PA0 | AF2 (TIM5_CH1) | CN7-28 | L298N `IN1` |
| 모터 2 | PA1 | AF2 (TIM5_CH2) | CN7-30 | L298N `IN2` |
| LED RED | PA4 | GPIO Out | CN7-32 (A2) | 330Ω → 빨강 LED |
| LED GREEN | PA5 | GPIO Out | CN10-11 (D13) | 330Ω → 초록 LED |
| 초음파 TRIG | PB0 | GPIO Out | CN7-34 (A3) | HC-SR04 `TRIG` |
| 초음파 ECHO | PB1 | EXTI1 입력 | CN10-24 | HC-SR04 `ECHO` |

### L298N (EZ 모터 R300 = 5V DC 모터)

| L298N | 연결 |
|---|---|
| `IN1` / `IN2` | PA0 / PA1 |
| `ENA` | 점퍼 유지 (상시 ON, 속도는 IN1/IN2 PWM으로) |
| `OUT1` / `OUT2` | 모터 2가닥 (극성 반대면 CW/CCW만 뒤바뀜) |
| `+12V` | 외부 전원 7~9V (+) |
| `GND` | 외부 전원 (−) **+ Nucleo GND (공통 접지 필수)** |
| `+5V` | 연결하지 말 것 |

L298N이 약 2V를 먹으므로 PWM 듀티 **70%** 로 실효 5V를 맞춥니다 (`MOTOR_GATE_DUTY`).

### HC-SR04

`VCC` → Nucleo 5V (CN7-18), `GND` → Nucleo GND, `TRIG` → PB0, `ECHO` → PB1.

> PB1은 데이터시트상 5V 내성이 없는 핀입니다. 값이 튀거나 핀이 죽으면 `US_Controller.h` 의
> `US_ECHO_PIN` 을 `6U`(PB6, 5V 내성)로 바꾸고 `Handler.c` 의 `EXTI1_IRQHandler` 를
> `EXTI9_5_IRQHandler` 로 바꾸면 됩니다.

### LED

`PA4/PA5 → 330Ω → LED 애노드(긴 다리) → 캐소드 → GND` (액티브 하이).
PA5는 보드의 LD2와 공유되어 함께 켜집니다.

---

## 시리얼 프로토콜

ASCII 라인, `\n` 종단, **115200 8N1**. PuTTY만으로도 펌웨어 단독 디버깅이 가능합니다.

### Qt → STM32

| 명령 | 응답 | 동작 |
|---|---|---|
| `PING` | `OK PONG` | 연결 확인 |
| `STATE?` | `OK STATE <이름>` | 상태 질의 |
| `DIST?` | `OK DIST <cm>` | 거리 질의 |
| `RECOG START` | `OK RECOG` | `US_OFF` 진입 (초록 → 빨강) |
| `RECOG CANCEL` | `OK SCAN` | `US_ON` 복귀 (빨강 → 초록) |
| `GATE OPEN` | `OK GATE` | 차단기 5초 시퀀스 시작 |
| `LED RED\|GREEN ON\|OFF` | `OK LED` | 수동 제어 (디버그) |
| `MOTOR CW\|CCW <duty>` / `MOTOR STOP` | `OK MOTOR ...` | 수동 제어 (디버그) |

### STM32 → Qt

| 라인 | 의미 |
|---|---|
| `DIST <cm>` | 100ms 주기 거리 보고 |
| `EVT DETECT <cm>` | 20cm 이내 진입 (차량당 1회) |
| `EVT CLEAR` | 25cm 밖으로 벗어남 |
| `EVT GATE OBSTRUCTED` | 닫기 직전 안전 확인에서 물체가 계속 감지되어 최대 대기(3초) 후 닫음 |
| `EVT GATE DONE` | 차단기 시퀀스 종료 |
| `STATE <이름>` | 상태 전이 알림 |

---

## 동작 흐름

```
초음파 감지 (또는 [TEST] 차량 감지)
        │  RECOG START  → LED 초록 ▶ 빨강
        ▼
   차량 인식중 다이얼로그
        ├ [TEST] 입차 ─▶ 입차 확인 다이얼로그 ─ 확인 ─────────┐
        ├ [TEST] 출차 ─┬ (사전정산 O) 안내만 표시 ────────────┤
        │              └ (사전정산 X) 출차 정산 ─ 결제하기 ───┤
        └ 취소 ────────▶ RECOG CANCEL, 초록 복귀              │
                                                              ▼
                                        GATE OPEN → 차단기 시퀀스
                            (CCW 2초 → 정지 1초 → 안전 확인 → CW 2초)
                                                              │
                                    EVT GATE DONE → 초록 복귀, 대수 갱신
```

**닫기 전 안전 확인**: 차량이 지나간 뒤 바로 닫지 않고 초음파로 재확인합니다. 감지 거리 안에
물체가 있으면 닫지 않고 대기하며, 최대 3초까지 기다린 뒤에는 `EVT GATE OBSTRUCTED`를 보내고
닫습니다(센서 고장으로 영원히 멈추는 것을 막기 위한 안전장치).

**사전정산**: 메인 화면의 `사전정산` 버튼 → 차량 번호 입력 → 조회 후 금액 확인 → 정산.
정산된 차량은 주차 대수에 그대로 잡혀 있다가, 출차 시 결제 화면 없이 안내만 뜨고 차단기가
바로 열립니다.

---

## 빌드 / 실행

```sh
make            # 펌웨어 + Qt 앱 전체
make firmware   # 펌웨어만
make qt         # Qt 앱만
make flash      # 보드에 write (STM32CubeProgrammer 필요)
make run        # 전체 빌드 + flash + Qt 앱 실행
make clean
```

**필요 도구**: ARM GNU Toolchain, STM32CubeProgrammer, MSYS2 UCRT64의 Qt5 (`widgets`, `serialport`,
`multimedia`). 이 컴퓨터에는 UCRT64에 Qt6도 같이 깔려 있어서 `qmake.exe`는 Qt6를 가리키므로,
반드시 Qt5 전용 바이너리인 **`qmake-qt5.exe`** 를 씁니다 (`Makefile`에 이미 반영됨).

STM32CubeProgrammer 설치 경로가 다르면 `CUBE_DIR` 로 덮어쓰면 됩니다:

```sh
make flash CUBE_DIR="C:/Program Files/STMicroelectronics/STM32Cube/STM32CubeProgrammer/bin"
```

Qt 앱은 **시리얼 미연결 상태에서도** `[TEST]` 버튼으로 전체 흐름을 확인할 수 있습니다.

---

## 파일 구성

### `firmware/`

`QT-Parking-system_Test/firmware` 와 완전히 동일합니다 (Qt 버전과 무관한 STM32 코드이므로 그대로 복사).

| 계층 | 파일 | 역할 |
|---|---|---|
| Controller | `LED_Controller`, `US_Controller`, `UART2_Controller`, `Motor_Controller` | 기능만 수행, 정책 없음 |
| | `Timebase` | SysTick 1ms + TIM2 1µs 프리런 카운터 |
| Handler | `Handler.c` | 모든 ISR(SysTick / EXTI1 / USART2) + 폴링 핸들러 4종 |
| StateMachine | `state_manager` | 전역 state 소유, controller에 지시 |
| | `protocol` | UART 라인 파서 / 이벤트 포매터 |

`crt0.s` 가 모든 벡터를 weak alias로 두기 때문에 ISR을 정의만 하면 자동으로 연결됩니다 (벡터테이블 수정 불필요).
**새 `.c` 파일은 `firmware/Makefile` 의 `C_SOURCES` 에 직접 추가해야 합니다.**

### `qt-parking/`

| 파일 | 역할 |
|---|---|
| `mainwindow` | 전체 흐름 소유. 다이얼로그는 서로를 열지 않고 결과만 반환 |
| `loadingdialog` | 번호판 인식 자리. 지금은 `[TEST]` 버튼으로 입차/출차 분기 |
| `entrydialog` / `exitdialog` | 입차 확인 / 출차 정산 |
| `serialcontroller` | QSerialPort 래퍼 (115200 8N1, 라인 단위 송수신) |
| `cameraview` | **Qt5 전용 구현.** `QAbstractVideoSurface`로 프레임을 받아 `QImage`로 변환 (RGB32/ARGB32만 지원 포맷으로 광고해 색공간 변환은 백엔드에 위임, 상하 반전 보정 포함). `mainwindow` 쪽에서 보이는 인터페이스는 Qt6 버전과 동일 |
| `parkingstore` | 메모리 저장소. **DB 합류 시 이 클래스만 교체하면 됨** |

### 설정값

| 항목 | 값 | 위치 |
|---|---|---|
| 감지 / 해제 거리 | 20cm / 25cm | `state_manager.h` |
| 감지 확정 횟수 | 3회 연속 (노이즈 제거) | `Handler.c` |
| 차단기 시퀀스 | 2초 / 1초 / 2초 | `Motor_Controller.h` |
| 닫기 전 안전 확인 | 60ms 주기 재측정, 최대 3초 대기 | `Motor_Controller.h` |
| 모터 듀티 | 70% | `Motor_Controller.h` |
| 최대 주차 | 10대 | `parkingstore.h` |
| 요금 | 10분당 1,000원 (최소 1,000원) | `parkingstore.h` |

---

## 남은 작업

1. **번호판 인식** — OpenCV + Tesseract. `LoadingDialog` 의 `[TEST]` 버튼을 인식 결과로 교체하고,
   `MainWindow` 가 `ParkingStore::findOpenEntry(plate)` 로 입차/출차를 판정하도록 변경.
2. **DB 연동** — `ParkingStore` 를 실제 DB 클래스로 교체 (인터페이스 동일하게 맞춰둠).
3. **`QT-Parking-system_Test` 와의 동기화** — 펌웨어나 `qt-parking/`의 공통 로직이 바뀌면
   이 저장소에도 같은 변경을 반영해야 합니다 (자동 동기화 아님, `cameraview.*` 제외 전부 수동 복사).
