
/*****************************************************************************
* File Name        : pmbus_hw_cfg.c
*
* Description      : This source file contains the hardware configuration for
*                    the PMBus Middleware.
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
#include "mtb_pmbus.h"
#include "cy_pdl.h"
#include "cybsp.h"

#if defined (PMBUS_SP_ENABLED)

    #define PMBUS_TARGET_I2C_HW myPMBus_0_I2C_HW
    #define PMBUS_TARGET_I2C_CONFIG myPMBus_0_I2C_config
    #define PMBUS_TARGET_I2C_IRQ myPMBus_0_I2C_IRQ

    #ifndef CY_DEVICE_PSC3_P8
        #define PMBUS_TIMEOUT_HW myPMBus_0_TIMEOUT_HW
        #define PMBUS_TIMEOUT_NUM myPMBus_0_TIMEOUT_NUM
        #define PMBUS_TIMEOUT_IRQ myPMBus_0_TIMEOUT_IRQ
        #define PMBUS_TIMEOUT_CONFIG myPMBus_0_TIMEOUT_config

    #else
        #define PMBUS_TARGET_I2C_TGS_CONFIG myPMBus_0_I2C_tgs_config
    #endif

#else
    #define PMBUS_TARGET_I2C_CONFIG PMBUS_TARGET_I2C_config

    #ifndef CY_DEVICE_PSC3_P8
        #define PMBUS_TARGET_I2C_TGS_CONFIG myPMBus_0_I2C_tgs_config        
    #else
        #define PMBUS_TARGET_I2C_TGS_CONFIG PMBUS_TARGET_I2C_tgs_config
    #endif
#endif


/******************************************************************************
 * I2C Hardware Configuration
 */

/** [PMBUS_HW_CFG] */
static cy_stc_scb_i2c_context_t  i2c_target_pdl_context;

mtb_pmbus_stc_t pmbus_target_inst;

#if (defined (MTB_PMBUS_ENABLE_TIMEOUT) && (MTB_PMBUS_ENABLE_TIMEOUT == 1U)) || defined (MTB_PMBUS_DOXYGEN)
#if (defined (MTB_PMBUS_HAL_USE_TCPWM) && (MTB_PMBUS_HAL_USE_TCPWM == 1U)) || defined (MTB_PMBUS_DOXYGEN)

/** [PMBUS_TIMEOUT_TIMER_ISR_CFG] */
void timer_isr(void)
{
    mtb_pmbus_timer_isr(&pmbus_target_inst);
}
/** [PMBUS_TIMEOUT_TIMER_ISR_CFG] */
#endif /* #if (defined (MTB_PMBUS_HAL_USE_TCPWM) && (MTB_PMBUS_HAL_USE_TCPWM == 1U)) */

#if (defined (MTB_PMBUS_HAL_USE_TGS) && (MTB_PMBUS_HAL_USE_TGS == 1U)) || defined (MTB_PMBUS_DOXYGEN)
void init_tgs(void)
{
/** [PMBUS_TIMEOUT_TGS_CFG] */
    cy_en_scb_tgs_status_t pdl_tgs_status;

    pdl_tgs_status = Cy_SCB_TGSx_Init(PMBUS_TARGET_I2C_HW, MTB_PMBUS_TGS_TIMER_NUM, &PMBUS_TARGET_I2C_TGS_CONFIG);
    if (CY_SCB_TGS_SUCCESS != pdl_tgs_status)
    {
        MTB_PMBUS_LOG_ERR("Error during TGS initialization. Status: %X", pdl_tgs_status);
    }
    else
    {
        Cy_SCB_TGSx_Enable(PMBUS_TARGET_I2C_HW, MTB_PMBUS_TGS_TIMER_NUM);
    }
/** [PMBUS_TIMEOUT_TGS_CFG] */
}
#endif /* #if (defined (MTB_PMBUS_HAL_USE_TGS) && (MTB_PMBUS_HAL_USE_TGS == 1U)) */
#endif /* #if (defined (MTB_PMBUS_ENABLE_TIMEOUT) && (MTB_PMBUS_ENABLE_TIMEOUT == 1U)) */
 
