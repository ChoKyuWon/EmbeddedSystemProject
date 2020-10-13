# Final Project

## 라즈베리 파이를 이용한 온습도계

## 아키텍처

1. Sensor <-> Pi (I2C)
2. LCD <-> Pi (I2C)
3. LED <-> Pi (GPIO)
4. Pi <-> Laptop(Bluetooth)

* 유저는 웹 인터페이스를 통해 pi의 설정 조절
* 유저는...
1. LCD에 나타날 항목 설정 (온도/습도)
2. 온도의 경우 섭씨/화씨 변환 가능
3. 습도가 N% 이상일 경우 LED 켜지게 설정 가능(N은 임의의 양수)