#include "board.h"

#include <debug.h>

#include <stddef.h>

#define EEPROM_ADDRESS 0x50
#define EEPROM_WRITE_DELAY 5
#define EEPROM_PAGE_SIZE 16
#define UART_BAUDRATE 115200
#define QSPI_BUS_SPEED 45000000  //45MHz

#define SDRAM_MODE_REGISTER_BURST_LEN_Pos 0
#define SDRAM_MODE_REGISTER_BURST_LEN_Msk (0x7U << SDRAM_MODE_REGISTER_BURST_LEN_Pos)
#define SDRAM_MODE_REGISTER_BURST_LEN_1 (0x0 << SDRAM_MODE_REGISTER_BURST_LEN_Pos)
#define SDRAM_MODE_REGISTER_BURST_LEN_2 (0x1 << SDRAM_MODE_REGISTER_BURST_LEN_Pos)
#define SDRAM_MODE_REGISTER_BURST_LEN_4 (0x2 << SDRAM_MODE_REGISTER_BURST_LEN_Pos)
#define SDRAM_MODE_REGISTER_BURST_LEN_8 (0x3 << SDRAM_MODE_REGISTER_BURST_LEN_Pos)
#define SDRAM_MODE_REGISTER_BURST_TYPE_Pos 3
#define SDRAM_MODE_REGISTER_BURST_TYPE_Msk (0x1U << SDRAM_MODE_REGISTER_BURST_TYPE_Pos)
#define SDRAM_MODE_REGISTER_BURST_TYPE_SEQUENTIAL (0x0 << SDRAM_MODE_REGISTER_BURST_TYPE_Pos)
#define SDRAM_MODE_REGISTER_BURST_TYPE_INTERLEAVED (0x1 << SDRAM_MODE_REGISTER_BURST_TYPE_Pos)
#define SDRAM_MODE_REGISTER_LATENCY_MODE_Pos 4
#define SDRAM_MODE_REGISTER_LATENCY_MODE_Msk (0x7U << SDRAM_MODE_REGISTER_LATENCY_MODE_Pos)
#define SDRAM_MODE_REGISTER_LATENCY_MODE_CAS_2 (0x2 << SDRAM_MODE_REGISTER_LATENCY_MODE_Pos)
#define SDRAM_MODE_REGISTER_LATENCY_MODE_CAS_3 (0x3 << SDRAM_MODE_REGISTER_LATENCY_MODE_Pos)
#define SDRAM_MODE_REGISTER_WRITE_BURST_MODE_Pos 9
#define SDRAM_MODE_REGISTER_WRITE_BURST_MODE_Msk (0x1 << SDRAM_MODE_REGISTER_WRITE_BURST_MODE_Pos)
#define SDRAM_MODE_REGISTER_WRITE_BURST_MODE_PROGRAMMED_LEN (0x0 << SDRAM_MODE_REGISTER_WRITE_BURST_MODE_Pos)
#define SDRAM_MODE_REGISTER_WRITE_BURST_MODE_SINGLE (0x1 << SDRAM_MODE_REGISTER_WRITE_BURST_MODE_Pos)

