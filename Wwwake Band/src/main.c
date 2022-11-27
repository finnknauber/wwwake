#include "stm8s.h"
#include "stm8s_i2c.h"
#include "stm8s_clk.h"
#include "stm8s_uart.h"

#define LED GPIOA, GPIO_PIN_3
#define DS_INTERRUPT_PIN GPIOD, GPIO_PIN_3
#define UART_INTERRUPT_PIN GPIOD, GPIO_PIN_2
#define DS_ADDRESS 0xD0
#define DS_CONTROL 0x0E
#define DS_STATUS 0x0F


// wait some time in milliseconds (not perfect but close enough)
void delay(uint32_t ms) {
	uint32_t nCount = ms*CLK_GetClockFreq()/2500;
	while (nCount != 0) {
		nCount--;
	}
}

// needed for successful compile ¯\_(ツ)_/¯
void assert_failed(uint8_t* file, uint32_t line) {
    while (1) {file=0; line=0;}
}

// setup i2c peripheral
void i2cSetup() {
    uint8_t Input_Clock = 0;
    Input_Clock = CLK_GetClockFreq()/1000000;
    I2C_Init(100000, 0, I2C_DUTYCYCLE_2, I2C_ACK_CURR, I2C_ADDMODE_7BIT, Input_Clock);
}

// write a byte in a clock register
void clockWriteByte(uint8_t writeAddr, uint8_t data) {
    while(I2C_GetFlagStatus(I2C_FLAG_BUSBUSY));
    I2C_GenerateSTART(ENABLE);
    while(!I2C_CheckEvent(I2C_EVENT_MASTER_MODE_SELECT));
    I2C_Send7bitAddress(DS_ADDRESS, I2C_DIRECTION_TX);
    while(!I2C_CheckEvent(I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));

    I2C_SendData(writeAddr);
    while(!I2C_CheckEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTED));
    I2C_SendData(data);
    while(!I2C_CheckEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTED));

    I2C_GenerateSTOP(ENABLE);
}

// read a byte from a clock register
uint8_t clockReadByte(uint8_t readAddr) {
    uint8_t pBuffer;
    while(I2C_GetFlagStatus(I2C_FLAG_BUSBUSY));
    I2C_GenerateSTART(ENABLE);
    while(!I2C_CheckEvent(I2C_EVENT_MASTER_MODE_SELECT));
    I2C_Send7bitAddress(DS_ADDRESS, I2C_DIRECTION_TX);
    while(!I2C_CheckEvent(I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));
    I2C_SendData(readAddr);
    while(!I2C_CheckEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTED));
    I2C_GenerateSTART(ENABLE);
    while(!I2C_CheckEvent(I2C_EVENT_MASTER_MODE_SELECT));
    I2C_Send7bitAddress(DS_ADDRESS, I2C_DIRECTION_RX);
    while(!I2C_CheckEvent(I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED));
    while(!I2C_CheckEvent(I2C_EVENT_MASTER_BYTE_RECEIVED));
    pBuffer = I2C_ReceiveData();
    I2C_AcknowledgeConfig(I2C_ACK_NONE);
    I2C_GenerateSTOP(ENABLE);
    return pBuffer;
}

// read multiple bytes from clock registers
void clockReadBytes(uint8_t *buffer, uint8_t startAddress, uint8_t numBytes) {
    for (int i = 0; i<numBytes; i++) {
        buffer[i] = clockReadByte(startAddress+i);
    }
}

// set clock format to 24 hours
void setupClock() {
    // write time register
    uint8_t currentValue = clockReadByte(0x02); 
    clockWriteByte(0x02, (currentValue&0xBF));
    // write alarm 1 register
    currentValue = clockReadByte(0x09);
    clockWriteByte(0x09, (currentValue&0xBF));
    currentValue = clockReadByte(DS_CONTROL);
    currentValue |= 0x1C;
    clockWriteByte(DS_CONTROL, currentValue);
}

