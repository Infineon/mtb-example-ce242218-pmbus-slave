
/*****************************************************************************
* File Name        : main.c
*
* Description      : This source file contains the main routine for secure
*                    application in the CM33 CPU
*
* Related Document : See README.md
*
********************************************************************************
* (c) 2026, Infineon Technologies AG, or an affiliate of Infineon
* Technologies AG. All rights reserved.
* This software, associated documentation and materials ("Software") is
* owned by Infineon Technologies AG or one of its affiliates ("Infineon")
* and is protected by and subject to worldwide patent protection, worldwide
* copyright laws, and international treaty provisions. Therefore, you may use
* this Software only as provided in the license agreement accompanying the
* software package from which you obtained this Software. If no license
* agreement applies, then any use, reproduction, modification, translation, or
* compilation of this Software is prohibited without the express written
* permission of Infineon.
*
* Disclaimer: UNLESS OTHERWISE EXPRESSLY AGREED WITH INFINEON, THIS SOFTWARE
* IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
* INCLUDING, BUT NOT LIMITED TO, ALL WARRANTIES OF NON-INFRINGEMENT OF
* THIRD-PARTY RIGHTS AND IMPLIED WARRANTIES SUCH AS WARRANTIES OF FITNESS FOR A
* SPECIFIC USE/PURPOSE OR MERCHANTABILITY.
* Infineon reserves the right to make changes to the Software without notice.
* You are responsible for properly designing, programming, and testing the
* functionality and safety of your intended application of the Software, as
* well as complying with any legal requirements related to its use. Infineon
* does not guarantee that the Software will be free from intrusion, data theft
* or loss, or other breaches ("Security Breaches"), and Infineon shall have
* no liability arising out of any Security Breaches. Unless otherwise
* explicitly approved by Infineon, the Software may not be used in any
* application where a failure of the Product or any consequences of the use
* thereof can reasonably be expected to result in personal injury.
*******************************************************************************/

#include "cy_pdl.h"
#include "cybsp.h"
#include "mtb_pmbus.h"
#include "mtb_hal.h"
#include "cy_retarget_io.h"
#include "pmbus_hw_cfg.h"
#include "pmbus_common.h"
#include <stdio.h>

/*******************************************************************************
* Macros
********************************************************************************/
/* These are the addresses where the core0 and core1 images are located. */
#define CORE0_IMAGE_ADDRESS    CYMEM_CM33_0_S_m33s_ppca0_nvm_C_S_START
#define CORE1_IMAGE_ADDRESS    CYMEM_CM33_0_S_m33s_ppca1_nvm_C_S_START
#define PPCA0_IMAGE_SIZE       CYMEM_CM33_0_S_ppca0_code_SIZE
#define PPCA1_IMAGE_SIZE       CYMEM_CM33_0_S_ppca1_code_SIZE

#define PMBUS_PAGE_CMD_CODE               (0x00U)
#define PMBUS_TEST_CMD_1_CODE             (0xD6U)
#define PMBUS_TEST_CMD_1_SIZE             (0x4U)
#define PMBUS_TEST_CMD_2_CODE             (0xD7U)
#define PMBUS_TEST_CMD_2_SIZE             (0x2U)
#define PMBUS_TEST_CMD_3_CODE             (0xC7U)
#define PMBUS_TEST_CMD_3_SIZE             (0x4U)
#define PMBUS_TEST_STATUS_BYTE_CODE       (0x78U)
#define PMBUS_TEST_STATUS_BYTE_SIZE       (0x1U)
#define PMBUS_TEST_STATUS_WORD_CODE       (0x79U)
#define PMBUS_TEST_STATUS_WORD_SIZE       (0x2U)

/*******************************************************************************
* Global Variables
********************************************************************************/
/** [PMBUS_UART_GLOBALS] */
/* Debug UART variables */
static cy_stc_scb_uart_context_t    DEBUG_UART_context; /* UART context */
static mtb_hal_uart_t               DEBUG_UART_hal_obj; /* Debug UART HAL object */
/* [PMBUS_UART_GLOBALS] */

