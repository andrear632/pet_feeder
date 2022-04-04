#ifndef MAIN_FUNCTIONS
#define MAIN_FUNCTIONS

static void *emcute_thread(void *arg);
static int con(void);
static int pub(char* msg);
static void on_pub(const emcute_topic_t *topic, void *data, size_t len);
static int sub(void);
static int add_address(char* addr);
void echo_cb(void* arg);
void dispense(void);
int read_distance(void);
int read_pir(void);
void check_level(void);
void sensor_init(void);
void mqtts_init(void);

#endif // MAIN_FUNCTIONS