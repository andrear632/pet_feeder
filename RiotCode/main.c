// Local broker:
// in mosquitto.rsmb/rsmb launch "./src/broker_mqtts config.conf"
//
// Transparent bridge:
// Launch "python3 transparent_bridge.py"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "msg.h"
#include "net/emcute.h"
#include "net/ipv6/addr.h"
#include "periph/gpio.h"
#include "xtimer.h"
#include "thread.h"
#include "periph/adc.h"
#include "main.h"

#define RES ADC_RES_8BIT
#define INTERVAL 10*1000000 //usec
#define EMCUTE_PRIO         (THREAD_PRIORITY_MAIN - 1)
#define _IPV6_DEFAULT_PREFIX_LEN        (64U)
#define BROKER_PORT         1885
#define BROKER_ADDRESS      "2000:2::1"
#define TOPIC_RECEIVE       "topic_in"
#define TOPIC_SEND          "topic_out"

//to add addresses to board interface
extern int _gnrc_netif_config(int argc, char **argv);

//pins stepper motor
gpio_t pin_step_1 = GPIO_PIN(PORT_B, 3); //D3 -> IN1
gpio_t pin_step_2 = GPIO_PIN(PORT_B, 5); //D4 -> IN2
gpio_t pin_step_3 = GPIO_PIN(PORT_B, 4); //D5 -> IN3
gpio_t pin_step_4 = GPIO_PIN(PORT_B, 10); //D6 -> IN4

//pins ultrasonic sensor
gpio_t trigger_pin = GPIO_PIN(PORT_A, 9); //D8 -> trigger
gpio_t echo_pin = GPIO_PIN(PORT_A, 8); //D7 -> echo

//pin PIR
//A0 -> PIR out

//pin led
gpio_t led_pin = GPIO_PIN(PORT_C, 7); //D9 -> resistor -> longled

//ultrasonic sensor
uint32_t echo_time;
uint32_t echo_time_start;

//mqtts
static char stack[THREAD_STACKSIZE_DEFAULT];
static msg_t queue[8];
static emcute_sub_t subscriptions[1];
static char topics[1][64U];

//1 if fill level alarm is triggered, 0 otherwise
int alarm_triggered = 0;
//1 if it can dispense, 0 if it is busy dispensing
int can_dispense = 1;
//1 if check_level() needs to be run, 0 otherwise
int need_check = 0;

// *** \/ MQTTS \/ ***

static void *emcute_thread(void *arg){
    (void)arg;
    emcute_run(BROKER_PORT, "board");
    return NULL;
}

static int con(void){
    sock_udp_ep_t gw = { .family = AF_INET6, .port = BROKER_PORT };
    char *topic = NULL;
    char *message = NULL;
    size_t len = 0;

    ipv6_addr_from_str((ipv6_addr_t *)&gw.addr.ipv6, BROKER_ADDRESS);
    emcute_con(&gw, true, topic, message, len, 0);

    return 0;
}

static int pub(char* msg){
    emcute_topic_t t;
    unsigned flags = EMCUTE_QOS_0;
    t.name = TOPIC_SEND;

    emcute_reg(&t);
    emcute_pub(&t, msg, strlen(msg), flags);

    return 0;
}

static void on_pub(const emcute_topic_t *topic, void *data, size_t len){
    (void)topic;

    char *in = (char *)data;
    char msg[len+1];
    strncpy(msg, in, len);
    msg[len] = '\0';
    if (strcmp(msg, "dispense") == 0){
        dispense();
    }
}

static int sub(void){
    unsigned flags = EMCUTE_QOS_0;
    subscriptions[0].cb = on_pub;
    strcpy(topics[0], TOPIC_RECEIVE);
    subscriptions[0].topic.name = topics[0];
    
    emcute_sub(&subscriptions[0], flags);

    return 0;
}

static int add_address(char* addr){
    char * arg[] = {"ifconfig", "4", "add", addr};
    return _gnrc_netif_config(4, arg);
}

// *** /\ MQTTS /\ ***


