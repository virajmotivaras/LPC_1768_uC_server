//OM SHREE SAI...
//This is SCAD_APP.H
//Created on 24-11-12, 05:11PM


#ifndef __SCAD_APP_H__
#define __SCAD_APP_H__

#ifndef UIP_APPCALL
#define UIP_APPCALL     scad_appfunc
#endif


typedef int uip_tcp_appstate_t;

//welcome sent is sent when connection is established
//welcome acked is sent when welcome is acknowledged
#define WELCOME_SENT 0
#define WELCOME_ACKED 1




void scadapp_init(void);
void scad_appfunc(void);

void init_light_ports(void);

#endif