// WARNING! This function takes raw register values, not actual time data
void clockAdjust(uint8_t seconds, uint8_t minutes, uint8_t hours, uint8_t date, uint8_t month, uint8_t year) {
    clockWriteByte(0x00, seconds);
    clockWriteByte(0x01, minutes);
    clockWriteByte(0x02, hours);
    clockWriteByte(0x04, date);
    clockWriteByte(0x05, month);
    clockWriteByte(0x06, year);
}

void setAlarm(uint8_t seconds, uint8_t minutes, uint8_t hours) {
    uint8_t seconds_register = 0x7F & seconds;
    uint8_t minutes_register = 0x7F & minutes;
    uint8_t hours_register = 0x3F & hours;
    // Set ask Bit 4 for matching seconds, minutes, hours
    uint8_t date_register = 0x80;

    clockWriteByte(0x07, seconds_register);
    clockWriteByte(0x08, minutes_register);
    clockWriteByte(0x09, hours_register);
    clockWriteByte(0x0A, date_register);
}

void changeAlarmStatus(bool enable) {
    uint8_t currentValue = clockReadByte(DS_CONTROL);
    currentValue |= 0x1C;
    if (enable) {
        uint8_t status = clockReadByte(DS_STATUS);
        clockWriteByte(DS_STATUS, (status&0xFE));
        currentValue |= 0x01;
    }
    else {
        currentValue &= 0xFE;
    }
    clockWriteByte(DS_CONTROL, currentValue);
}


main() {
    // set clock frequency to 16 Mhz
    CLK_HSIPrescalerConfig(0);
    //  init Pins
    GPIO_DeInit(GPIOA);
    GPIO_Init(LED, GPIO_MODE_OUT_PP_LOW_SLOW);
    GPIO_Init(DS_INTERRUPT_PIN, GPIO_MODE_IN_PU_IT);
    GPIO_Init(UART_INTERRUPT_PIN, GPIO_MODE_IN_PU_IT);

    // init Comms
    uartBegin();
    i2cSetup();
    setupClock();
    // clockAdjust(0x30, 0x56, 0x18, 0x27, 0x11, 0x22);
    delay(100);
    // setAlarm(0x00, 0x37, 0x18);


    uint8_t buffer[7];

    while (1) {
        clockReadBytes(buffer, 0x00, 7);
        uint8_t seconds = ((buffer[0]&0x70)>>4)*10+(buffer[0]&DS_STATUS);
        uint8_t minutes = ((buffer[1]&0x70)>>4)*10+(buffer[1]&DS_STATUS);
        uint8_t hours = (((buffer[2]&0x20)>>5)*20+(buffer[2]&0x10)>>4)*10+(buffer[2]&DS_STATUS);
        uint8_t day = ((buffer[4]&0x30)>>4)*10+(buffer[4]&DS_STATUS);
        uint8_t month = ((buffer[5]&0x10)>>4)*10+(buffer[5]&DS_STATUS);
        uint16_t year = ((buffer[6]&0xF0)>>4)*10+(buffer[6]&DS_STATUS) + 2000U;


        uartPrintInt(year);
        uartPrintString(", ");
        uartPrintInt(month);
        uartPrintString(", ");
        uartPrintInt(day);
        uartPrintString(", ");
        uartPrintInt(hours);
        uartPrintString(", ");
        uartPrintInt(minutes);
        uartPrintString(", ");
        uartPrintInt(seconds);
        uartNewLine();


        uartPrintString("STATUS: ");
        uartPrintInt(clockReadByte(DS_STATUS));
        uartPrintString(", Pin: ");
        uartPrintInt(GPIO_ReadInputPin(DS_INTERRUPT_PIN));
        uartNewLine();

        if (uartAvailable()) {
            uartReadChar();
            uint8_t status = clockReadByte(DS_STATUS);
            clockWriteByte(DS_STATUS, (status&0xFE));
            // if ((clockReadByte(DS_CONTROL)&0x01)) {
            //     changeAlarmStatus(FALSE);
            //     uartPrintString("Disabled Alarm");
            // }
            // else {
            //     changeAlarmStatus(TRUE);
            //     uartPrintString("Enabled Alarm");
            // }
            uartNewLine();
        }
        else {
            delay(50);
            // halt();
        }
        delay(1500);
    }
}


