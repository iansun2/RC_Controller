#include <Arduino.h>
#include <nRF24L01.h>
#include <printf.h>
#include <RF24.h>
#include <RF24_config.h>
//https://nrf24.github.io/RF24/classRF24.html

//Version 1.1 2021/2/7

/*
                        y_input_forward_max
                                 |
                                 |
                                 |
                         / y_input_center \
x_input_left_max --------                  ---------- x_input_right_max
                         \ x_input_center /
                                 |
                                 |
                                 |
                        y_input_backward_max
*/

//pin setting//=========================================
//NRF24L01
#define CE_pin 7
#define CSN_pin 6
//control input
#define x_input_pin A0
#define y_input_pin A1

//input setting//==================================================
//x input
#define x_input_center 512
#define x_input_right_max 1023
#define x_input_left_max 0
#define x_input_deadzone 20
#define x_input_reverse 0
//y input
#define y_input_center 512
#define y_input_forward_max 1023
#define y_input_backward_max 0
#define y_input_deadzone 20
#define y_input_reverse 0

//output setting//=========================================
//x output
#define x_output_center 2000
#define x_output_left_max 1500
#define x_output_right_max 2500
//y output
#define y_output_center 0
#define y_output_forward_max 255
#define y_output_backward_max -255

//declear//=====================================================
RF24 radio(CE_pin, CSN_pin);
const byte nrf24l01_address[6] = "shink";
typedef struct{
  int16_t x_axis;
  int16_t y_axis;
}tx_data;
tx_data main_data;

//init nrf24l01 function//====================================================
void init_nrf24l01(){
  Serial.println("init nRF24L01");
  radio.begin();
  printf_begin(); 
  radio.openWritingPipe(nrf24l01_address);
  radio.setPALevel(RF24_PA_MAX,1);
  radio.setDataRate(RF24_250KBPS);
  radio.setChannel(120);
  radio.stopListening();
  radio.printDetails();
}

//controller function//====================================================
void controller_map(int16_t x_input,int16_t y_input){
  int16_t x_input_right_min = x_input_center + x_input_deadzone;
  int16_t x_input_left_min = x_input_center - x_input_deadzone;
  int16_t y_input_forward_min = y_input_center + y_input_deadzone;
  int16_t y_input_backward_min = y_input_center - y_input_deadzone;

  //input reverse
  if(x_input_reverse){
    x_input = x_input_right_max - x_input;
  }
  if(y_input_reverse){
    y_input = y_input_forward_max - y_input;
  }

  //x in center
  if(x_input > x_input_left_min && x_input < x_input_right_min){
    main_data.x_axis = x_output_center;
  //x in right
  }else if(x_input > x_input_right_min){
    main_data.x_axis = map(x_input, x_input_right_min, x_input_right_max, x_output_center, x_output_right_max);
  //x in left
  }else if(x_input < x_input_left_min){
    main_data.x_axis = map(x_input, x_input_left_min, x_input_left_max, x_output_center, x_output_left_max);
  }

  //y in center
  if(y_input < y_input_forward_min && y_input > y_input_backward_min){
    main_data.y_axis = y_output_center;
  //y in forward
  }else if(y_input > y_input_forward_min){
    main_data.y_axis = map(y_input, y_input_forward_min, y_input_forward_max, y_output_center, y_output_forward_max);
  //y in backward
  }else if(y_input < y_input_backward_min){
    main_data.y_axis = map(y_input, y_input_backward_min, y_input_backward_max, y_output_center, y_output_backward_max);
  }
}

//setup//=================================================================
void setup() {
  Serial.begin(115200);
  init_nrf24l01();
  main_data.x_axis = 0;
  main_data.y_axis = 0;
}

//main loop//===================================================================
void loop() {
  int16_t x_read = analogRead(x_input_pin);
  int16_t y_read = analogRead(y_input_pin);
  controller_map(x_read,y_read);
  radio.write(&main_data, sizeof(main_data) );
  Serial.println(main_data.x_axis);
  Serial.println(main_data.y_axis);
  delay(10);
}