//OM SHREE SAI...
//This is SCAD_APP.C
//Created on 03-05-13, 11:08AM

#include "scad_app.h"

#include "uip.h"
#include "httpd.h"
#include "httpd-fs.h"
#include "httpd-cgi.h"
#include "http-strings.h"

#include <string.h>

//user includes..
#include "uart.h"
#include <stdio.h>
#include "lpc17xx_gpio.h"
#include "lpc17xx_pinsel.h"
#include "timer.h"

//#defines for the return type of parse strong func
#define UPDATE_PACKET_OK	0x12
#define COMMAND_PACKET_OK	0x13
#define ERROR				0x80

#define DONGLE_FACTOR		2

#define	ON	0xAA
#define OFF	0x55
uint8_t prev_state_1 = 0x55;
uint8_t prev_state_2 = 0x55;


uint8_t state_of_connection; //this variable will keep a tab on the connection status
//char greeting_message[] = "Welcome to SCAD TECHNOLOGIES\n";
char greeting_message[] = "Welcome to Smarthome Server\n";

char data_string[] = "COM,L11OF45,L12OF33\n";
char dummy_str[(sizeof(data_string)-1)*2*DONGLE_FACTOR];
char shadow_str[sizeof(data_string)-1];	//-1 accounts for the removal of \n

char send_string[] = "12345\n";
//char send_string[] = {"\t"};

int parse_string(void);
void display_string(void);

void scadapp_init(void)
{
  uip_listen(HTONS(55555));
  //printf("\nserv init");
	init_light_ports();
}

void init_light_ports(void){
	PINSEL_CFG_Type PinCfg;
	
	//port 1 defines
	PinCfg.Funcnum = 0;
	PinCfg.OpenDrain = 0;
	PinCfg.Pinmode = 0;
	
	PinCfg.Portnum = PINSEL_PORT_1;
	PinCfg.Pinnum = PINSEL_PIN_24;
	PINSEL_ConfigPin(&PinCfg);
	GPIO_SetDir(PINSEL_PORT_1, (1<<PINSEL_PIN_24),1);
	GPIO_SetValue(1,1<<24);
}

void pin_on(void){
	GPIO_SetValue(1,1<<24);
}

void pin_off(void){
	GPIO_ClearValue(1,1<<24);
}

struct timer interrupt_timer;
void t_delay(void){
	timer_set(&interrupt_timer, CLOCK_SECOND*1);//5 seconds delay, put this in #define later
	while(!timer_expired(&interrupt_timer));
}

void interrupt_cycle(void){
	pin_off();
	t_delay();
	pin_on();
	t_delay();
}

void case_0(){
	interrupt_cycle();
}
void case_1(){
	interrupt_cycle();
	interrupt_cycle();
}
void case_2(){
	interrupt_cycle();
	interrupt_cycle();
	interrupt_cycle();
}
void case_3(){
	interrupt_cycle();
	interrupt_cycle();
	interrupt_cycle();
	interrupt_cycle();
}


char count;
void scad_appfunc(void){
	//removed by sam uint8_t *ptr;
	//removed by sam int i;
	//removed by sam portBASE_TYPE xStatus;
	char temp[25];
	int len, parse_result;
	if(uip_connected()){
		state_of_connection = WELCOME_SENT;
		uip_send("Welcome to Smarthome Server\n", sizeof(greeting_message));
	}

	if((uip_acked()) && (state_of_connection == WELCOME_SENT)) {
		state_of_connection = WELCOME_ACKED;
	}
	if ( uip_newdata() ){ //if any new data is recd.
	//taking the data packet from the client
	//and storing it into a queue	
		
		len = uip_datalen();
		//sprintf(dummy_str, "\nsize of data is %d", len);
		//UARTSend(3, (uint8_t *)dummy_str, sizeof(dummy_str) );
		//uip_send(uip_appdata, len);
		// check the data length..
		count += 1;
		if(count == 2){
			memcpy((void *)dummy_str, uip_appdata,len);
			//UARTSend(3, (uint8_t *)dummy_str, sizeof(dummy_str) );	
			
			display_string();	 //android sends each character in the string as a 16-bit unicode

			//so to extract each character this function has to be called
			parse_result = parse_string();
			switch(parse_result){
				case UPDATE_PACKET_OK:
					uip_send(data_string, sizeof(data_string));
					
					break;
				case COMMAND_PACKET_OK:
					sprintf(send_string, "OK...\n");
					uip_send(send_string, sizeof(send_string));				
					//uip_send(data_string, sizeof(data_string));	
					sprintf(temp, "\nthis is a com packet:OK");
					UARTSend(3, (uint8_t *)temp, sizeof(temp) );
					break;
				case ERROR:
				default:
					sprintf(send_string, "OOPS!\n");
					uip_send(send_string, sizeof(send_string));
					break;
			
			}
			count = 0;
		}
		
		
		//uip_send(send_string, sizeof(send_string));
	}
	if ( uip_rexmit() ){ //if the condition is true means the server is asking 
							//for retransmission
		switch(state_of_connection){
			case WELCOME_SENT:
				uip_send("Welcome to MLC \n", 16);
				break;
			case WELCOME_ACKED:
				uip_send("Again\n", 6);
				break;
		}
	}
}