/** [PMBUS_HW_CFG_GLOBALS] */
extern mtb_pmbus_stc_t pmbus_target_inst;
/* [PMBUS_HW_CFG_GLOBALS] */

STATUS_WORD_REG_t status_word;
MTB_PMBUS_CAPABILITY_REG_t capability_data;

/** [PMBUS_GEN_CALLBACK] */
static volatile uint8_t user_led_toggled_cnt = 0U;
/* Data storage */
volatile uint16_t user_data = 0U;
volatile uint32_t user_data1 = 0U;


void pmbus_gen_callback(mtb_pmbus_events_t event);

void pmbus_gen_callback(mtb_pmbus_events_t event)
{
     MTB_PMBUS_LOG_INF("CMD gen match");

    if (event == MTB_PMBUS_QUICK_CMD_WR_EVENT)
    {
        printf("Gen: MTB_PMBUS_QUICK_CMD_WR_EVENT\n\r");
        /* Toggle User LED on quick write command */
        Cy_GPIO_Inv(CYBSP_USER_LED_PORT, CYBSP_USER_LED_PIN);
        user_led_toggled_cnt++;
        printf("User LED toggled\n\r");
    }
}
/* [PMBUS_GEN_CALLBACK] */

/** [PMBUS_CMD1_CALLBACK] */
bool pmbus_cmd1_callback(mtb_pmbus_cmd_events_t event, int32_t page, int32_t phase, uint8_t byte)
{
    /* Avoid compiler warnings */
    (void) page;
    (void) phase;
    (void) byte;

    if (event == MTB_PMBUS_CMD_MATCH)
    {
        MTB_PMBUS_LOG_INF("CMD1 match");
    }
    else if (event == MTB_PMBUS_CMD_READ_REQ)
    {
        Cy_GPIO_Inv(CYBSP_USER_LED_PORT, CYBSP_USER_LED_PIN);
        uint32_t tmp = user_led_toggled_cnt++; /* Put the number of times the User LED was toggled in cmd buffer */
        mtb_pmbus_cmd_update_data_isr(&pmbus_target_inst, PMBUS_TEST_CMD_1_CODE, (uint8_t*)&tmp, PMBUS_TEST_CMD_1_SIZE);
    }

    return true;
}
/* [PMBUS_CMD1_CALLBACK] */

/** [PMBUS_CMD2_CALLBACK] */
bool pmbus_cmd2_callback(mtb_pmbus_cmd_events_t event, int32_t page, int32_t phase, uint8_t byte)
{
    if (event == MTB_PMBUS_CMD_MATCH)
    {
        /* Local copy */
        uint16_t tmp = user_data;
        /* Update the cmd buffer using local data */
        mtb_pmbus_cmd_update_data_ext_isr(&pmbus_target_inst, PMBUS_TEST_CMD_2_CODE, page, phase, (uint8_t*)&tmp, PMBUS_TEST_CMD_2_SIZE);
    }
    else if (event == MTB_PMBUS_CMD_WRITE_DONE)
    {
        uint16_t tmp;
        /* Update the local data using cmd buffer */
        mtb_pmbus_cmd_read_data_ext_isr(&pmbus_target_inst, PMBUS_TEST_CMD_2_CODE, page, phase, (uint8_t*)&tmp, PMBUS_TEST_CMD_2_SIZE);
        /* Copy to storage */
        user_data = tmp;

    }

    return true;
}
/* [PMBUS_CMD2_CALLBACK] */