static gpio_driver_t *gpio_pc13_user_led0_driver = NULL;
static gpio_driver_t *gpio_pa01_user_led1_driver = NULL;
static gpio_driver_t *gpio_pa00_user_button_driver = NULL;
static gpio_driver_t *gpio_pa2_uart2_tx_driver = NULL;
static gpio_driver_t *gpio_pa3_uart2_rx_driver = NULL;
static gpio_driver_t *gpio_pb8_i2c_scl_driver = NULL;
static gpio_driver_t *gpio_pb9_i2c_sda_driver = NULL;
static gpio_driver_t *gpio_pf6_qspi_io3_driver = NULL;
static gpio_driver_t *gpio_pf7_qspi_io2_driver = NULL;
static gpio_driver_t *gpio_pf8_qspi_io0_driver = NULL;
static gpio_driver_t *gpio_pf9_qspi_io1_driver = NULL;
static gpio_driver_t *gpio_pd3_qspi_sck_driver = NULL;
static gpio_driver_t *gpio_pb6_qspi_ncs_driver = NULL;
static gpio_driver_t *gpio_pc0_sdram_we_driver = NULL;
static gpio_driver_t *gpio_pc2_sdram_cs_driver = NULL;
static gpio_driver_t *gpio_pc3_sdram_cke_driver = NULL;
static gpio_driver_t *gpio_pd0_sdram_dq2_driver = NULL;
static gpio_driver_t *gpio_pd1_sdram_dq3_driver = NULL;
static gpio_driver_t *gpio_pd8_sdram_dq13_driver = NULL;
static gpio_driver_t *gpio_pd9_sdram_dq14_driver = NULL;
static gpio_driver_t *gpio_pd10_sdram_dq15_driver = NULL;
static gpio_driver_t *gpio_pd14_sdram_dq0_driver = NULL;
static gpio_driver_t *gpio_pd15_sdram_dq1_driver = NULL;
static gpio_driver_t *gpio_pe0_sdram_ldqm_driver = NULL;
static gpio_driver_t *gpio_pe1_sdram_udqm_driver = NULL;
static gpio_driver_t *gpio_pe7_sdram_dq4_driver = NULL;
static gpio_driver_t *gpio_pe8_sdram_dq5_driver = NULL;
static gpio_driver_t *gpio_pe9_sdram_dq6_driver = NULL;
static gpio_driver_t *gpio_pe10_sdram_dq7_driver = NULL;
static gpio_driver_t *gpio_pe11_sdram_dq8_driver = NULL;
static gpio_driver_t *gpio_pe12_sdram_dq9_driver = NULL;
static gpio_driver_t *gpio_pe13_sdram_dq10_driver = NULL;
static gpio_driver_t *gpio_pe14_sdram_dq11_driver = NULL;
static gpio_driver_t *gpio_pe15_sdram_dq12_driver = NULL;
static gpio_driver_t *gpio_pf0_sdram_a0_driver = NULL;
static gpio_driver_t *gpio_pf1_sdram_a1_driver = NULL;
static gpio_driver_t *gpio_pf2_sdram_a2_driver = NULL;
static gpio_driver_t *gpio_pf3_sdram_a3_driver = NULL;
static gpio_driver_t *gpio_pf4_sdram_a4_driver = NULL;
static gpio_driver_t *gpio_pf5_sdram_a5_driver = NULL;
static gpio_driver_t *gpio_pf11_sdram_ras_driver = NULL;
static gpio_driver_t *gpio_pf12_sdram_a6_driver = NULL;
static gpio_driver_t *gpio_pf13_sdram_a7_driver = NULL;
static gpio_driver_t *gpio_pf14_sdram_a8_driver = NULL;
static gpio_driver_t *gpio_pf15_sdram_a9_driver = NULL;
static gpio_driver_t *gpio_pg0_sdram_a10_driver = NULL;
static gpio_driver_t *gpio_pg1_sdram_a11_driver = NULL;
static gpio_driver_t *gpio_pg2_sdram_a12_driver = NULL;
static gpio_driver_t *gpio_pg4_sdram_ba0_driver = NULL;
static gpio_driver_t *gpio_pg5_sdram_ba1_driver = NULL;
static gpio_driver_t *gpio_pg8_sdram_clk_driver = NULL;
static gpio_driver_t *gpio_pg15_sdram_cas_driver = NULL;

static i2c_driver_t *i2c_i2c1_driver = NULL;
static qspi_driver_t *qspi_qspi_driver = NULL;
static fmc_driver_t *fmc_fmc_driver = NULL;

led_driver_t *led_user_led0_driver = NULL;
led_driver_t *led_user_led1_driver = NULL;
button_driver_t *button_user_button_driver = NULL;
uart_driver_t *uart_uart2_driver = NULL;
eeprom_driver_t *eeprom_eeprom_driver = NULL;
flash_driver_t *flash_flash_driver = NULL;
sdram_driver_t *sdram_sdram_driver = NULL;