void display_string(void){
	int idx=0,j;
	char temp[40*DONGLE_FACTOR]={'\t'};
	for(idx=0,j=0;idx<=sizeof(dummy_str);idx++){
		
		if (dummy_str[idx]!=0x00){
			sprintf(temp,"\nchar: %c, pos:%d", dummy_str[idx],idx);
			UARTSend(3, (uint8_t *)temp, sizeof(temp) );
			shadow_str[j] = dummy_str[idx];
			j++;
						
		}
		if(dummy_str[idx]=='\n'){
			break;
		}
		
	}
	//UARTSend(3, (uint8_t *)shadow_str, sizeof(shadow_str) );
}
//#defines for the packet types
#define PACKET_TYPE_0	0
#define PACKET_TYPE_1	1
#define PACKET_TYPE_2	2

//#defines for the light position
#define LIGHT_ROOM_11	5
#define LIGHT_NUM_11	6
#define LIGHT_ROOM_12	13
#define LIGHT_NUM_12	14

//#defines for the status
#define LIGHT_11_STATUS_1	7 
#define LIGHT_11_STATUS_2	8
#define LIGHT_12_STATUS_1	15 
#define LIGHT_12_STATUS_2	16

//#defines for the intensity
#define LIGHT_11_INTENSITY_1	9 
#define LIGHT_11_INTENSITY_2	10
#define LIGHT_12_INTENSITY_1	17 
#define LIGHT_12_INTENSITY_2	18
int parse_string(void){
	int check_rest=0,temp_num=0,intensity=0;
	char temp[100] = {'\t'};

	
	if(shadow_str[PACKET_TYPE_0]=='C' && shadow_str[PACKET_TYPE_1]=='O' && shadow_str[PACKET_TYPE_2]=='M'){
		check_rest = 1;
		sprintf(temp, "\nthis is a command packet");
		UARTSend(3, (uint8_t *)temp, sizeof(temp) );	
	}
	else if(shadow_str[PACKET_TYPE_0]=='U' && shadow_str[PACKET_TYPE_1]=='P' && shadow_str[PACKET_TYPE_2]=='D'){
		check_rest = 0;
		sprintf(temp, "\nthis is a update packet");
		UARTSend(3, (uint8_t *)temp, sizeof(temp) );
		return UPDATE_PACKET_OK;
	}

	//check for light 1
	if(shadow_str[LIGHT_ROOM_11]=='1' && shadow_str[LIGHT_NUM_11]=='1' && check_rest==1){
		//check for on or off
		//first check if my current status and recd status are same
		//if yes then ignore
		//else make the changes and return
		if(((shadow_str[LIGHT_11_STATUS_1]=='O' && shadow_str[LIGHT_11_STATUS_2]=='N')
				&& (data_string[LIGHT_11_STATUS_1]=='O' && data_string[LIGHT_11_STATUS_2]=='N'))
			|| ((shadow_str[LIGHT_11_STATUS_1]=='O' && shadow_str[LIGHT_11_STATUS_2]=='F')
				&& (data_string[LIGHT_11_STATUS_1]=='O' && data_string[LIGHT_11_STATUS_2]=='F'))){
				//do nothing

		}
		else{//if not the smae then read the configuration recd
			sprintf(temp, "\nlight number 11...");
			UARTSend(3, (uint8_t *)temp, sizeof(temp) );	
			if (shadow_str[LIGHT_11_STATUS_1]=='O' && shadow_str[LIGHT_11_STATUS_2]=='N'){
			//if the device is intended to be on then check for the intensity
				temp_num = (int)shadow_str[LIGHT_11_INTENSITY_1];
				intensity = (temp_num%48) * 10;
				temp_num = (int)shadow_str[LIGHT_11_INTENSITY_2];
				intensity += (temp_num%48);	
				
				//data_string[] = "COM,L11ON99,L12OF33\n";
				data_string[LIGHT_11_STATUS_1] = 'O';
				data_string[LIGHT_11_STATUS_2] = 'N';
				data_string[LIGHT_11_INTENSITY_1] = '9';
				data_string[LIGHT_11_INTENSITY_2] = '9';
				
				if( prev_state_2 == 0x55 ){
					case_2();
				}
				else if( prev_state_2 == 0xAA ){
					case_3();
				}
				
				prev_state_1 = 0xAA;
				
				
				sprintf(temp, "\nlight inten 11= .%d", intensity);
				UARTSend(3, (uint8_t *)temp, sizeof(temp) );
				return COMMAND_PACKET_OK;
				
				
			}
			if (shadow_str[LIGHT_11_STATUS_1]=='O' && shadow_str[LIGHT_11_STATUS_2]=='F'){
			//if the device is intended to be off then send command for off
				
				//data_string[] = "COM,L11OF00,L12OF33\n";
				data_string[LIGHT_11_STATUS_1] = 'O';
				data_string[LIGHT_11_STATUS_2] = 'F';
				data_string[LIGHT_11_INTENSITY_1] = '0';
				data_string[LIGHT_11_INTENSITY_2] = '0';
				
				if( prev_state_2 == 0x55 ){
					case_0();
				}
				else if( prev_state_2 == 0xAA ){
					case_1();
				}
				prev_state_1 = 0x55;
				
				sprintf(temp, "\nlight 11 off.....");
				UARTSend(3, (uint8_t *)temp, sizeof(temp) );	
				return COMMAND_PACKET_OK;
				
				
			}
		}	 	
	}

	//check for light 2
	if(shadow_str[LIGHT_ROOM_12]=='1' && shadow_str[LIGHT_NUM_12]=='2' && check_rest==1){
		//check for on or off
		//first check if my current status and recd status are same
		//if yes then ignore
		//else make the changes and return
		if(((shadow_str[LIGHT_12_STATUS_1]=='O' && shadow_str[LIGHT_12_STATUS_2]=='N')
				&& (data_string[LIGHT_12_STATUS_1]=='O' && data_string[LIGHT_12_STATUS_2]=='N'))
			|| ((shadow_str[LIGHT_12_STATUS_1]=='O' && shadow_str[LIGHT_12_STATUS_2]=='F')
				&& (data_string[LIGHT_12_STATUS_1]=='O' && data_string[LIGHT_12_STATUS_2]=='F'))){
				//do nothing

		}
		else{
			if (shadow_str[LIGHT_12_STATUS_1]=='O' && shadow_str[LIGHT_12_STATUS_2]=='N'){
			//if the device is intended to be on then check for the intensity
				temp_num = (int)shadow_str[LIGHT_12_INTENSITY_1];
				intensity = (temp_num%48) * 10;
				temp_num = (int)shadow_str[LIGHT_12_INTENSITY_2];
				intensity += (temp_num%48);						
				
				//data_string[] = "COM,L11ON99,L12ON99\n";
				data_string[LIGHT_12_STATUS_1] = 'O';
				data_string[LIGHT_12_STATUS_2] = 'N';
				data_string[LIGHT_12_INTENSITY_1] = '9';
				data_string[LIGHT_12_INTENSITY_2] = '9';
				
				if( prev_state_1 == 0x55 ){
					case_1();
				}
				else if( prev_state_1 == 0xAA ){
					case_3();
				}
				prev_state_2 = 0xAA;
				
				sprintf(temp, "\nlight inten 12= %d....", intensity);
				UARTSend(3, (uint8_t *)temp, sizeof(temp) );
				return COMMAND_PACKET_OK;						
			}
			if (shadow_str[LIGHT_12_STATUS_1]=='O' && shadow_str[LIGHT_12_STATUS_2]=='F'){
			//if the device is intended to be off then send command for off
				sprintf(temp, "\nlight12 off");
				UARTSend(3, (uint8_t *)temp, sizeof(temp) );	
				//data_string[] = "COM,L11ON99,L12OF00\n";
				data_string[LIGHT_12_STATUS_1] = 'O';
				data_string[LIGHT_12_STATUS_2] = 'F';
				data_string[LIGHT_12_INTENSITY_1] = '0';
				data_string[LIGHT_12_INTENSITY_2] = '0';
				
				if( prev_state_1 == 0x55 ){
					case_0();
				}
				else if( prev_state_1 == 0xAA ){
					case_2();
				}
				prev_state_2 = 0x55;
				
				return COMMAND_PACKET_OK;
			}
		}	 	
	}
	return ERROR;
}


