#include <stdint.h>

extern int main(void);
extern void SystemInit(void);
extern void SystemCoreClockUpdate(void);

typedef void( *const intfunc )( void );

extern uint32_t _textdata;
extern uint32_t _sdata;
extern uint32_t _edata;
extern uint32_t _bss_start;
extern uint32_t _bss_end;
extern uint32_t _estack;

void crt0(void);
void reset_handler(void);
void default_handler(void);
void default_clock_init(void);

void __attribute__ ((weak, alias ("default_clock_init"))) ClockInit(void);

void __attribute__ ((weak, alias ("default_handler"))) NMI_Handler(void);
void __attribute__ ((weak, alias ("default_handler"))) HardFault_Handler(void);
void __attribute__ ((weak, alias ("default_handler"))) MemManage_Handler(void);
void __attribute__ ((weak, alias ("default_handler"))) BusFault_Handler(void);
void __attribute__ ((weak, alias ("default_handler"))) UsageFault_Handler(void);
void __attribute__ ((weak, alias ("default_handler"))) SVC_Handler(void);
void __attribute__ ((weak, alias ("default_handler"))) DebugMon_Handler(void);
void __attribute__ ((weak, alias ("default_handler"))) PendSV_Handler(void);
void __attribute__ ((weak, alias ("default_handler"))) SysTick_Handler(void);

