#include "ets_sys.h"
#include "osapi.h"
#include "os_type.h"
#include "user_interface.h"

static volatile os_timer_t timer;
//Connection status
unsigned int    status = 0;

//Print the status, from the numeric one
static void ICACHE_FLASH_ATTR print_status(unsigned char status)
{
    switch(status)
    {
        case STATION_IDLE:
            os_printf("Idle...\n");
            break;
        case STATION_CONNECTING:
            os_printf("Connecting...\n");
            break;
        case STATION_WRONG_PASSWORD:
            os_printf("Wrong password...\n");
            break;            
        case STATION_NO_AP_FOUND:
            os_printf("Could not find an AP...\n");
            break;
        case STATION_CONNECT_FAIL:
            os_printf("Idle...\n");
            break;
        case STATION_GOT_IP:
            os_printf("Connected.\n");
            break;
        default:
            os_printf("Unknown status: %d\n", status);
            break;    
    }
}  

//Task to connect to an AP
void connect(void (*arg)())
{
    //Get connection status
    status = wifi_station_get_connect_status();
    switch(status)
    {
        case STATION_GOT_IP:
             print_status(status);
            //Stop calling me
            os_timer_disarm(&timer);
            //Call callback.
            arg();
            break;
        //Not connected
        case STATION_IDLE: 
        case STATION_WRONG_PASSWORD:
        case STATION_NO_AP_FOUND:
        case STATION_CONNECT_FAIL:
            print_status(status);
            //Connect
            if (wifi_station_connect())
            {
                os_printf("Trying to connect...\n");
            }
            else
            {
                os_printf("Failed to connect\n");
            }
            break;
        default:
            print_status(status);
            break;    
    }
}

//This is called when a cennection has been made.
void connected_cb(void)
{
    os_printf("This is where further stuff should happen.\n");
}

//Init function 
void ICACHE_FLASH_ATTR user_init()
{
    char ssid[32] = "default";
    char password[64] = "password";
    struct station_config station_conf;

    //No auto connect
    wifi_station_set_auto_connect(false);
    
    // Set baud rate of debug port
    uart_div_modify(0,UART_CLK_FREQ / 115200);

    //Set station mode
    wifi_set_opmode( STATION_MODE );

    //Set AP settings
    os_memcpy(&station_conf.ssid, ssid, 32);
    os_memcpy(&station_conf.password, password, 64);
    //No BSSID
    station_conf.bssid_set = 0;

    //Set config
    wifi_station_set_config(&station_conf);

    //Start connection task
    //Disarm timer
    os_timer_disarm(&timer);
    //Setup timer, pass callbaack as parameter.
    os_timer_setfn(&timer, (os_timer_func_t *)connect, connected_cb);
    //Arm the timer, run every 1 second.
    os_timer_arm(&timer, 1000, 1);
    
    os_printf("\nLeaving user_init...\n");
}
