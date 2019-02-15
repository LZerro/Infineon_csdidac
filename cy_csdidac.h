/***************************************************************************//**
* \file cy_csdidac.h
* \version 1.0
*
* \brief
* This file provides function prototypes and constants specific to the CSDIDAC
* middleware.
*
********************************************************************************
* \copyright
* Copyright 2019, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/
/**
* \mainpage Cypress CSDIDAC Middleware Library
*
* The CSDIDAC provides an API that allows IDAC functionality of the
* CSD HW block. It can be useful for devices that do not include 
* another DAC options. 
*
********************************************************************************
* \section section_csdidac_general General Description
********************************************************************************
*
* The CSD HW block enables multiple sensing capabilities on PSoC devices 
* including self-cap and mutual-cap capacitive touch sensing solution, 
* a 10-bit ADC, IDAC, and Comparator. The CSD driver is a low-level 
* peripheral driver, a wrapper to manage access to the CSD HW block. 
* Each middleware access to the CSD HW block is through the CSD Driver. 
* 
* The CSD HW block can support only one function at a time. However, all  
* supported functionality (like CapSense, CSDADC, CSDIDAC, etc.) can be 
* time-multiplexed in a design. I.e. you can save the existing state 
* of the CapSense middleware, restore the state of the CSDIDAC middleware, 
* perform DAC operations, and then switch back to the CapSense functionality.
* For more details and code examples, refer to the description of the 
* Cy_CSDIDAC_Save() and Cy_CSDIDAC_Restore() functions.
* 
* \image html capsense_solution.png "CapSense Solution" width=800px
* \image latex capsense_solution.png
* 
* This section describes only CSDIDAC middleware. Refer to the corresponding 
* sections for documentation of other middleware supported by the CSD HW block.
* The CSDIDAC library is designed to be used with the CSD driver.
* The application program does not need to interact with the CSD driver 
* and/or other drivers such as GPIO or SysClk directly. All of that is 
* configured and managed by middleware.
* 
* Include cy_csdidac.h to get access to all functions and other declarations 
* in this library.
*
* The Cy_CSDIDAC API is described in the following sections:
* * \ref group_csdidac_macros
* * \ref group_csdidac_data_structures
* * \ref group_csdidac_enums
* * \ref group_csdidac_functions
*
* <b>Features:</b>
* * Two-channel IDAC with 7-bit resolution
* * IDAC A and IDAC B output can be enabled/disabled independently
* * IDAC A and IDAC B output can be configured with sourcing/sinking 
*   current independently
* * 0 to 609.6 uA (609600 nA) current range is available for each IDAC output
* * Each IDAC can use independently one of six available LSB depending 
*   on a desired output current:
*
* <table class="doxtable">
*   <tr><th>LSB Index</th><th>LSB</th><th>Available Current Range</th></tr>
*   <tr>
*     <td>0</td>
*     <td>37.5 nA</td>
*     <td>0 to 4762.5 nA</td>
*   </tr>
*   <tr>
*     <td>1</td>
*     <td>75.0 nA</td>
*     <td>0 to 9525.0 nA</td>
*   </tr>
*   <tr>
*     <td>2</td>
*     <td>0.3 uA</td>
*     <td>0 to  38.1 uA</td>
*   </tr>
*   <tr>
*     <td>3</td>
*     <td>0.6 uA</td>
*     <td>0 to  76.2 uA</td>
*   </tr>
*   <tr>
*     <td>4</td>
*     <td>2.4 uA</td>
*     <td>0 to 304.8 uA</td>
*   </tr>
*   <tr>
*     <td>5</td>
*     <td>4.8 uA</td>
*     <td>0 to 609.6 uA</td>
*   </tr>
* </table>
*
* \section group_csdidac_configuration Configuration Considerations
*
* The CSDIDAC operates on top of the CSD driver. The CSD driver has
* some prerequisites for proper operation. Refer to the "CSD (CapSense
* Sigma Delta)" section of the PDL API Reference Manual.
* In ModusToolbox IDE, the Device Configurator CSD personality should be 
* used for CSDIDAC middleware initial configuration.
* If the user checks the "Enable CSDIDAC" checkbox, all needed 
* configuration parameters appear. The user can choose available
* output channel(s), assign its pins or AmuxBus and set all other 
* parameters needed for CSDIDAC middleware configuration. 
* All the parameters in CSD personality have pop-ups with detailed 
* description.
* If the user does not use ModusToolbox IDE, the user could create 
* a CSDIDAC configuration structure manually by using the API 
* Reference Guide and the configuration structure prototype 
* in the csdidac.h file. 
*
* <b>Building CSDIDAC </b>
*
* The CSDIDAC middleware should be enabled for CM4 core of the PSoC 6 
* by Middleware Selector in ModusToolbox.
* The CSDIDAC middleware using for CM0+ core is not supported in the 
* ModusToolbox, but the user can create CM0+ configuration 
* with CSDIDAC middleware manually.
*
* \note
* If building the project with CSDIDAC outside the ModusToolbox environment, 
* the path to the CSDIDAC middleware should be manually specified 
* in the project settings or in the Makefile.
*
* <b>Initializing CSDIDAC </b>
*
* To initialize a CSDIDAC, the user must declare the CSDIDAC context 
* structure. An example of the CSDIDAC context structure declaration is below:
*
*       cy_stc_csdidac_context_t cy_csdidac_context;
*
* Note that the name "cy_csdidac_context" is shown only for a reference.
* Any other name can be used instead. 
*
* The CSDIDAC configuration structure is generated by the Device
* Configurator CSD personality and should then be passed to 
* the Cy_CSDIDAC_Init() function as well as the context structure.
*
* \section group_csdidac_more_information More Information
* For more information, refer to the following documents:
*
* * <a href="http://www.cypress.com/trm218176"><b>Technical Reference Manual
* (TRM)</b></a>
*
* * <a href="http://www.cypress.com/ds218787"><b>PSoC 63 with BLE Datasheet
* Programmable System-on-Chip datasheet</b></a>
*
* * <a href="http://www.cypress.com/an210781"><b>AN210781 Getting Started
* with PSoC 6 MCU with Bluetooth Low Energy (BLE) Connectivity</b></a>
*
* * <a href="../../capsense_api_reference_manual.html"><b>CapSense MW API 
* Reference</b></a>
*
* * <a href="../../csdadc_api_reference_manual.html"><b>CSDADC MW API 
* Reference</b></a>
*
* * <a href="../../pdl_api_reference_manual/html/group__group__csd.html"><b>CSD 
* Driver API Reference</b></a>
*
* \section group_csdidac_MISRA MISRA-C Compliance
*
* The Cy_CSDIDAC library has the following specific deviations:
*
* <table class="doxtable">
*   <tr>
*     <th>MISRA Rule</th>
*     <th>Rule Class (Required/Advisory)</th>
*     <th>Rule Description</th>
*     <th>Description of Deviation(s)</th>
*   </tr>
*   <tr>
*     <td>11.4</td>
*     <td>A</td>
*     <td>A conversion should not be performed between a pointer to object 
*         and an integer type.</td>
*     <td>Such a conversion is performed with CSDIDAC context
*         in the DeepSleepCallback() function.
*         This case is verified on correct operation.</td>
*   </tr>
*   <tr>
*     <td>1.2</td>
*     <td rowspan=2> R</td>
*     <td rowspan=2> Constant: Dereference of NULL pointer.</td>
*     <td rowspan=2> These violations are reported as a result of using 
*         offset macros of the CSD Driver with corresponding documented 
*         violation 20.6. Refer to the CSD Driver API Reference Guide.</td>
*   </tr>
*   <tr>
*     <td>20.3</td>
*   </tr>
* </table>
*
* \section group_csdidac_changelog Changelog
* <table class="doxtable">
*   <tr><th>Version</th><th>Changes</th><th>Reason for Change</th></tr>
*   <tr>
*     <td>1.0</td>
*     <td>The initial version.</td>
*     <td></td>
*   </tr>
* </table>
*
* \defgroup group_csdidac_macros Macros
* \brief
* This section describes the CSDIDAC macros. These macros can be used for 
* checking a maximum IDAC code and a maximum IDAC output current.
* For detailed information about macros, see each macro description. 
*
* \defgroup group_csdidac_enums Enumerated types
* \brief
* Describes the enumeration types defined by the CSDIDAC. These enumerations 
* can be used for checking CSDIDAC functions' return status,
* for defining a CSDIDAC LSB and polarity, and for choosing IDAC for an 
* operation and defining its states.  For detailed information about
* enumerations, see each enumeration description. 
*
* \defgroup group_csdidac_data_structures Data Structures
* \brief
* Describes the data structures defined by the CSDIDAC. The CSDIDAC 
* middleware use structures for output channel pins, 
* middleware configuration, and context. The pin structure is included 
* in the configuration structure and both of them can be defined by the 
* user with the CSD personality in the Device Configurator or manually 
* if the user doesn't use ModusToolbox.
* The context structure contains a copy of the configuration structure 
* and current CSDIDAC middleware state data. The context 
* structure should be allocated by the user and be passed to all 
* CSDIDAC middleware functions. CSDIDAC middleware structure sizes 
* are shown in the table below:
*
* <table class="doxtable">
*   <tr><th>Structure</th><th>Size in bytes (w/o padding)</th></tr>
*   <tr>
*     <td>cy_stc_csdidac_pin_t</td>
*     <td>5</td>
*   </tr>
*   <tr>
*     <td>cy_stc_csdidac_config_t</td>
*     <td>23</td>
*   </tr>
*   <tr>
*     <td>cy_stc_csdidac_context_t</td>
*     <td>31</td>
*   </tr>
* </table>
*
* \defgroup group_csdidac_functions Functions
* \brief
* This section describes the CSDIDAC Function Prototypes.
*/

