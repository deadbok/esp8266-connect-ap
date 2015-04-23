#include "ets_sys.h"
#include "osapi.h"
#include "gpio.h"
#include "os_type.h"
#include "user_interface.h"

//Task priority
#define connect_task_prio        0
//Task queue length.
#define connect_task_queue_len   4

//Task queue
os_event_t      connect_task_queue[connect_task_queue_len];
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
static void ICACHE_FLASH_ATTR connect(os_event_t *events)
{
    //Get connection status
    status = wifi_station_get_connect_status();
    switch(status)
    {
        //Connected
        case STATION_GOT_IP:
            print_status(status);
            break;
        //Not connected
        case STATION_IDLE: 
        case STATION_WRONG_PASSWORD:
        case STATION_NO_AP_FOUND:
        case STATION_CONNECT_FAIL:
            print_status(status);
            //Connect
            os_printf("Connect: %d\n", wifi_station_connect());
            //Reschedule
            system_os_post(connect_task_prio, 0, 0 );
            break;
        default:
            print_status(status);
            //Reschedule
            system_os_post(connect_task_prio, 0, 0 );
            break;    
    }
    os_delay_us(100000);
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
    wifi_set_opmode( 0x1 );

    //Set AP settings
    os_memcpy(&station_conf.ssid, ssid, 32);
    os_memcpy(&station_conf.password, password, 64);
    wifi_station_set_config(&station_conf);
    //No BSSID
    station_conf.bssid_set = 0;

    //Start connection task
    system_os_task(connect, connect_task_prio, connect_task_queue, connect_task_queue_len);
    system_os_post(connect_task_prio, 0, 0 );
    
    os_printf("Leaving user_init...\n");
}