void echo_cb(void* arg){ //callback function - ultrasonic sensor
	int val = gpio_read(echo_pin);
	uint32_t echo_time_stop;

    (void) arg;

	if(val){
		echo_time_start = xtimer_now_usec();
	}
    else{
		echo_time_stop = xtimer_now_usec();
		echo_time = echo_time_stop - echo_time_start;
	}
}

void dispense(void){ //stepper motor
    if (can_dispense) {
        can_dispense = 0;
        int steps = 0;
        int count = 2000;
        while (count) {
            switch(steps) {
                case 0:
                gpio_clear(pin_step_1);
                gpio_clear(pin_step_2);
                gpio_clear(pin_step_3);
                gpio_set(pin_step_4);
                break;
                case 1:
                gpio_clear(pin_step_1);
                gpio_clear(pin_step_2);
                gpio_set(pin_step_3);
                gpio_clear(pin_step_4);
                break;
                case 2:
                gpio_clear(pin_step_1);
                gpio_set(pin_step_2);
                gpio_clear(pin_step_3);
                gpio_clear(pin_step_4);
                break;
                case 3:
                gpio_set(pin_step_1);
                gpio_clear(pin_step_2);
                gpio_clear(pin_step_3);
                gpio_clear(pin_step_4);
                break;
            } 
            steps++;
            xtimer_msleep(2);
            if (steps>3){
                steps=0;
            }
            count--;
        }
        gpio_clear(pin_step_1);
        can_dispense = 1;
        need_check = 1;
    }
}

int read_distance(void){ //ultrasonic sensor
	echo_time = 0;
	gpio_clear(trigger_pin);
	xtimer_usleep(20);
	gpio_set(trigger_pin);
	xtimer_msleep(100);
	return echo_time;
}

int read_pir(void){ //PIR sensor
    int sample = 0;
    sample = adc_sample(ADC_LINE(0), RES);
    return sample;
}

void check_level(void){ //checks the fill level and sends it to the cloud
    int distance = read_distance(); //150 means full, >330 means empty, >300 means almost empty
    char msg[4];
    sprintf(msg, "%d", distance);
    pub(msg);
    if (alarm_triggered == 0 && distance >= 300){
        gpio_set(led_pin);
        alarm_triggered = 1;
    }
    else if (alarm_triggered == 1 && distance < 300){
        gpio_clear(led_pin);
        alarm_triggered = 0;
    }
}

void sensor_init(void){ //initializes all the pins
    gpio_init(pin_step_1, GPIO_OUT);
    gpio_init(pin_step_2, GPIO_OUT);
    gpio_init(pin_step_3, GPIO_OUT);
    gpio_init(pin_step_4, GPIO_OUT);

    gpio_init(trigger_pin, GPIO_OUT);
    gpio_init_int(echo_pin, GPIO_IN, GPIO_BOTH, &echo_cb, NULL); //imposta callback quando riceve input

    adc_init(ADC_LINE(0));

    gpio_init(led_pin, GPIO_OUT);

    read_distance(); //first read returns always 0
}

void mqtts_init(void){ //initializes the connection with the MQTT-SN broker
    msg_init_queue(queue, ARRAY_SIZE(queue));
    memset(subscriptions, 0, (1 * sizeof(emcute_sub_t)));
    thread_create(stack, sizeof(stack), EMCUTE_PRIO, 0, emcute_thread, NULL, "emcute");
    
    char * addr1 = "2000:2::2";
    add_address(addr1);
    char * addr2 = "ff02::1:ff1c:3fba";
    add_address(addr2);
    char * addr3 = "ff02::1:ff00:2";
    add_address(addr3);

    con();
    sub();
}

int main(void){
    xtimer_sleep(3);
    sensor_init();
    mqtts_init();

    uint32_t dispensed_time = -1 * INTERVAL;
    
    while (true) { //continuously reads the PIR value and dispenses food if needed
        if (read_pir()>128 && xtimer_now_usec() > dispensed_time + INTERVAL){
            dispense();
            dispensed_time = xtimer_now_usec();
        }
        if (need_check){
            check_level();
            need_check = 0;
        }
        xtimer_sleep(1);
    }

    return 0;
}