/******************************************************************************/
#if !defined(CY_CSDIDAC_H)
#define CY_CSDIDAC_H

#include "cy_device_headers.h"
#include "cy_csd.h"


/* The C binding of definitions if building with the C++ compiler */
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifndef CY_IP_MXCSDV2
    #error "The CSDIDAC middleware is not supported on this device"
#endif

/**
* \addtogroup group_csdidac_macros
* \{
*/
/** Middleware major version */
#define CY_CSDIDAC_MW_VERSION_MAJOR             (1)

/** Middleware minor version */
#define CY_CSDIDAC_MW_VERSION_MINOR             (0)

/** 
* CSDIDAC PDL ID. 
* The user can identify the CSDIDAC MW error codes by this macro. 
*/
#define CY_CSDIDAC_ID                           (CY_PDL_DRV_ID(0x44u))

/** 
* The CSDIDAC max code value. The user should provide the code 
* parameter for the Cy_CSDIDAC_OutputEnableExt() function 
* in range from 0u to CY_CSDIDAC_MAX_CODE.
*/
#define CY_CSDIDAC_MAX_CODE                     (127u)

/** 
* The CSDIDAC max output current value. The user should provide 
* the value of the current parameter for the Cy_CSDIDAC_OutputEnable() 
* function in range from 0 to +/-(CY_CSDIDAC_MAX_CURRENT_NA).
*/
#define CY_CSDIDAC_MAX_CURRENT_NA               (609600uL)

