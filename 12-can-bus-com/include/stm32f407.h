#ifndef STM32F407_H
#define STM32F407_H

#include <stdint.h>
typedef struct {
    volatile uint32_t MODER;    // Offset 0x00
    volatile uint32_t OTYPER;   // Offset 0x04
    volatile uint32_t OSPEEDR;  // Offset 0x08
    volatile uint32_t PUPDR;    // Offset 0x0C
    volatile uint32_t IDR;      // Offset 0x10
    volatile uint32_t ODR;      // Offset 0x14
    volatile uint32_t BSRR;     // Offset 0x18
    volatile uint32_t LCKR;     // Offset 0x1C
    volatile uint32_t AFR[2];   // Offset 0x20-0x24
} GPIO_TypeDef;

typedef struct {
    volatile uint32_t CR;           // Offset 0x00
    volatile uint32_t PLLCFGR;      // Offset 0x04
    volatile uint32_t CFGR;         // Offset 0x08
    volatile uint32_t CIR;          // Offset 0x0C
    volatile uint32_t AHB1RSTR;     // Offset 0x10
    volatile uint32_t AHB2RSTR;     // Offset 0x14
    volatile uint32_t AHB3RSTR;     // Offset 0x18
    volatile uint32_t FILL_0;       // Reserved
    volatile uint32_t APB1RSTR;     // Offset 0x20
    volatile uint32_t APB2RSTR;     // Offset 0x24
    volatile uint32_t FILL_1[2];    // Reserved
    volatile uint32_t AHB1ENR;      // Offset 0x30
    volatile uint32_t AHB2ENR;      // Offset 0x34
    volatile uint32_t AHB3ENR;      // Offset 0x38
    volatile uint32_t FILL_2;       // Reserved
    volatile uint32_t APB1ENR;      // Offset 0x40
    volatile uint32_t APB2ENR;      // Offset 0x44
    volatile uint32_t FILL_3[2];    // Reserved
    volatile uint32_t AHB1LPENR;    // Offset 0x50
    volatile uint32_t AHB2LPENR;    // Offset 0x54
    volatile uint32_t AHB3LPENR;    // Offset 0x58
    volatile uint32_t FILL_4;       // Reserved
    volatile uint32_t APB1LPENR;    // Offset 0x60
    volatile uint32_t APB2LPENR;    // Offset 0x64
    volatile uint32_t FILL_5[2];    // Reserved
    volatile uint32_t BDCR;         // Offset 0x70
    volatile uint32_t CSR;          // Offset 0x74
    volatile uint32_t FILL_6[2];    // Reserved
    volatile uint32_t SSCGR;        // Offset 0x80
    volatile uint32_t PLLI2SCFGR;   // Offset 0x84
    volatile uint32_t PLLSAICFGR;   // Offset 0x88
    volatile uint32_t DCKCFGR;      // Offset 0x8C
} RCC_TypeDef;

typedef struct {
    volatile uint32_t SR;   // Offset 0x00
    volatile uint32_t DR;   // Offset 0x04
    volatile uint32_t BRR;  // Offset 0x08
    volatile uint32_t CR1;  // Offset 0x0C
    volatile uint32_t CR2;  // Offset 0x10
    volatile uint32_t CR3;  // Offset 0x14
    volatile uint32_t GTPR; // Offset 0x18
} USART_TypeDef;

typedef struct {
    volatile uint32_t CR1;      // Offset 0x00
    volatile uint32_t CR2;      // Offset 0x04
    volatile uint32_t SR;       // Offset 0x08
    volatile uint32_t DR;       // Offset 0x0C
    volatile uint32_t CRCPR;    // Offset 0x10
    volatile uint32_t RXCRCR;   // Offset 0x14
    volatile uint32_t TXCRCR;   // Offset 0x18
    volatile uint32_t I2SCFGR;  // Offset 0x1C
    volatile uint32_t I2SPR;    // Offset 0x20
} SPI_TypeDef;

typedef struct {
    volatile uint32_t SR;       // Offset 0x00
    volatile uint32_t CR1;      // Offset 0x04
    volatile uint32_t CR2;      // Offset 0x08
    volatile uint32_t SMPR1;    // Offset 0x0C
    volatile uint32_t SMPR2;    // Offset 0x10
    volatile uint32_t JOFR1;    // Offset 0x14
    volatile uint32_t JOFR2;    // Offset 0x18
    volatile uint32_t JOFR3;    // Offset 0x1C
    volatile uint32_t JOFR4;    // Offset 0x20
    volatile uint32_t HTR;      // Offset 0x24
    volatile uint32_t LTR;      // Offset 0x28
    volatile uint32_t SQR1;     // Offset 0x2C
    volatile uint32_t SQR2;     // Offset 0x30
    volatile uint32_t SQR3;     // Offset 0x34
    volatile uint32_t JSQR;     // Offset 0x38
    volatile uint32_t JDR1;     // Offset 0x3C
    volatile uint32_t JDR2;     // Offset 0x40
    volatile uint32_t JDR3;     // Offset 0x44
    volatile uint32_t JDR4;     // Offset 0x48
    volatile uint32_t DR;       // Offset 0x4C
} ADC_TypeDef;

typedef struct {
    volatile uint32_t CSR;  // Offset 0x00
    volatile uint32_t CCR;  // Offset 0x04
    volatile uint32_t CDR;  // Offset 0x08
} ADC_Common_TypeDef;