void __attribute__ ((weak, alias ("default_handler"))) WWDG_IRQHandler(void);
void __attribute__ ((weak, alias ("default_handler"))) PVD_IRQHandler(void);
void __attribute__ ((weak, alias ("default_handler"))) TAMP_STAMP_IRQHandler(void);
void __attribute__ ((weak, alias ("default_handler"))) RTC_WKUP_IRQHandler(void);
void __attribute__ ((weak, alias ("default_handler"))) FLASH_IRQHandler(void);
void __attribute__ ((weak, alias ("default_handler"))) RCC_IRQHandler(void);
void __attribute__ ((weak, alias ("default_handler"))) EXTI0_IRQHandler(void);
void __attribute__ ((weak, alias ("default_handler"))) EXTI1_IRQHandler(void);
void __attribute__ ((weak, alias ("default_handler"))) EXTI2_IRQHandler(void);
void __attribute__ ((weak, alias ("default_handler"))) EXTI3_IRQHandler(void);
void __attribute__ ((weak, alias ("default_handler"))) EXTI4_IRQHandler(void);
void __attribute__ ((weak, alias ("default_handler"))) DMA1_Stream0_IRQHandler(void);
void __attribute__ ((weak, alias ("default_handler"))) DMA1_Stream1_IRQHandler(void);
void __attribute__ ((weak, alias ("default_handler"))) DMA1_Stream2_IRQHandler(void);
void __attribute__ ((weak, alias ("default_handler"))) DMA1_Stream3_IRQHandler(void);
void __attribute__ ((weak, alias ("default_handler"))) DMA1_Stream4_IRQHandler(void);
void __attribute__ ((weak, alias ("default_handler"))) DMA1_Stream5_IRQHandler(void);
void __attribute__ ((weak, alias ("default_handler"))) DMA1_Stream6_IRQHandler(void);
void __attribute__ ((weak, alias ("default_handler"))) ADC_IRQHandler(void);
void __attribute__ ((weak, alias ("default_handler"))) CAN1_TX_IRQHandler(void);
void __attribute__ ((weak, alias ("default_handler"))) CAN1_RX0_IRQHandler(void);
void __attribute__ ((weak, alias ("default_handler"))) CAN1_RX1_IRQHandler(void);
void __attribute__ ((weak, alias ("default_handler"))) CAN1_SCE_IRQHandler(void);
void __attribute__ ((weak, alias ("default_handler"))) EXTI9_5_IRQHandler(void);
void __attribute__ ((weak, alias ("default_handler"))) TIM1_BRK_TIM9_IRQHandler(void);
void __attribute__ ((weak, alias ("default_handler"))) TIM1_UP_TIM10_IRQHandler(void);
void __attribute__ ((weak, alias ("default_handler"))) TIM1_TRG_COM_TIM11_IRQHandler(void);
void __attribute__ ((weak, alias ("default_handler"))) TIM1_CC_IRQHandler(void);
void __attribute__ ((weak, alias ("default_handler"))) TIM2_IRQHandler(void);
void __attribute__ ((weak, alias ("default_handler"))) TIM3_IRQHandler(void);
void __attribute__ ((weak, alias ("default_handler"))) TIM4_IRQHandler(void);
void __attribute__ ((weak, alias ("default_handler"))) I2C1_EV_IRQHandler(void);
void __attribute__ ((weak, alias ("default_handler"))) I2C1_ER_IRQHandler(void);
void __attribute__ ((weak, alias ("default_handler"))) I2C2_EV_IRQHandler(void);
void __attribute__ ((weak, alias ("default_handler"))) I2C2_ER_IRQHandler(void);
void __attribute__ ((weak, alias ("default_handler"))) SPI1_IRQHandler(void);
void __attribute__ ((weak, alias ("default_handler"))) SPI2_IRQHandler(void);
void __attribute__ ((weak, alias ("default_handler"))) USART1_IRQHandler(void);
void __attribute__ ((weak, alias ("default_handler"))) USART2_IRQHandler(void);
void __attribute__ ((weak, alias ("default_handler"))) USART3_IRQHandler(void);
void __attribute__ ((weak, alias ("default_handler"))) EXTI15_10_IRQHandler(void);
void __attribute__ ((weak, alias ("default_handler"))) RTC_Alarm_IRQHandler(void);
void __attribute__ ((weak, alias ("default_handler"))) OTG_FS_WKUP_IRQHandler(void);
void __attribute__ ((weak, alias ("default_handler"))) TIM8_BRK_TIM12_IRQHandler(void);
void __attribute__ ((weak, alias ("default_handler"))) TIM8_UP_TIM13_IRQHandler(void);
void __attribute__ ((weak, alias ("default_handler"))) TIM8_TRG_COM_TIM14_IRQHandler(void);
void __attribute__ ((weak, alias ("default_handler"))) TIM8_CC_IRQHandler(void);
void __attribute__ ((weak, alias ("default_handler"))) DMA1_Stream7_IRQHandler(void);
void __attribute__ ((weak, alias ("default_handler"))) FMC_IRQHandler(void);
void __attribute__ ((weak, alias ("default_handler"))) SDIO_IRQHandler(void);
void __attribute__ ((weak, alias ("default_handler"))) TIM5_IRQHandler(void);
void __attribute__ ((weak, alias ("default_handler"))) SPI3_IRQHandler(void);
void __attribute__ ((weak, alias ("default_handler"))) UART4_IRQHandler(void);
void __attribute__ ((weak, alias ("default_handler"))) UART5_IRQHandler(void);
void __attribute__ ((weak, alias ("default_handler"))) TIM6_DAC_IRQHandler(void);
void __attribute__ ((weak, alias ("default_handler"))) TIM7_IRQHandler(void);
void __attribute__ ((weak, alias ("default_handler"))) DMA2_Stream0_IRQHandler(void);
void __attribute__ ((weak, alias ("default_handler"))) DMA2_Stream1_IRQHandler(void);
void __attribute__ ((weak, alias ("default_handler"))) DMA2_Stream2_IRQHandler(void);
void __attribute__ ((weak, alias ("default_handler"))) DMA2_Stream3_IRQHandler(void);
void __attribute__ ((weak, alias ("default_handler"))) DMA2_Stream4_IRQHandler(void);
void __attribute__ ((weak, alias ("default_handler"))) CAN2_TX_IRQHandler(void);
void __attribute__ ((weak, alias ("default_handler"))) CAN2_RX0_IRQHandler(void);
void __attribute__ ((weak, alias ("default_handler"))) CAN2_RX1_IRQHandler(void);
void __attribute__ ((weak, alias ("default_handler"))) CAN2_SCE_IRQHandler(void);
void __attribute__ ((weak, alias ("default_handler"))) OTG_FS_IRQHandler(void);
void __attribute__ ((weak, alias ("default_handler"))) DMA2_Stream5_IRQHandler(void);
void __attribute__ ((weak, alias ("default_handler"))) DMA2_Stream6_IRQHandler(void);
void __attribute__ ((weak, alias ("default_handler"))) DMA2_Stream7_IRQHandler(void);
void __attribute__ ((weak, alias ("default_handler"))) USART6_IRQHandler(void);
void __attribute__ ((weak, alias ("default_handler"))) I2C3_EV_IRQHandler(void);
void __attribute__ ((weak, alias ("default_handler"))) I2C3_ER_IRQHandler(void);
void __attribute__ ((weak, alias ("default_handler"))) OTG_HS_EP1_OUT_IRQHandler(void);
void __attribute__ ((weak, alias ("default_handler"))) OTG_HS_EP1_IN_IRQHandler(void);
void __attribute__ ((weak, alias ("default_handler"))) OTG_HS_WKUP_IRQHandler(void);
void __attribute__ ((weak, alias ("default_handler"))) OTG_HS_IRQHandler(void);
void __attribute__ ((weak, alias ("default_handler"))) DCMI_IRQHandler(void);
void __attribute__ ((weak, alias ("default_handler"))) FPU_IRQHandler(void);
void __attribute__ ((weak, alias ("default_handler"))) SPI4_IRQHandler(void);
void __attribute__ ((weak, alias ("default_handler"))) SAI1_IRQHandler(void);
void __attribute__ ((weak, alias ("default_handler"))) SAI2_IRQHandler(void);
void __attribute__ ((weak, alias ("default_handler"))) QUADSPI_IRQHandler(void);
void __attribute__ ((weak, alias ("default_handler"))) CEC_IRQHandler(void);
void __attribute__ ((weak, alias ("default_handler"))) SPDIF_RX_IRQHandler(void);
void __attribute__ ((weak, alias ("default_handler"))) FMPI2C1_EV_IRQHandler(void);
void __attribute__ ((weak, alias ("default_handler"))) FMPI2C1_ER_IRQHandler(void);