void board_init(void){
    DEBUG_INFO("Initializing board.");

    //----------------------------------------------------------------
    // configure user led 0

    gpio_driver_init(&gpio_pc13_user_led0_driver, GPIOC, 13);
    gpio_driver_config_as_output(gpio_pc13_user_led0_driver, OTYPER_PUSH_PULL, OSPEEDR_LOW, PUPDR_NONE);
    gpio_driver_set_name(gpio_pc13_user_led0_driver, "GPIOC", "USER_LED0");

    led_driver_init(&led_user_led0_driver, gpio_pc13_user_led0_driver);
    led_driver_set_name(led_user_led0_driver, "USER_LED0");

    //----------------------------------------------------------------
    // configure user led 1

    gpio_driver_init(&gpio_pa01_user_led1_driver, GPIOA, 1);
    gpio_driver_config_as_output(gpio_pa01_user_led1_driver, OTYPER_PUSH_PULL, OSPEEDR_LOW, PUPDR_NONE);
    gpio_driver_set_name(gpio_pa01_user_led1_driver, "GPIOA", "USER_LED1");

    led_driver_init(&led_user_led1_driver, gpio_pa01_user_led1_driver);
    led_driver_set_name(led_user_led1_driver, "USER_LED1");

    //----------------------------------------------------------------
    // configure user button

    gpio_driver_init(&gpio_pa00_user_button_driver, GPIOA, 0);
    gpio_driver_config_as_input(gpio_pa00_user_button_driver, PUPDR_NONE);
    gpio_driver_set_name(gpio_pa00_user_button_driver, "GPIOA", "USER_BUTTON");

    button_driver_init(&button_user_button_driver, gpio_pa00_user_button_driver);
    button_driver_set_name(button_user_button_driver, "USER_BUTTON");

    //----------------------------------------------------------------
    // configure uart2

    gpio_driver_init(&gpio_pa2_uart2_tx_driver, GPIOA, 2);
    gpio_driver_config_as_alternate(gpio_pa2_uart2_tx_driver, OTYPER_PUSH_PULL, OSPEEDR_LOW, PUPDR_NONE, AFR_PA2_USART2_TX);
    gpio_driver_set_name(gpio_pa2_uart2_tx_driver, "GPIOA", "UART2_TX");

    gpio_driver_init(&gpio_pa3_uart2_rx_driver, GPIOA, 3);
    gpio_driver_config_as_alternate(gpio_pa3_uart2_rx_driver, OTYPER_PUSH_PULL, OSPEEDR_LOW, PUPDR_NONE, AFR_PA3_USART2_RX);
    gpio_driver_set_name(gpio_pa3_uart2_rx_driver, "GPIOA", "UART2_RX");

    uart_driver_init(&uart_uart2_driver, USART2, gpio_pa3_uart2_rx_driver, gpio_pa2_uart2_tx_driver, UART_BAUDRATE);
    uart_driver_set_name(uart_uart2_driver, "UART2");

    uart_driver_enable_rx_interrupt(uart_uart2_driver);
    NVIC_EnableIRQ(USART2_IRQn);

    //----------------------------------------------------------------
    // configure i2c1 eeprom

    gpio_driver_init(&gpio_pb8_i2c_scl_driver, GPIOB, 8);
    gpio_driver_config_as_alternate(gpio_pb8_i2c_scl_driver, OTYPER_OPEN_DRAIN, OSPEEDR_LOW, PUPDR_NONE, AFR_PB8_I2C1_SCL);
    gpio_driver_set_name(gpio_pb8_i2c_scl_driver, "GPIOB", "I2C_SCL");

    gpio_driver_init(&gpio_pb9_i2c_sda_driver, GPIOB, 9);
    gpio_driver_config_as_alternate(gpio_pb9_i2c_sda_driver, OTYPER_OPEN_DRAIN, OSPEEDR_LOW, PUPDR_NONE, AFR_PB9_I2C1_SDA);
    gpio_driver_set_name(gpio_pb9_i2c_sda_driver, "GPIOB", "I2C_SDA");

    i2c_driver_init(&i2c_i2c1_driver, I2C1, gpio_pb9_i2c_sda_driver, gpio_pb8_i2c_scl_driver);
    i2c_driver_set_name(i2c_i2c1_driver, "I2C1");

    eeprom_driver_init(&eeprom_eeprom_driver, i2c_i2c1_driver, EEPROM_ADDRESS);
    eeprom_driver_config(eeprom_eeprom_driver, EEPROM_SIZE, EEPROM_PAGE_SIZE, EEPROM_WRITE_DELAY, 1);
    eeprom_driver_set_name(eeprom_eeprom_driver, "EEPROM");

    //----------------------------------------------------------------
    // configure qspi flash

    gpio_driver_init(&gpio_pf6_qspi_io3_driver, GPIOF, 6);
    gpio_driver_config_as_alternate(gpio_pf6_qspi_io3_driver, OTYPER_PUSH_PULL, OSPEEDR_HIGH, PUPDR_NONE, AFR_PF6_QUADSPI_BK1_IO3);
    gpio_driver_set_name(gpio_pf6_qspi_io3_driver, "GPIOF", "QSPI_IO3");

    gpio_driver_init(&gpio_pf7_qspi_io2_driver, GPIOF, 7);
    gpio_driver_config_as_alternate(gpio_pf7_qspi_io2_driver, OTYPER_PUSH_PULL, OSPEEDR_HIGH, PUPDR_NONE, AFR_PF7_QUADSPI_BK1_IO2);
    gpio_driver_set_name(gpio_pf7_qspi_io2_driver, "GPIOF", "QSPI_IO2");

    gpio_driver_init(&gpio_pf8_qspi_io0_driver, GPIOF, 8);
    gpio_driver_config_as_alternate(gpio_pf8_qspi_io0_driver, OTYPER_PUSH_PULL, OSPEEDR_HIGH, PUPDR_NONE, AFR_PF8_QUADSPI_BK1_IO0);
    gpio_driver_set_name(gpio_pf8_qspi_io0_driver, "GPIOF", "QSPI_IO0");

    gpio_driver_init(&gpio_pf9_qspi_io1_driver, GPIOF, 9);
    gpio_driver_config_as_alternate(gpio_pf9_qspi_io1_driver, OTYPER_PUSH_PULL, OSPEEDR_HIGH, PUPDR_NONE, AFR_PF9_QUADSPI_BK1_IO1);
    gpio_driver_set_name(gpio_pf9_qspi_io1_driver, "GPIOF", "QSPI_IO1");

    gpio_driver_init(&gpio_pd3_qspi_sck_driver, GPIOD, 3);
    gpio_driver_config_as_alternate(gpio_pd3_qspi_sck_driver, OTYPER_PUSH_PULL, OSPEEDR_HIGH, PUPDR_NONE, AFR_PD3_QUADSPI_CLK);
    gpio_driver_set_name(gpio_pd3_qspi_sck_driver, "GPIOD", "QSPI_SCK");

    gpio_driver_init(&gpio_pb6_qspi_ncs_driver, GPIOB, 6);
    gpio_driver_config_as_alternate(gpio_pb6_qspi_ncs_driver, OTYPER_PUSH_PULL, OSPEEDR_HIGH, PUPDR_NONE, AFR_PB6_QUADSPI_BK1_NCS);
    gpio_driver_set_name(gpio_pb6_qspi_ncs_driver, "GPIOB", "QSPI_NCS");

    qspi_driver_init(&qspi_qspi_driver, QUADSPI, QSPI_BUS_SPEED, gpio_pf8_qspi_io0_driver, gpio_pf9_qspi_io1_driver, gpio_pf7_qspi_io2_driver, gpio_pf6_qspi_io3_driver, gpio_pd3_qspi_sck_driver, gpio_pb6_qspi_ncs_driver);
    qspi_driver_set_spimode(qspi_qspi_driver, SPI_MODE_0);
    qspi_driver_set_csht(qspi_qspi_driver, 1);
    qspi_driver_set_fsize(qspi_qspi_driver, 22);
    qspi_driver_set_name(qspi_qspi_driver, "QUADSPI");

    flash_driver_init(&flash_flash_driver, qspi_qspi_driver);
    flash_driver_config(flash_flash_driver, FLASH_SIZE, FLASH_SECTOR_SIZE, 3);
    flash_driver_set_name(flash_flash_driver, "FLASH");

    //----------------------------------------------------------------
    // configure sdram

    gpio_driver_init(&gpio_pc0_sdram_we_driver, GPIOC, 0);
    gpio_driver_config_as_alternate(gpio_pc0_sdram_we_driver, OTYPER_PUSH_PULL, OSPEEDR_HIGH, PUPDR_NONE, AFR_PC0_FMC_SDNWE);
    gpio_driver_set_name(gpio_pc0_sdram_we_driver, "GPIOC", "SDRAM_WE");

    gpio_driver_init(&gpio_pc2_sdram_cs_driver, GPIOC, 2);
    gpio_driver_config_as_alternate(gpio_pc2_sdram_cs_driver, OTYPER_PUSH_PULL, OSPEEDR_HIGH, PUPDR_NONE, AFR_PC2_FMC_SDNE0);
    gpio_driver_set_name(gpio_pc2_sdram_cs_driver, "GPIOC", "SDRAM_CS");

    gpio_driver_init(&gpio_pc3_sdram_cke_driver, GPIOC, 3);
    gpio_driver_config_as_alternate(gpio_pc3_sdram_cke_driver, OTYPER_PUSH_PULL, OSPEEDR_HIGH, PUPDR_NONE, AFR_PC3_FMC_SDCKE0);
    gpio_driver_set_name(gpio_pc3_sdram_cke_driver, "GPIOC", "SDRAM_CKE");

    gpio_driver_init(&gpio_pd0_sdram_dq2_driver, GPIOD, 0);
    gpio_driver_config_as_alternate(gpio_pd0_sdram_dq2_driver, OTYPER_PUSH_PULL, OSPEEDR_HIGH, PUPDR_NONE, AFR_PD0_FMC_D2);
    gpio_driver_set_name(gpio_pd0_sdram_dq2_driver, "GPIOD", "SDRAM_DQ2");

    gpio_driver_init(&gpio_pd1_sdram_dq3_driver, GPIOD, 1);
    gpio_driver_config_as_alternate(gpio_pd1_sdram_dq3_driver, OTYPER_PUSH_PULL, OSPEEDR_HIGH, PUPDR_NONE, AFR_PD1_FMC_D3);
    gpio_driver_set_name(gpio_pd1_sdram_dq3_driver, "GPIOD", "SDRAM_DQ3");

    gpio_driver_init(&gpio_pd8_sdram_dq13_driver, GPIOD, 8);
    gpio_driver_config_as_alternate(gpio_pd8_sdram_dq13_driver, OTYPER_PUSH_PULL, OSPEEDR_HIGH, PUPDR_NONE, AFR_PD8_FMC_D13);
    gpio_driver_set_name(gpio_pd8_sdram_dq13_driver, "GPIOD", "SDRAM_DQ13");

    gpio_driver_init(&gpio_pd9_sdram_dq14_driver, GPIOD, 9);
    gpio_driver_config_as_alternate(gpio_pd9_sdram_dq14_driver, OTYPER_PUSH_PULL, OSPEEDR_HIGH, PUPDR_NONE, AFR_PD9_FMC_D14);
    gpio_driver_set_name(gpio_pd9_sdram_dq14_driver, "GPIOD", "SDRAM_DQ14");

    gpio_driver_init(&gpio_pd10_sdram_dq15_driver, GPIOD, 10);
    gpio_driver_config_as_alternate(gpio_pd10_sdram_dq15_driver, OTYPER_PUSH_PULL, OSPEEDR_HIGH, PUPDR_NONE, AFR_PD10_FMC_D15);
    gpio_driver_set_name(gpio_pd10_sdram_dq15_driver, "GPIOD", "SDRAM_DQ15");

    gpio_driver_init(&gpio_pd14_sdram_dq0_driver, GPIOD, 14);
    gpio_driver_config_as_alternate(gpio_pd14_sdram_dq0_driver, OTYPER_PUSH_PULL, OSPEEDR_HIGH, PUPDR_NONE, AFR_PD14_FMC_D0);
    gpio_driver_set_name(gpio_pd14_sdram_dq0_driver, "GPIOD", "SDRAM_DQ0");

    gpio_driver_init(&gpio_pd15_sdram_dq1_driver, GPIOD, 15);
    gpio_driver_config_as_alternate(gpio_pd15_sdram_dq1_driver, OTYPER_PUSH_PULL, OSPEEDR_HIGH, PUPDR_NONE, AFR_PD15_FMC_D1);
    gpio_driver_set_name(gpio_pd15_sdram_dq1_driver, "GPIOD", "SDRAM_DQ1");

    gpio_driver_init(&gpio_pe0_sdram_ldqm_driver, GPIOE, 0);
    gpio_driver_config_as_alternate(gpio_pe0_sdram_ldqm_driver, OTYPER_PUSH_PULL, OSPEEDR_HIGH, PUPDR_NONE, AFR_PE0_FMC_NBL0);
    gpio_driver_set_name(gpio_pe0_sdram_ldqm_driver, "GPIOE", "SDRAM_LDQM");

    gpio_driver_init(&gpio_pe1_sdram_udqm_driver, GPIOE, 1);
    gpio_driver_config_as_alternate(gpio_pe1_sdram_udqm_driver, OTYPER_PUSH_PULL, OSPEEDR_HIGH, PUPDR_NONE, AFR_PE1_FMC_NBL1);
    gpio_driver_set_name(gpio_pe1_sdram_udqm_driver, "GPIOE", "SDRAM_UDQM");

    gpio_driver_init(&gpio_pe7_sdram_dq4_driver, GPIOE, 07);
    gpio_driver_config_as_alternate(gpio_pe7_sdram_dq4_driver, OTYPER_PUSH_PULL, OSPEEDR_HIGH, PUPDR_NONE, AFR_PE7_FMC_D4);
    gpio_driver_set_name(gpio_pe7_sdram_dq4_driver, "GPIOE", "SDRAM_DQ4");

    gpio_driver_init(&gpio_pe8_sdram_dq5_driver, GPIOE, 8);
    gpio_driver_config_as_alternate(gpio_pe8_sdram_dq5_driver, OTYPER_PUSH_PULL, OSPEEDR_HIGH, PUPDR_NONE, AFR_PE8_FMC_D5);
    gpio_driver_set_name(gpio_pe8_sdram_dq5_driver, "GPIOE", "SDRAM_DQ5");

    gpio_driver_init(&gpio_pe9_sdram_dq6_driver, GPIOE, 9);
    gpio_driver_config_as_alternate(gpio_pe9_sdram_dq6_driver, OTYPER_PUSH_PULL, OSPEEDR_HIGH, PUPDR_NONE, AFR_PE9_FMC_D6);
    gpio_driver_set_name(gpio_pe9_sdram_dq6_driver, "GPIOE", "SDRAM_DQ6");

    gpio_driver_init(&gpio_pe10_sdram_dq7_driver, GPIOE, 10);
    gpio_driver_config_as_alternate(gpio_pe10_sdram_dq7_driver, OTYPER_PUSH_PULL, OSPEEDR_HIGH, PUPDR_NONE, AFR_PE10_FMC_D7);
    gpio_driver_set_name(gpio_pe10_sdram_dq7_driver, "GPIOE", "SDRAM_DQ7");

    gpio_driver_init(&gpio_pe11_sdram_dq8_driver, GPIOE, 11);
    gpio_driver_config_as_alternate(gpio_pe11_sdram_dq8_driver, OTYPER_PUSH_PULL, OSPEEDR_HIGH, PUPDR_NONE, AFR_PE11_FMC_D8);
    gpio_driver_set_name(gpio_pe11_sdram_dq8_driver, "GPIOE", "SDRAM_DQ8");

    gpio_driver_init(&gpio_pe12_sdram_dq9_driver, GPIOE, 12);
    gpio_driver_config_as_alternate(gpio_pe12_sdram_dq9_driver, OTYPER_PUSH_PULL, OSPEEDR_HIGH, PUPDR_NONE, AFR_PE12_FMC_D9);
    gpio_driver_set_name(gpio_pe12_sdram_dq9_driver, "GPIOE", "SDRAM_DQ9");

    gpio_driver_init(&gpio_pe13_sdram_dq10_driver, GPIOE, 13);
    gpio_driver_config_as_alternate(gpio_pe13_sdram_dq10_driver, OTYPER_PUSH_PULL, OSPEEDR_HIGH, PUPDR_NONE, AFR_PE13_FMC_D10);
    gpio_driver_set_name(gpio_pe13_sdram_dq10_driver, "GPIOE", "SDRAM_DQ10");

    gpio_driver_init(&gpio_pe14_sdram_dq11_driver, GPIOE, 14);
    gpio_driver_config_as_alternate(gpio_pe14_sdram_dq11_driver, OTYPER_PUSH_PULL, OSPEEDR_HIGH, PUPDR_NONE, AFR_PE14_FMC_D11);
    gpio_driver_set_name(gpio_pe14_sdram_dq11_driver, "GPIOE", "SDRAM_DQ11");

    gpio_driver_init(&gpio_pe15_sdram_dq12_driver, GPIOE, 15);
    gpio_driver_config_as_alternate(gpio_pe15_sdram_dq12_driver, OTYPER_PUSH_PULL, OSPEEDR_HIGH, PUPDR_NONE, AFR_PE15_FMC_D12);
    gpio_driver_set_name(gpio_pe15_sdram_dq12_driver, "GPIOE", "SDRAM_DQ12");

    gpio_driver_init(&gpio_pf0_sdram_a0_driver, GPIOF, 0);
    gpio_driver_config_as_alternate(gpio_pf0_sdram_a0_driver, OTYPER_PUSH_PULL, OSPEEDR_HIGH, PUPDR_NONE, AFR_PF0_FMC_A0);
    gpio_driver_set_name(gpio_pf0_sdram_a0_driver, "GPIOF", "SDRAM_A0");

    gpio_driver_init(&gpio_pf1_sdram_a1_driver, GPIOF, 1);
    gpio_driver_config_as_alternate(gpio_pf1_sdram_a1_driver, OTYPER_PUSH_PULL, OSPEEDR_HIGH, PUPDR_NONE, AFR_PF1_FMC_A1);
    gpio_driver_set_name(gpio_pf1_sdram_a1_driver, "GPIOF", "SDRAM_A1");

    gpio_driver_init(&gpio_pf2_sdram_a2_driver, GPIOF, 2);
    gpio_driver_config_as_alternate(gpio_pf2_sdram_a2_driver, OTYPER_PUSH_PULL, OSPEEDR_HIGH, PUPDR_NONE, AFR_PF2_FMC_A2);
    gpio_driver_set_name(gpio_pf2_sdram_a2_driver, "GPIOF", "SDRAM_A2");

    gpio_driver_init(&gpio_pf3_sdram_a3_driver, GPIOF, 3);
    gpio_driver_config_as_alternate(gpio_pf3_sdram_a3_driver, OTYPER_PUSH_PULL, OSPEEDR_HIGH, PUPDR_NONE, AFR_PF3_FMC_A3);
    gpio_driver_set_name(gpio_pf3_sdram_a3_driver, "GPIOF", "SDRAM_A3");

    gpio_driver_init(&gpio_pf4_sdram_a4_driver, GPIOF, 4);
    gpio_driver_config_as_alternate(gpio_pf4_sdram_a4_driver, OTYPER_PUSH_PULL, OSPEEDR_HIGH, PUPDR_NONE, AFR_PF4_FMC_A4);
    gpio_driver_set_name(gpio_pf4_sdram_a4_driver, "GPIOF", "SDRAM_A4");

    gpio_driver_init(&gpio_pf5_sdram_a5_driver, GPIOF, 5);
    gpio_driver_config_as_alternate(gpio_pf5_sdram_a5_driver, OTYPER_PUSH_PULL, OSPEEDR_HIGH, PUPDR_NONE, AFR_PF5_FMC_A5);
    gpio_driver_set_name(gpio_pf5_sdram_a5_driver, "GPIOF", "SDRAM_A5");

    gpio_driver_init(&gpio_pf11_sdram_ras_driver, GPIOF, 11);
    gpio_driver_config_as_alternate(gpio_pf11_sdram_ras_driver, OTYPER_PUSH_PULL, OSPEEDR_HIGH, PUPDR_NONE, AFR_PF11_FMC_SDNRAS);
    gpio_driver_set_name(gpio_pf11_sdram_ras_driver, "GPIOF", "SDRAM_RAS");

    gpio_driver_init(&gpio_pf12_sdram_a6_driver, GPIOF, 12);
    gpio_driver_config_as_alternate(gpio_pf12_sdram_a6_driver, OTYPER_PUSH_PULL, OSPEEDR_HIGH, PUPDR_NONE, AFR_PF12_FMC_A6);
    gpio_driver_set_name(gpio_pf12_sdram_a6_driver, "GPIOF", "SDRAM_A6");

    gpio_driver_init(&gpio_pf13_sdram_a7_driver, GPIOF, 13);
    gpio_driver_config_as_alternate(gpio_pf13_sdram_a7_driver, OTYPER_PUSH_PULL, OSPEEDR_HIGH, PUPDR_NONE, AFR_PF13_FMC_A7);
    gpio_driver_set_name(gpio_pf13_sdram_a7_driver, "GPIOF", "SDRAM_A7");

    gpio_driver_init(&gpio_pf14_sdram_a8_driver, GPIOF, 14);
    gpio_driver_config_as_alternate(gpio_pf14_sdram_a8_driver, OTYPER_PUSH_PULL, OSPEEDR_HIGH, PUPDR_NONE, AFR_PF14_FMC_A8);
    gpio_driver_set_name(gpio_pf14_sdram_a8_driver, "GPIOF", "SDRAM_A8");

    gpio_driver_init(&gpio_pf15_sdram_a9_driver, GPIOF, 15);
    gpio_driver_config_as_alternate(gpio_pf15_sdram_a9_driver, OTYPER_PUSH_PULL, OSPEEDR_HIGH, PUPDR_NONE, AFR_PF15_FMC_A9);
    gpio_driver_set_name(gpio_pf15_sdram_a9_driver, "GPIOF", "SDRAM_A9");

    gpio_driver_init(&gpio_pg0_sdram_a10_driver, GPIOG, 0);
    gpio_driver_config_as_alternate(gpio_pg0_sdram_a10_driver, OTYPER_PUSH_PULL, OSPEEDR_HIGH, PUPDR_NONE, AFR_PG0_FMC_A10);
    gpio_driver_set_name(gpio_pg0_sdram_a10_driver, "GPIOG", "SDRAM_A10");

    gpio_driver_init(&gpio_pg1_sdram_a11_driver, GPIOG, 1);
    gpio_driver_config_as_alternate(gpio_pg1_sdram_a11_driver, OTYPER_PUSH_PULL, OSPEEDR_HIGH, PUPDR_NONE, AFR_PG1_FMC_A11);
    gpio_driver_set_name(gpio_pg1_sdram_a11_driver, "GPIOG", "SDRAM_A11");

    gpio_driver_init(&gpio_pg2_sdram_a12_driver, GPIOG, 2);
    gpio_driver_config_as_alternate(gpio_pg2_sdram_a12_driver, OTYPER_PUSH_PULL, OSPEEDR_HIGH, PUPDR_NONE, AFR_PG2_FMC_A12);
    gpio_driver_set_name(gpio_pg2_sdram_a12_driver, "GPIOG", "SDRAM_A12");

    gpio_driver_init(&gpio_pg4_sdram_ba0_driver, GPIOG, 4);
    gpio_driver_config_as_alternate(gpio_pg4_sdram_ba0_driver, OTYPER_PUSH_PULL, OSPEEDR_HIGH, PUPDR_NONE, AFR_PG4_FMC_BA0);
    gpio_driver_set_name(gpio_pg4_sdram_ba0_driver, "GPIOG", "SDRAM_BA0");

    gpio_driver_init(&gpio_pg5_sdram_ba1_driver, GPIOG, 5);
    gpio_driver_config_as_alternate(gpio_pg5_sdram_ba1_driver, OTYPER_PUSH_PULL, OSPEEDR_HIGH, PUPDR_NONE, AFR_PG5_FMC_BA1);
    gpio_driver_set_name(gpio_pg5_sdram_ba1_driver, "GPIOG", "SDRAM_BA1");

    gpio_driver_init(&gpio_pg8_sdram_clk_driver, GPIOG, 8);
    gpio_driver_config_as_alternate(gpio_pg8_sdram_clk_driver, OTYPER_PUSH_PULL, OSPEEDR_HIGH, PUPDR_NONE, AFR_PG8_FMC_SDCLK);
    gpio_driver_set_name(gpio_pg8_sdram_clk_driver, "GPIOG", "SDRAM_CLK");

    gpio_driver_init(&gpio_pg15_sdram_cas_driver, GPIOG, 15);
    gpio_driver_config_as_alternate(gpio_pg15_sdram_cas_driver, OTYPER_PUSH_PULL, OSPEEDR_HIGH, PUPDR_NONE, AFR_PG15_FMC_SDNCAS);
    gpio_driver_set_name(gpio_pg15_sdram_cas_driver, "GPIOG", "SDRAM_CAS");

    //CAS=2 @ 90MHz is42s16160j-7
    fmc_driver_bank_5_6_timing_t sdram_timing;
    sdram_timing.trp = 2;
    sdram_timing.trc = 6;
    sdram_timing.trcd = 2;
    sdram_timing.twr = 2;
    sdram_timing.tras = 4;
    sdram_timing.txsr = 7;
    sdram_timing.tmrd = 2;
    sdram_timing.pres = 2;
    sdram_timing.rtr = 650;

    fmc_driver_bank_5_6_device_config_t sdram_config;
    sdram_config.cas = 2;
    sdram_config.banks = 4;
    sdram_config.data_width = 16;
    sdram_config.column_address_width = 9;
    sdram_config.row_address_width = 13;
    sdram_config.nrfs = 4;
    sdram_config.mode_register = (SDRAM_MODE_REGISTER_LATENCY_MODE_CAS_2 | SDRAM_MODE_REGISTER_WRITE_BURST_MODE_SINGLE);

    fmc_driver_init(&fmc_fmc_driver);
    fmc_driver_bank_5_6_config(fmc_fmc_driver, FMC_Bank5_6, &sdram_timing, &sdram_config, FMC_ONLY_BANK_5);
    fmc_driver_set_name(fmc_fmc_driver, "FMC");

    sdram_driver_init(&sdram_sdram_driver, fmc_fmc_driver);
    sdram_driver_config(sdram_sdram_driver, SDRAM_SIZE);
    sdram_driver_set_name(sdram_sdram_driver, "SDRAM");
}

static volatile int button_sampling_timer = 0;

void board_periodic_call(void){
    if(button_sampling_timer++ == 100){
        button_driver_periodic(button_user_button_driver);
        button_sampling_timer = 0;
    }
}

void USART2_IRQHandler(void){
    uart_driver_irq_callback(uart_uart2_driver);
}

void ClockInit(void){
    mcu_flash_config(false, true, true, 5);

    clock_ll_hse_en();

    clock_ll_pll_set_src(pll_src_hse);
    clock_ll_pll_set_conf(4, 8, 2, 360, 25);
    clock_ll_pll_en();

    clock_ll_hclk_set_div(1);
    clock_ll_pclk1_set_div(4);
    clock_ll_pclk2_set_div(2);

    clock_ll_sysclk_set_src(sysclk_pllclk);

    SystemCoreClockUpdate();
}