void target_i2c_isr(void)
{
    mtb_pmbus_i2c_isr(&pmbus_target_inst);
}

void init_pmbus_hw(void)
{
    cy_en_scb_i2c_status_t pdl_i2c_status;
    cy_en_sysint_status_t  pld_interrupt_status;

    pdl_i2c_status = Cy_SCB_I2C_Init(PMBUS_TARGET_I2C_HW, &PMBUS_TARGET_I2C_CONFIG, &i2c_target_pdl_context);
    if (CY_SCB_I2C_SUCCESS != pdl_i2c_status)
    {
        MTB_PMBUS_LOG_ERR("Error during I2C PDL initialization. Status: %X", pdl_i2c_status);
    }
    else
    {
        cy_stc_sysint_t target_i2c_isr_cfg =
        {
            .intrSrc = PMBUS_TARGET_I2C_IRQ,
            .intrPriority = 3U
        };

        pld_interrupt_status = Cy_SysInt_Init(&target_i2c_isr_cfg, target_i2c_isr);
        if (CY_SYSINT_SUCCESS != pld_interrupt_status)
        {
            MTB_PMBUS_LOG_ERR("Error during I2C Interrupt initialization. Status: %X", pld_interrupt_status);
        }
        else
        {
            MTB_PMBUS_LOG_INF("I2C transport is initialized");
        }
    
    }

#if (defined (MTB_PMBUS_ENABLE_TIMEOUT) && (MTB_PMBUS_ENABLE_TIMEOUT == 1U)) 
#if (defined (MTB_PMBUS_HAL_USE_TCPWM) && (MTB_PMBUS_HAL_USE_TCPWM == 1U))
/** [PMBUS_TIMEOUT_TIMER_CFG] */
    cy_en_tcpwm_status_t pdl_tcpwm_status = Cy_TCPWM_Counter_Init(PMBUS_TIMEOUT_HW, PMBUS_TIMEOUT_NUM, &PMBUS_TIMEOUT_config);
    if (CY_TCPWM_SUCCESS != pdl_tcpwm_status)
    {
        MTB_PMBUS_LOG_ERR("Error during TCPWM PDL initialization. Status: %X", pdl_tcpwm_status);
    }
    else
    {
        cy_stc_sysint_t timer_isr_cfg =
        {
            .intrSrc = PMBUS_TIMEOUT_IRQ,
            .intrPriority = 3U
        };

        pld_interrupt_status = Cy_SysInt_Init(&timer_isr_cfg, timer_isr);
        if (CY_SYSINT_SUCCESS != pld_interrupt_status)
        {
            MTB_PMBUS_LOG_ERR("Error during Timer Interrupt initialization. Status: %X", pld_interrupt_status);
        }
        else
        {
            MTB_PMBUS_LOG_INF("Timer is initialized");
        }
    }
/** [PMBUS_TIMEOUT_TIMER_CFG] */
#endif /* #if (defined (MTB_PMBUS_HAL_USE_TCPWM) && (MTB_PMBUS_HAL_USE_TCPWM == 1U)) */
#if (defined (MTB_PMBUS_HAL_USE_TGS) && (MTB_PMBUS_HAL_USE_TGS == 1U))
    init_tgs();
#endif /* #if (defined (MTB_PMBUS_HAL_USE_TGS) && (MTB_PMBUS_HAL_USE_TGS == 1U)) */
#endif /* #if (defined (MTB_PMBUS_ENABLE_TIMEOUT) && (MTB_PMBUS_ENABLE_TIMEOUT == 1U)) */
}
 