/** \} group_csdidac_macros */

/***************************************
* Enumerated Types and Parameters
***************************************/
/**
* \addtogroup group_csdidac_enums
* \{
*/

/** CSDIDAC return enumeration type */
typedef enum
{
    CY_CSDIDAC_SUCCESS           = (0u),   
                                            /**< The operation executed successfully */
    CY_CSDIDAC_BAD_PARAM         = (CY_CSDIDAC_ID + (uint32_t)CY_PDL_STATUS_ERROR + 1u),
                                            /**< 
                                             * An input parameter is invalid. 
                                             * The user should check whether all
                                             * the input parameters are valid.
                                             */
    CY_CSDIDAC_HW_BUSY           = (CY_CSDIDAC_ID + (uint32_t)CY_PDL_STATUS_ERROR + 2u),
                                            /**< 
                                             * The CSD HW block is busy, 
                                             * i.e. any of current output (A or B) 
                                             * is enabled. 
                                             */
    CY_CSDIDAC_HW_LOCKED         = (CY_CSDIDAC_ID + (uint32_t)CY_PDL_STATUS_ERROR + 3u),
                                            /**< 
                                             * The CSD HW block is acquired and 
                                             * locked by another middleware 
                                             * or application. The CSDIDAC 
                                             * middleware must wait for 
                                             * the CSD HW block to be released
                                             * to acquire it for use. 
                                             */
} cy_en_csdidac_status_t;

