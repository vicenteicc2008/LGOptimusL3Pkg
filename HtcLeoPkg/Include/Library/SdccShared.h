static uchar spec_ver;
static int high_capacity = 0;

// Function prototypes
static void mmc_decode_cid(uint32_t * resp);
static void mmc_decode_csd(uint32_t * resp);
static int mmc_send_cmd(uint16_t cmd, uint32_t arg, uint32_t response[]);
static int check_clear_read_status(void);
static int check_clear_write_status(void);
static int card_set_block_size(uint32_t size);
static int read_SCR_register(uint16_t rca);
static int read_SD_status(uint16_t rca);
static int switch_mode(uint16_t rca);
int card_identification_selection(uint32_t cid[], uint16_t* rca, uint8_t* num_of_io_func);
static int card_transfer_init(uint16_t rca, uint32_t csd[], uint32_t cid[]);
static int read_a_block(uint32_t block_number, uint32_t read_buffer[]);
static int write_a_block(uint32_t block_number, uint32_t write_buffer[], uint16_t rca);
static int SDCn_init(uint32_t instance);