/** [PMBUS_TIMEOUT_TIMER_EN_CALLBACK] */ 
void hw_resource_enable_callback(mtb_pmbus_hw_resources_ctrl_action_t action)
{
    if (action == MTB_PMBUS_HW_RESOURCES_ENABLE)
    {
        Cy_SCB_I2C_Enable(PMBUS_TARGET_I2C_HW);
#if (defined (MTB_PMBUS_ENABLE_TIMEOUT) && (MTB_PMBUS_ENABLE_TIMEOUT == 1U))
#if (defined (MTB_PMBUS_HAL_USE_TCPWM) && (MTB_PMBUS_HAL_USE_TCPWM == 1U))
        Cy_TCPWM_Counter_Enable(PMBUS_TIMEOUT_HW, PMBUS_TIMEOUT_NUM);
#endif /* #if (defined (MTB_PMBUS_HAL_USE_TCPWM) && (MTB_PMBUS_HAL_USE_TCPWM == 1U)) */
#endif /* #if (defined (MTB_PMBUS_ENABLE_TIMEOUT) && (MTB_PMBUS_ENABLE_TIMEOUT == 1U)) */
    }
    else if (action == MTB_PMBUS_HW_RESOURCES_DISABLE)
    {
        Cy_SCB_I2C_Disable(PMBUS_TARGET_I2C_HW, &i2c_target_pdl_context);
#if (defined (MTB_PMBUS_ENABLE_TIMEOUT) && (MTB_PMBUS_ENABLE_TIMEOUT == 1U))
#if (defined (MTB_PMBUS_HAL_USE_TCPWM) && (MTB_PMBUS_HAL_USE_TCPWM == 1U))
        Cy_TCPWM_Counter_Disable(PMBUS_TIMEOUT_HW, PMBUS_TIMEOUT_NUM);
#endif /* #if (defined (MTB_PMBUS_HAL_USE_TCPWM) && (MTB_PMBUS_HAL_USE_TCPWM == 1U)) */
#endif /* #if (defined (MTB_PMBUS_ENABLE_TIMEOUT) && (MTB_PMBUS_ENABLE_TIMEOUT == 1U)) */
    }
}
/** [PMBUS_TIMEOUT_TIMER_EN_CALLBACK] */

/** [PMBUS_TIMEOUT_TIMER_INT_CTRL] */
void target_hw_isr_enable(void)
{
    NVIC_EnableIRQ((IRQn_Type) PMBUS_TARGET_I2C_IRQ);
#if (defined (MTB_PMBUS_ENABLE_TIMEOUT) && (MTB_PMBUS_ENABLE_TIMEOUT == 1U))
#if (defined (MTB_PMBUS_HAL_USE_TCPWM) && (MTB_PMBUS_HAL_USE_TCPWM == 1U))
    NVIC_EnableIRQ((IRQn_Type) PMBUS_TIMEOUT_IRQ);
#endif /* #if (defined (MTB_PMBUS_HAL_USE_TCPWM) && (MTB_PMBUS_HAL_USE_TCPWM == 1U)) */
#endif /* #if (defined (MTB_PMBUS_ENABLE_TIMEOUT) && (MTB_PMBUS_ENABLE_TIMEOUT == 1U)) */
}
 
void target_hw_isr_disable(void)
{
    NVIC_DisableIRQ((IRQn_Type) PMBUS_TARGET_I2C_IRQ);
#if (defined (MTB_PMBUS_ENABLE_TIMEOUT) && (MTB_PMBUS_ENABLE_TIMEOUT == 1U))
#if (defined (MTB_PMBUS_HAL_USE_TCPWM) && (MTB_PMBUS_HAL_USE_TCPWM == 1U))
    NVIC_DisableIRQ((IRQn_Type) PMBUS_TIMEOUT_IRQ);
#endif /* #if (defined (MTB_PMBUS_HAL_USE_TCPWM) && (MTB_PMBUS_HAL_USE_TCPWM == 1U)) */
#endif /* #if (defined (MTB_PMBUS_ENABLE_TIMEOUT) && (MTB_PMBUS_ENABLE_TIMEOUT == 1U)) */
}
/** [PMBUS_TIMEOUT_TIMER_INT_CTRL] */