/**
* CSDIDAC output current LSB enumeration type. The user can choose 
* LSB when the Cy_CSDIDAC_OutputEnableExt() function is called and
* can check what LSB was choosen by the Cy_CSDIDAC_OutputEnable() 
* function in the cy_stc_csdidac_context_t structure.
*/

typedef enum
{ 
    CY_CSDIDAC_LSB_37_IDX   = 0u,       /**< Index for 37.5 nA LSB */
    CY_CSDIDAC_LSB_75_IDX   = 1u,       /**< Index for 75.0 nA LSB */
    CY_CSDIDAC_LSB_300_IDX  = 2u,       /**< Index for 0.3 uA LSB */
    CY_CSDIDAC_LSB_600_IDX  = 3u,       /**< Index for 0.6 uA LSB */
    CY_CSDIDAC_LSB_2400_IDX = 4u,       /**< Index for 2.4 uA LSB */
    CY_CSDIDAC_LSB_4800_IDX = 5u,       /**< Index for 4.8 uA LSB */
}cy_en_csdidac_lsb_t;

/**
* CSDIDAC polarity enumeration type. The user can choose the polarity 
* when the Cy_CSDIDAC_OutputEnableExt() function is called and can 
* check what a polarity was chosen by the Cy_CSDIDAC_OutputEnable() 
* function in the cy_stc_csdidac_context_t structure.
*/
typedef enum
{
    CY_CSDIDAC_SOURCE       = 0u,       /**< The source polarity */
    CY_CSDIDAC_SINK         = 1u,       /**< The sink polarity */
}cy_en_csdidac_polarity_t;

/**
* CSDIDAC channel enabling enumeration type. The user can check what 
* channel (A or B or both) is currently enabled 
* in the cy_stc_csdidac_context_t structure.
*/
typedef enum
{
    CY_CSDIDAC_DISABLE      = 0u,       /**< The IDAC channel is disabled */
    CY_CSDIDAC_ENABLE       = 1u,       /**< The IDAC channel is enabled */
}cy_en_csdidac_state_t;

/**
* CSDIDAC choosing enumeration type. The user can choose channel A or B 
* to operate with the Cy_CSDIDAC_OutputEnableExt(), Cy_CSDIDAC_OutputDisable(), 
* or Cy_CSDIDAC_OutputEnable() functions.
*/
typedef enum
{
  CY_CSDIDAC_A              = 0uL,      /**< The IDAC A is chosen for an operation */
  CY_CSDIDAC_B              = 1uL,      /**< The IDAC B is chosen for an operation */
} cy_en_csdidac_choice_t;

/** \} group_csdidac_enums */


/***************************************
* Data Structure definitions
***************************************/
/**
* \addtogroup group_csdidac_data_structures
* \{
*/

/** The CSDIDAC pin structure */
typedef struct {
    GPIO_PRT_Type * ioPcPtr;                /**< Pointer to channel IO PC register */
    uint8_t pin;                            /**< Channel IO pin */
} cy_stc_csdidac_pin_t;