/** [PMBUS_CMD2_CALLBACK] */
bool pmbus_cmd3_callback(mtb_pmbus_cmd_events_t event, int32_t page, int32_t phase, uint8_t byte)
{
    if (event == MTB_PMBUS_CMD_MATCH)
    {
        /* Local copy */
        uint32_t tmp = user_data1;
        /* Update the cmd buffer using local data */
        mtb_pmbus_cmd_update_data_ext_isr(&pmbus_target_inst, PMBUS_TEST_CMD_3_CODE, page, phase, (uint8_t*)&tmp, PMBUS_TEST_CMD_3_SIZE);
    }
    else if (event == MTB_PMBUS_CMD_WRITE_DONE)
    {
        uint32_t tmp;
        /* Update the local data using cmd buffer */
        mtb_pmbus_cmd_read_data_ext_isr(&pmbus_target_inst, PMBUS_TEST_CMD_3_CODE, page, phase, (uint8_t*)&tmp, PMBUS_TEST_CMD_3_SIZE);
        /* Copy to storage */
        user_data1 = tmp;
    }

    return true;
}
/* [PMBUS_CMD3_CALLBACK] */

bool pmbus_cmd_status_byte_callback(mtb_pmbus_cmd_events_t event, int32_t page, int32_t phase, uint8_t byte)
{
    /* Avoid compiler warnings */
    (void) page;
    (void) phase;
    (void) byte;

    if (event == MTB_PMBUS_CMD_MATCH)
    {
        MTB_PMBUS_LOG_INF("CMD_STATUS_BYTE match");
    }
    else if (event == MTB_PMBUS_CMD_READ_REQ)
    {
        printf("\n\rCMD_STATUS_BYTE read request\n\r");
        mtb_pmbus_cmd_update_data_isr(&pmbus_target_inst, PMBUS_TEST_STATUS_BYTE_CODE, (uint8_t*)&status_word.status_lower_bits, PMBUS_TEST_STATUS_BYTE_SIZE);
    }

    return true;
}

bool pmbus_cmd_status_word_callback(mtb_pmbus_cmd_events_t event, int32_t page, int32_t phase, uint8_t byte)
{
    /* Avoid compiler warnings */
    (void) page;
    (void) phase;
    (void) byte;

    if (event == MTB_PMBUS_CMD_MATCH)
    {
        MTB_PMBUS_LOG_INF("CMD_STATUS_WORD match");
    }
    else if (event == MTB_PMBUS_CMD_READ_REQ)
    {
        uint8_t pmbus_status_word_data[2] = {status_word.status_lower_bits, status_word.status_upper_bits};
        mtb_pmbus_cmd_update_data_isr(&pmbus_target_inst, PMBUS_TEST_STATUS_WORD_CODE, (uint8_t*)&pmbus_status_word_data, PMBUS_TEST_STATUS_WORD_SIZE);
    }

    return true;
}

/** [PMBUS_ZONE_CALLBACK] */
uint8_t pmbus_zone_callback(mtb_pmbus_zone_events_t event, uint8_t mask, int32_t page)
{
    uint8_t status = 0x00U;
    
    if (event == MTB_PMBUS_ZONE_READ_STATUS_BYTE_REQ)
    {
        printf("Zone: Status Byte requested for page %d, mask=0x%02X\n\r", (int)page, mask);
        /* Example: Return lower byte of STATUS_WORD, masked by controller's request */
        status = 0xAA & mask;  /* Replace 0xAA with actual status value */
    }
    else if (event == MTB_PMBUS_ZONE_READ_STATUS_WORD_REQ)
    {
        printf("Zone: Status Word requested for page %d, mask=0x%02X\n\r", (int)page, mask);
        /* Example: Return word status (lower byte sent first), masked by controller */
        status = 0xBB & mask;  /* Replace 0xBB with actual status word low byte */
    }
    else if (event == MTB_PMBUS_ZONE_READ_STATUS_CMPLT)
    {
        printf("Zone: Read Status completed for page %d\n\r", (int)page);
        status = 0x01U;  /* Not used for this event */
    }
    else if (event == MTB_PMBUS_ZONE_READ_STATUS_ARB_LOST)
    {
        printf("Zone: Arbitration lost during Zone Read for page %d\n\r", (int)page);
        status = 0x01U;  /* Not used for this event */
    }
    
    return status;
}
/* [PMBUS_ZONE_CALLBACK] */