typedef struct {
    volatile uint32_t CR;       // Offset 0x00
    volatile uint32_t NDTR;     // Offset 0x04
    volatile uint32_t PAR;      // Offset 0x08
    volatile uint32_t M0AR;     // Offset 0x0C
    volatile uint32_t M1AR;     // Offset 0x10
    volatile uint32_t FCR;      // Offset 0x14
} DMA_Stream_TypeDef;

typedef struct {
    volatile uint32_t LISR;     // Offset 0x00
    volatile uint32_t HISR;     // Offset 0x04
    volatile uint32_t LIFCR;    // Offset 0x08
    volatile uint32_t HIFCR;    // Offset 0x0C
} DMA_TypeDef;

typedef struct {
    volatile uint32_t KR;   // Offset 0x00
    volatile uint32_t PR;   // Offset 0x04
    volatile uint32_t RLR;  // Offset 0x08
    volatile uint32_t SR;   // Offset 0x0C
} IWDG_TypeDef;

typedef struct {
    volatile uint32_t TIR;     // Offset 0x00
    volatile uint32_t TDTR;    // Offset 0x04
    volatile uint32_t TDLR;    // Offset 0x08
    volatile uint32_t TDHR;    // Offset 0x0C
} CAN_TX_Mailbox_TypeDef;

typedef struct {
    volatile uint32_t RIR;      // Offset 0x00
    volatile uint32_t RDTR;      // Offset 0x04
    volatile uint32_t RDLR;      // Offset 0x08
    volatile uint32_t RDHR;      // Offset 0x0C
} CAN_FIFO_Mailbox_TypeDef;

typedef struct {
    volatile uint32_t FR1;  // Offset 0x00
    volatile uint32_t FR2;  // Offset 0x04
} CAN_Filter_Register_TypeDef;

typedef struct {
    volatile uint32_t MCR;                  // Offset 0x000
    volatile uint32_t MSR;                  // Offset 0x004
    volatile uint32_t TSR;                  // Offset 0x008
    volatile uint32_t RF0R;                 // Offset 0x00C
    volatile uint32_t RF1R;                 // Offset 0x010
    volatile uint32_t IER;                  // Offset 0x014
    volatile uint32_t ESR;                  // Offset 0x018
    volatile uint32_t BTR;                  // Offset 0x01C
    volatile uint32_t FILL_0[88];           // Reserved
    CAN_TX_Mailbox_TypeDef TX[3];           // Offset 0x180
    CAN_FIFO_Mailbox_TypeDef RX[2];         // Offset 0x1B0
    volatile uint32_t FILL_1[12];           // Reserved
    volatile uint32_t FMR;                  // Offset 0x200
    volatile uint32_t FM1R;                 // Offset 0x204
    volatile uint32_t FILL_2;               // Reserved
    volatile uint32_t FS1R;                 // Offset 0x20C
    volatile uint32_t FILL_3;               // Reserved
    volatile uint32_t FFA1R;                // Offset 0x214
    volatile uint32_t FILL_4;               // Reserved
    volatile uint32_t FA1R;                 // Offset 0x21C
    volatile uint32_t FILL_5[8];            // Reserved
    CAN_Filter_Register_TypeDef FR[28];     // Offest 0x240
} CAN_TypeDef;

#define DMA2 ((DMA_TypeDef *) 0x40026400)
#define DMA2_Stream0 ((DMA_Stream_TypeDef *) 0x40026410)

#define RCC ((RCC_TypeDef *) 0x40023800)

#define GPIOA ((GPIO_TypeDef *) 0x40020000)
#define GPIOB ((GPIO_TypeDef *) 0x40020400)
#define GPIOC ((GPIO_TypeDef *) 0x40020800)
#define GPIOD ((GPIO_TypeDef *) 0x40020C00)
#define GPIOE ((GPIO_TypeDef *) 0x40021000)
#define GPIOF ((GPIO_TypeDef *) 0x40021400)
#define GPIOG ((GPIO_TypeDef *) 0x40021800)
#define GPIOH ((GPIO_TypeDef *) 0x40021C00)
#define GPIOI ((GPIO_TypeDef *) 0x40022000)
//#define GPIOJ ((GPIO_TypeDef *) 0x40022400)
//#define GPIOK ((GPIO_TypeDef *) 0x40022800)

#define SPI1 ((SPI_TypeDef *) 0x40013000)

#define ADC_COMMON ((ADC_Common_TypeDef *) 0x40012300)
#define ADC1 ((ADC_TypeDef *) 0x40012000)

#define USART2 ((USART_TypeDef *) 0x40004400)

#define IWDG ((IWDG_TypeDef *) 0x40003000)

#define CAN1_BASE   (0x40006400UL)
#define CAN1        ((CAN_TypeDef *) CAN1_BASE)


// Cortex-M4 Defines
typedef struct {
    volatile uint32_t filler[3];
    volatile uint32_t DEMCR;        // Debug Exception and Monitor Contorl Register
} CoreDebug_TypeDef;

typedef struct {
    volatile uint32_t CTRL;     // Control Register - bit 0 enables (CYCCNTENA) enables cycle counter (offset 0x00)
    volatile uint32_t CYCCNT;   // Cycle Count Register - 32-bit free-running CPU cycle counter (offset 0x04)
} DWT_TypeDef;

#define CoreDebug ((CoreDebug_TypeDef *) 0xE000EDF0)
#define DWT ((DWT_TypeDef *) 0xE0001000)

#endif