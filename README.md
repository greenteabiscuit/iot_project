# iot_project

Place file in the directory same as libftd2xx.1.4.16.dylib

### To Compile:

gcc libftd2xx.1.4.16.dylib lamp_sensor.c -o iot

### To Run:

./iot 1000

### Changeable Variables

```
#define CS D5
#define SCK D7
#define SDI D6
#define LDAC D0
#define CLK D1
#define Din D3
#define Dout D4
#define CS_SHDN D2
```

You can change the variables here to light up the lamp.