/** [PMBUS_TIMEOUT_TIMER_HAL_OBJ] */
mtb_pmbus_stc_config_hal_t myPMBus_0_hal_config =
{
    .hw_ptr = PMBUS_TARGET_I2C_HW,
    .pdl_i2c_context = &i2c_target_pdl_context,
#if (defined (MTB_PMBUS_ENABLE_TIMEOUT) && (MTB_PMBUS_ENABLE_TIMEOUT == 1U))
#if (defined (MTB_PMBUS_HAL_USE_TCPWM) && (MTB_PMBUS_HAL_USE_TCPWM == 1U))
    .timeout_tcpwm_base = PMBUS_TIMEOUT_HW,
    .timeout_tcpwm_cntnum = PMBUS_TIMEOUT_NUM,
#endif /* #if (defined (MTB_PMBUS_HAL_USE_TCPWM) && (MTB_PMBUS_HAL_USE_TCPWM == 1U)) */
#endif /* #if (defined (MTB_PMBUS_ENABLE_TIMEOUT) && (MTB_PMBUS_ENABLE_TIMEOUT == 1U)) */
#if (defined (MTB_PMBUS_SUPPORT_SMBALERT) && (MTB_PMBUS_SUPPORT_SMBALERT != 0U))
    .smbalert_port_addr = PMBUS_SMBALERT_PORT,
    .smbalert_pin_num = PMBUS_SMBALERT_PIN,
#endif /* #if (defined (MTB_PMBUS_SUPPORT_SMBALERT) && (MTB_PMBUS_SUPPORT_SMBALERT != 0U)) */
};
/** [PMBUS_TIMEOUT_TIMER_HAL_OBJ] */

void myPMBus_0_hw_resource_ctrl(mtb_pmbus_hw_resources_ctrl_action_t action)
{
    hw_resource_enable_callback(action);
}

void myPMBus_0_hw_irq_enable(void)
{
    target_hw_isr_enable();
}

void myPMBus_0_hw_irq_disable(void)
{
    target_hw_isr_disable();
}

#if defined (__ICCARM__) /* IAR */
__weak void pmbus_gen_callback(mtb_pmbus_events_t event)
#else /* ARM-MDK || GCC */
void __attribute__((weak)) pmbus_gen_callback(mtb_pmbus_events_t event)
#endif
{
    (void) event;
}

void myPMBus_0_gen_callback(mtb_pmbus_events_t event)
{
    pmbus_gen_callback(event);
}

#if defined (__ICCARM__) /* IAR */
__weak void pmbus_error_callback(uint32_t events, uint8_t cmd_code, bool cmd_is_ext)
#else /* ARM-MDK || GCC */
void __attribute__((weak)) pmbus_error_callback(uint32_t events, uint8_t cmd_code, bool cmd_is_ext)
#endif
{
    (void) events;
    (void) cmd_code;
    (void) cmd_is_ext;
}

void myPMBus_0_error_callback(uint32_t events, uint8_t cmd_code, bool cmd_is_ext)
{
    pmbus_error_callback(events, cmd_code, cmd_is_ext);
}

#if (defined(MTB_PMBUS_SUPPORT_ZONE) && (MTB_PMBUS_SUPPORT_ZONE != 0U))
#if defined (__ICCARM__) /* IAR */
__weak uint8_t pmbus_zone_callback(mtb_pmbus_zone_events_t event, uint8_t data_byte, int32_t page)
#else /* ARM-MDK || GCC */
uint8_t __attribute__((weak)) pmbus_zone_callback(mtb_pmbus_zone_events_t event, uint8_t data_byte, int32_t page)
#endif
{
    uint8_t status = 0U;

    (void) event;
    (void) data_byte;
    (void) page;

    return status;
}

uint8_t myPMBus_0_zone_callback(mtb_pmbus_zone_events_t event, uint8_t data_byte, int32_t page)
{
    uint8_t status = 0U;

    status = pmbus_zone_callback(event, data_byte, page);

    return status;
}
#endif /* #if (defined(MTB_PMBUS_SUPPORT_ZONE) && (MTB_PMBUS_SUPPORT_ZONE != 0U)) */
