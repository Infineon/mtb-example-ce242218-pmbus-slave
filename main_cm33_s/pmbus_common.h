/*****************************************************************************
* File Name        : pmbus_common.h
*
* Description      :  Provides API declarations for the PMBus Middleware.
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

#ifndef PMBUS_COMMON_H_
#define PMBUS_COMMON_H_

#include "mtb_pmbus.h"
#include "pmbus_hw_cfg.h"
/**
 *  Configuration data structure of STATUS_BYTE and STATUS_WORD register bit definitions.
 *  Needed to be updated by the PMBus node as defined in the PMBus specification and to be exchanged with
 *  PMBus host/device on demand.
 *  Lower status byte or upper status byte or complete status word can be exchanged between Host and device.
 */
typedef struct STATUS_WORD_REG
{
  union
  {
     struct
     {
      uint32_t none_of_the_above   :1; /**< 0 A fault or warning not listed in bits [7:1] of this byte has occurred*/
      uint32_t cml                 :1; /**< 1 A communications, memory or logic fault has occurred*/
      uint32_t temperature         :1; /**< 2 A temperature fault or warning has occurred*/
      uint32_t vin_uv              :1; /**< 3 An input under voltage fault has occurred*/
      uint32_t iout_oc             :1; /**< 4 An output over current fault has occurred*/
      uint32_t vout_ov             :1; /**< 5 An output over voltage fault has occurred */
      uint32_t off                 :1; /**< 6 This bit is asserted if the unit is not providing power to the output,
                                              regardless of the reason, including simply not being enabled. */
      uint32_t busy                :1; /**< 7 A fault was declared because the device was busy and unable to respond.*/
    };
    uint8_t  status_lower_bits;
  };

  union
  {
     struct
     {
      uint32_t unknown        :1; /**< 8 */
      uint32_t other          :1; /**< 9 */
      uint32_t fans           :1; /**< 10 A fan or airflow fault or warning has occurred */
      uint32_t power_good     :1; /**< 11 The POWER_GOOD signal, if present, is negated */
      uint32_t mfr            :1; /**< 12 manufacturer specific fault */
      uint32_t input          :1; /**< 13 An input voltage, input current, or input power fault or
                                        warning has occurred */
      uint32_t iout_pout      :1; /**< 14 An output current or output power fault or warning has occurred */
      uint32_t vout           :1; /**< 15 An output voltage fault or warning has occurred */
    };
    uint8_t  status_upper_bits;
  };
}STATUS_WORD_REG_t;

/**
 *  Configuration data structure of CAPABILITY_REG register bit definitions.
 *  Needed to be updated by the PMBus device as defined in the PMBus Spec and to be exchanged with PMBus host on
 *  demand.
 */
typedef struct MTB_PMBUS_CAPABILITY_REG
{
  union
  {
    struct
    {
      uint32_t rsvd1         :2; /**< 0:1 Reserved */
      uint32_t avsbus_sup    :1; /**< 2 AVSBus Supported/Not supported */
      uint32_t numeric_form  :1; /**< 3 Numeric Data format LINEAR11 or DIRECT format */
      uint32_t smb_alert     :1; /**< 4 Support for SMBus Alert Response protocol */
      uint32_t max_bus_speed :2; /**< 5:6 Maximum supported bus speed */
      uint32_t pec           :1; /**< 7 Packet Error Checking support */
    };
    uint8_t capability_reg;
  };
} MTB_PMBUS_CAPABILITY_REG_t;

#endif /* PMBUS_COMMON_H_ */