__attribute__ ((section(".vectors"), used))
void (* const g_pfnVectors[])(void) = {
    (intfunc)((uint32_t)&_estack),
    reset_handler,
    NMI_Handler,
    HardFault_Handler,
    MemManage_Handler,
    BusFault_Handler,
    UsageFault_Handler,
    0,
    0,
    0,
    0,
    SVC_Handler,
    DebugMon_Handler,
    0,
    PendSV_Handler,
    SysTick_Handler,
    WWDG_IRQHandler,
    PVD_IRQHandler,
    TAMP_STAMP_IRQHandler,
    RTC_WKUP_IRQHandler,
    FLASH_IRQHandler,
    RCC_IRQHandler,
    EXTI0_IRQHandler,
    EXTI1_IRQHandler,
    EXTI2_IRQHandler,
    EXTI3_IRQHandler,
    EXTI4_IRQHandler,
    DMA1_Stream0_IRQHandler,
    DMA1_Stream1_IRQHandler,
    DMA1_Stream2_IRQHandler,
    DMA1_Stream3_IRQHandler,
    DMA1_Stream4_IRQHandler,
    DMA1_Stream5_IRQHandler,
    DMA1_Stream6_IRQHandler,
    ADC_IRQHandler,
    CAN1_TX_IRQHandler,
    CAN1_RX0_IRQHandler,
    CAN1_RX1_IRQHandler,
    CAN1_SCE_IRQHandler,
    EXTI9_5_IRQHandler,
    TIM1_BRK_TIM9_IRQHandler,
    TIM1_UP_TIM10_IRQHandler,
    TIM1_TRG_COM_TIM11_IRQHandler,
    TIM1_CC_IRQHandler,
    TIM2_IRQHandler,
    TIM3_IRQHandler,
    TIM4_IRQHandler,
    I2C1_EV_IRQHandler,
    I2C1_ER_IRQHandler,
    I2C2_EV_IRQHandler,
    I2C2_ER_IRQHandler,
    SPI1_IRQHandler,
    SPI2_IRQHandler,
    USART1_IRQHandler,
    USART2_IRQHandler,
    USART3_IRQHandler,
    EXTI15_10_IRQHandler,
    RTC_Alarm_IRQHandler,
    OTG_FS_WKUP_IRQHandler,
    TIM8_BRK_TIM12_IRQHandler,
    TIM8_UP_TIM13_IRQHandler,
    TIM8_TRG_COM_TIM14_IRQHandler,
    TIM8_CC_IRQHandler,
    DMA1_Stream7_IRQHandler,
    FMC_IRQHandler,
    SDIO_IRQHandler,
    TIM5_IRQHandler,
    SPI3_IRQHandler,
    UART4_IRQHandler,
    UART5_IRQHandler,
    TIM6_DAC_IRQHandler,
    TIM7_IRQHandler,
    DMA2_Stream0_IRQHandler,
    DMA2_Stream1_IRQHandler,
    DMA2_Stream2_IRQHandler,
    DMA2_Stream3_IRQHandler,
    DMA2_Stream4_IRQHandler,
    0,
    0,
    CAN2_TX_IRQHandler,
    CAN2_RX0_IRQHandler,
    CAN2_RX1_IRQHandler,
    CAN2_SCE_IRQHandler,
    OTG_FS_IRQHandler,
    DMA2_Stream5_IRQHandler,
    DMA2_Stream6_IRQHandler,
    DMA2_Stream7_IRQHandler,
    USART6_IRQHandler,
    I2C3_EV_IRQHandler,
    I2C3_ER_IRQHandler,
    OTG_HS_EP1_OUT_IRQHandler,
    OTG_HS_EP1_IN_IRQHandler,
    OTG_HS_WKUP_IRQHandler,
    OTG_HS_IRQHandler,
    DCMI_IRQHandler,
    0,
    0,
    FPU_IRQHandler,
    0,
    0,
    SPI4_IRQHandler,
    0,
    0,
    SAI1_IRQHandler,
    0,
    0,
    0,
    SAI2_IRQHandler,
    QUADSPI_IRQHandler,
    CEC_IRQHandler,
    SPDIF_RX_IRQHandler,
    FMPI2C1_EV_IRQHandler,
    FMPI2C1_ER_IRQHandler,
};

void crt0(void){
    uint32_t *src, *dst;

    src = &_textdata;
    dst = &_sdata;

    while(dst < &_edata)
        *(dst++) = *(src++);

    dst = &_bss_start;

    while(dst < &_bss_end)
        *(dst++) = 0;

    return;
}

void reset_handler(void){
    crt0();
    SystemInit();
    ClockInit();
    SystemCoreClockUpdate();
    main();
    while(1);
}

void default_handler(void){
    while(1);
}

void default_clock_init(void){
    return;
}