/** The CSDIDAC configuration structure */
typedef struct
{
    CSD_Type * base;                        /**< The pointer to the CSD HW Block */
    cy_stc_csd_context_t * csdCxtPtr;       /**< The pointer to the CSD context driver */
    uint32_t periClk;                       /**< PeriClock in Hz */
    cy_en_csdidac_state_t busOnlyA;         /**< The IdacA output connection only to the AmuxBusA enabling */
    const cy_stc_csdidac_pin_t * ptrPinA;   /**< The pointer to the Idac A IO output pin structure */
    cy_en_csdidac_state_t busOnlyB;         /**< The IdacB output connection only to the AmuxBusB enabling */
    const cy_stc_csdidac_pin_t * ptrPinB;   /**< The pointer to the Idac B IO output pin structure */
    uint8_t csdInitTime;                    /**< The CSD HW Block initialization time */
} cy_stc_csdidac_config_t;

/** The CSDIDAC context structure, that contains the internal middleware data */
typedef struct{
    cy_stc_csdidac_config_t cfgCopy;        /**< Configuration structure copy */
    cy_en_csdidac_polarity_t polarityA;     /**< The current IdacA polarity */
    cy_en_csdidac_lsb_t lsbA;               /**< The current IdacA LSB */
    uint8_t codeA;                          /**< The current IdacA code */
    cy_en_csdidac_state_t channelStateA;    /**< The IDAC channel A enabled */
    cy_en_csdidac_polarity_t polarityB;     /**< The current IdacB polarity */
    cy_en_csdidac_lsb_t lsbB;               /**< The current IdacB LSB */
    uint8_t codeB;                          /**< The current IdacB code */
    cy_en_csdidac_state_t channelStateB;    /**< The IDAC channel B enabled */
}cy_stc_csdidac_context_t;

/** \} group_csdidac_data_structures */


/*******************************************************************************
* Function Prototypes
*******************************************************************************/

/**
* \addtogroup group_csdidac_functions
* \{
*/
cy_en_csdidac_status_t Cy_CSDIDAC_Init(
                const cy_stc_csdidac_config_t * config,
                cy_stc_csdidac_context_t * context);
cy_en_csdidac_status_t Cy_CSDIDAC_DeInit(
                cy_stc_csdidac_context_t * context);
cy_en_csdidac_status_t Cy_CSDIDAC_WriteConfig(
                const cy_stc_csdidac_config_t * config,
                cy_stc_csdidac_context_t * context);
cy_en_csdidac_status_t Cy_CSDIDAC_Wakeup(
                const cy_stc_csdidac_context_t * context);
cy_en_syspm_status_t Cy_CSDIDAC_DeepSleepCallback(
                cy_stc_syspm_callback_params_t * callbackParams,
                cy_en_syspm_callback_mode_t mode);
cy_en_csdidac_status_t Cy_CSDIDAC_Save(
                cy_stc_csdidac_context_t * context);
cy_en_csdidac_status_t Cy_CSDIDAC_Restore(
                cy_stc_csdidac_context_t * context);
cy_en_csdidac_status_t Cy_CSDIDAC_OutputEnable(
                cy_en_csdidac_choice_t ch,
                int32_t current,
                cy_stc_csdidac_context_t * context);
cy_en_csdidac_status_t Cy_CSDIDAC_OutputEnableExt(
                cy_en_csdidac_choice_t outputCh,
                cy_en_csdidac_polarity_t polarity,
                cy_en_csdidac_lsb_t lsbIndex,
                uint32_t idacCode,
                cy_stc_csdidac_context_t * context);
cy_en_csdidac_status_t Cy_CSDIDAC_OutputDisable(
                cy_en_csdidac_choice_t ch,
                cy_stc_csdidac_context_t * context);

/** \} group_csdidac_functions */


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CY_CSDIDAC_H */


/* [] END OF FILE */