void retarget_io_config(void)
{
    cy_rslt_t result;
    cy_en_scb_uart_status_t init_status;

    init_status = Cy_SCB_UART_Init(DEBUG_UART_HW, &DEBUG_UART_config, &DEBUG_UART_context);
    if (init_status!=CY_SCB_UART_SUCCESS)
    {
        CY_ASSERT(0);
    }

    Cy_SCB_UART_Enable(DEBUG_UART_HW);

    result = mtb_hal_uart_setup(&DEBUG_UART_hal_obj, &DEBUG_UART_hal_config, &DEBUG_UART_context, NULL);
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }

    result = cy_retarget_io_init(&DEBUG_UART_hal_obj);
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }

    /* \x1b[2J\x1b[;H - ANSI ESC sequence for clear screen */
    printf("\x1b[2J\x1b[;H");

    printf("Retarget-io is configured\n\r");
}

/*******************************************************************************
* Function Name: main
********************************************************************************
* Summary:
* This is the main function for CM33 CPU. It does...
*    1.
*    2.
*
* Parameters:
*  void
*
* Return:
*  int
*
*******************************************************************************/
int main(void)
{
    cy_rslt_t result;
    /* Initialize the device and board peripherals */
    result = cybsp_init();

    /* Board init failed. Stop program execution */
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }

    /* enable interrupts */
    __enable_irq();

    Cy_System_Init_CPU0((void*)CORE0_IMAGE_ADDRESS, PPCA0_IMAGE_SIZE);
    Cy_System_Init_CPU1((void*)CORE1_IMAGE_ADDRESS, PPCA1_IMAGE_SIZE);

    /** [PMBUS_UART_SETUP] */
    retarget_io_config();

    /* Transmit header to the terminal */
    /* \x1b[2J\x1b[;H - ANSI ESC sequence for clear screen */
    printf("\x1b[2J\x1b[;H");

    printf("************************************************************\r\n");
    printf("PSOC Control C3M/P8: PMBus slave\r\n");
    printf("************************************************************\r\n\n");
    /* [PMBUS_UART_SETUP] */

    /** [PMBUS_ENABLE_IRQ] */
    /* Enable global interrupts */
    __enable_irq();
    /* [PMBUS_ENABLE_IRQ] */

    /** [PMBUS_HW_INIT] */
    init_pmbus_hw();

    /* Log successful initialization */
    MTB_PMBUS_LOG_INF("I2C transport is initialized");

    /* Configure I2C interrupt masks */
    SCB_INTR_S_MASK(myPMBus_0_I2C_HW) = CY_SCB_SLAVE_INTR_I2C_BUS_ERROR |
                                         CY_SCB_SLAVE_INTR_I2C_STOP |
                                         SCB_INTR_S_I2C_RESTART_Msk |
                                         SCB_INTR_S_I2C_START_Msk |
                                         SCB_INTR_S_I2C_ADDR_MATCH_Msk |
                                         CY_SCB_SLAVE_INTR_I2C_ARB_LOST;

    Cy_SCB_SetRxInterruptMask(myPMBus_0_I2C_HW, SCB_INTR_RX_TRIGGER_Msk);
    Cy_SCB_SetI2CInterruptMask(myPMBus_0_I2C_HW, 0U);
    Cy_SCB_SetTxFifoLevel(myPMBus_0_I2C_HW, 1U);
    /* [PMBUS_HW_INIT] */

    /** [PMBUS_INIT_ENABLE] */
    mtb_pmbus_status_t status;

    status = mtb_pmbus_init(&pmbus_target_inst, &myPMBus_0_config);

    if (MTB_PMBUS_STATUS_SUCCESS != status)
    {
        /* Handle the error status */
        MTB_PMBUS_LOG_DBG("PMBus initialization failed!");
    }
    else
    {
        status = mtb_pmbus_enable(&pmbus_target_inst);

        if (MTB_PMBUS_STATUS_SUCCESS != status)
        {
            /* Handle the error status */
        }
    }

    for (;;)
    {
         status_word.status_lower_bits = 4U;
         status_word.status_upper_bits = 8U;
    }
    /* [PMBUS_INIT_ENABLE] */
}
