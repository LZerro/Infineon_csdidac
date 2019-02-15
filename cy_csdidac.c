/***************************************************************************//**
* \file cy_csdidac.c
* \version 1.0
*
* \brief
* This file provides the CSD HW block IDAC functionality implementation.
*
********************************************************************************
* \copyright
* Copyright 2019, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/
#include "cy_device_headers.h"
#include "cy_syslib.h"
#include "cy_syspm.h"
#include "cy_csdidac.h"
#include "cy_gpio.h"
#include "cy_csd.h"

/*******************************************************************************
* Function Prototypes - internal functions
*******************************************************************************/
/**
* \cond SECTION_CSDIDAC_INTERNAL
* \addtogroup group_csdidac_internal
* \{
*/
static void Cy_CSDIDAC_DisconnectChannelA(cy_stc_csdidac_context_t * context);
static void Cy_CSDIDAC_DisconnectChannelB(cy_stc_csdidac_context_t * context);
/** \}
* \endcond */


/*******************************************************************************
* Local definition
*******************************************************************************/
#define CY_CSDIDAC_FSM_ABORT                        (0x08u)

/* Idac configuration register */
/* +--------+---------------+-------------------------------------------------------------------+
 * |  BITS  |   FIELD       |             DEFAULT MODE                                          |
 * |--------|---------------|-------------------------------------------------------------------|
 * | 6:0    | VAL           | 0x00(Set IDAC value to "0")                                       |
 * | 7      | POL_STATIC    | 0x00(Set static IDAC polarity)                                    |
 * | 9:8    | POLARITY      | 0x00(IDAC polarity SOURCE)                                        |
 * | 11:10  | BAL_MODE      | 0x00(IDAC is enabled in PHI2 and disabled at the end of balancing)|
 * | 17:16  | LEG1_MODE     | 0x00(Configure LEG1 to GP_static mode)                            |
 * | 19:18  | LEG2_MODE     | 0x00(Configure LEG1 to GP_static mode)                            |
 * | 21     | DSI_CTRL_EN   | 0x00(IDAC DSI control is disabled)                                |
 * | 23:22  | RANGE         | 0x00(Set range parameter value to LOW: 1LSB = 37.5 nA)            |
 * | 24     | LEG1_EN       | 0x00(Output for LEG1 is disabled)                                 |
 * | 25     | LEG2_EN       | 0x00(Output for LEG2 is disabled)                                 |
 * +--------+---------------+--------------------------------------------------------------------+*/
#define CY_CSDIDAC_DEFAULT_CFG                      (0x01800000uL)
#define CY_CSDIDAC_POLARITY_POS                     (8uL)
#define CY_CSDIDAC_POLARITY_MASK                    (3uL << CY_CSDIDAC_POLARITY_POS)
#define CY_CSDIDAC_LSB_POS                          (22uL)
#define CY_CSDIDAC_LSB_MASK                         (3uL << CY_CSDIDAC_LSB_POS)
#define CY_CSDIDAC_LEG1_EN_POS                      (24uL)
#define CY_CSDIDAC_LEG1_EN_MASK                     (1uL << CY_CSDIDAC_LEG1_EN_POS)
#define CY_CSDIDAC_LEG2_EN_POS                      (25uL)
#define CY_CSDIDAC_LEG2_EN_MASK                     (1uL << CY_CSDIDAC_LEG2_EN_POS)
#define CY_CSDIDAC_RANGE_MASK                       (CY_CSDIDAC_LSB_MASK | CY_CSDIDAC_LEG1_EN_MASK | CY_CSDIDAC_LEG2_EN_MASK)

/* 
* All defines below correspond to IDAC LSB in nA. As the 
* lowest LSB is 37.5 nA, its define is increased for 10 times.
* This is taken into account when IDAC code is calculated.
*/
#define CY_CSDIDAC_LSB_37                           (375u)
#define CY_CSDIDAC_LSB_75                           (75u)
#define CY_CSDIDAC_LSB_300                          (300u)
#define CY_CSDIDAC_LSB_600                          (600u)
#define CY_CSDIDAC_LSB_2400                         (2400u)
#define CY_CSDIDAC_LSB_4800                         (4800u)

#define CY_CSDIDAC_CODE_MASK                        (127u)
#define CY_CSDIDAC_CONST_2                          (2u)
#define CY_CSDIDAC_CONST_10                         (10u)

/* CSD HW block CONFIG register definitions */
#define CY_CSDIDAC_CSD_REG_CONFIG_INIT              (0x80001000uL)
#define CY_CSDIDAC_CSD_REG_CONFIG_DEFAULT           (CY_CSDIDAC_CSD_REG_CONFIG_INIT)

/* CSD_INTR register masks */
#define CY_CSDIDAC_CSD_INTR_SAMPLE_MSK              (0x00000001uL)
#define CY_CSDIDAC_CSD_INTR_INIT_MSK                (0x00000002uL)
#define CY_CSDIDAC_CSD_INTR_ADC_RES_MSK             (0x00000100uL)
#define CY_CSDIDAC_CSD_INTR_ALL_MSK                 (CY_CSDIDAC_CSD_INTR_SAMPLE_MSK | \
                                                     CY_CSDIDAC_CSD_INTR_INIT_MSK | \
                                                     CY_CSDIDAC_CSD_INTR_ADC_RES_MSK)
/* CSD_INTR_MASK register masks */
#define CY_CSDIDAC_CSD_INTR_MASK_SAMPLE_MSK         (0x00000001uL)
#define CY_CSDIDAC_CSD_INTR_MASK_INIT_MSK           (0x00000002uL)
#define CY_CSDIDAC_CSD_INTR_MASK_ADC_RES_MSK        (0x00000100uL)
#define CY_CSDIDAC_CSD_INTR_MASK_CLEAR_MSK          (0x00000000uL)

/* Switch definitions */
#define CY_CSDIDAC_SW_BYPA_ENABLE                   (0x00001000uL)
#define CY_CSDIDAC_SW_BYPB_ENABLE                   (0x00010000uL)
#define CY_CSDIDAC_SW_REFGEN_SEL_IBCB_ON            (0x00000010uL)

#define CY_CSDIDAC_CSD_CONFIG_DEFAULT  {\
    .config         = 0x80001000uL,\
    .spare          = 0x00000000uL,\
    .status         = 0x00000000uL,\
    .statSeq        = 0x00000000uL,\
    .statCnts       = 0x00000000uL,\
    .statHcnt       = 0x00000000uL,\
    .resultVal1     = 0x00000000uL,\
    .resultVal2     = 0x00000000uL,\
    .adcRes         = 0x00000000uL,\
    .intr           = 0x00000000uL,\
    .intrSet        = 0x00000000uL,\
    .intrMask       = 0x00000000uL,\
    .intrMasked     = 0x00000000uL,\
    .hscmp          = 0x00000000uL,\
    .ambuf          = 0x00000000uL,\
    .refgen         = 0x00000000uL,\
    .csdCmp         = 0x00000000uL,\
    .swRes          = 0x00000000uL,\
    .sensePeriod    = 0x00000000uL,\
    .senseDuty      = 0x00000000uL,\
    .swHsPosSel     = 0x00000000uL,\
    .swHsNegSel     = 0x00000000uL,\
    .swShieldSel    = 0x00000000uL,\
    .swAmuxbufSel   = 0x00000000uL,\
    .swBypSel       = 0x00000000uL,\
    .swCmpPosSel    = 0x00000000uL,\
    .swCmpNegSel    = 0x00000000uL,\
    .swRefgenSel    = CY_CSDIDAC_SW_REFGEN_SEL_IBCB_ON,\
    .swFwModSel     = 0x00000000uL,\
    .swFwTankSel    = 0x00000000uL,\
    .swDsiSel       = 0x00000000uL,\
    .ioSel          = 0x00000000uL,\
    .seqTime        = 0x00000000uL,\
    .seqInitCnt     = 0x00000000uL,\
    .seqNormCnt     = 0x00000000uL,\
    .adcCtl         = 0x00000000uL,\
    .seqStart       = 0x00000000uL,\
    .idacA          = 0x00000000uL,\
    .idacB          = 0x00000000uL,\
    }

    
/*******************************************************************************
* Function Name: Cy_CSDIDAC_Init
****************************************************************************//**
*
* Captures the CSD HW block and configures it to the default state. 
* This function is called by the application program prior to calling 
* any other middleware function.
*
* Initializes the CSDIDAC middleware. Acquires, locks, and initializes 
* the CSD HW block by using the low-level CSD driver.
* The function performs the following tasks:
* * Verifies input parameters. The CY_CSDIDAC_BAD_PARAM is returned if
*   verification fails.
* * Acquires and locks the CSD HW block for use of the CSDIDAC, if the CSD HW
*   block is in a free state.
* * If the CSD HW block is acquired, it is initialized with a default
*   configuration of the CSDIDAC middleware. Output pins are not connected to
*   the CSD HW block. Outputs are disabled and CY_CSDIDAC_SUCCESS is returned.
* 
* To connect an output pin and enable an output current the 
* Cy_CSDIDAC_OutputEnable() or Cy_CSDIDAC_OutputEnableExt() functions 
* should be used.
* If the CSD HW block is unavailable, CY_CSDIDAC_HW_BUSY status is returned,
* and the CSDIDAC middleware must wait for the CSD HW block to be in idle
* state to initialize.
*
* \param config
* The pointer to the configuration structure \ref cy_stc_csdidac_config_t that 
* contains the initial configuration data of the CSDIDAC MW, generated 
* by the CSD personality of the ModusToolbox Device Configurator tool.
*
* \param context
* The pointer to the CSDIDAC context structure \ref cy_stc_csdidac_context_t
* passed by the user. After the initialization, this structure will contain 
* both CSDIDAC configuration and internal data. It is used during the whole 
* CSDIDAC operation.
*
* \return
* The function returns the status of its operation.
* * CY_CSDIDAC_SUCCESS   - The initialization successfully completed.
* * CY_CSDIDAC_HW_LOCKED - The CSD HW block is already in use by another
*                          middleware. CSDIDAC middleware must wait until 
*                          the CSD HW block returns to the idle state to 
*                          initialize it. The initialization is not completed.
* * CY_CSDIDAC_BAD_PARAM - Input pointers are NULL or invalid. 
*                          The initialization is not completed.
*
*******************************************************************************/
cy_en_csdidac_status_t Cy_CSDIDAC_Init(
                const cy_stc_csdidac_config_t * config,
                cy_stc_csdidac_context_t * context)
{
    cy_en_csdidac_status_t result = CY_CSDIDAC_BAD_PARAM;

    if ((NULL != config) && (NULL != context))
    {
        /* Copy the configuration structure to the context */
        context->cfgCopy = *config;
        /* Capture the CSD HW block for the IDAC functionality */
        result = Cy_CSDIDAC_Restore(context);
        if (CY_CSDIDAC_SUCCESS == result)
        {
            /* Disconnect all CSDIDAC channels */
            Cy_CSDIDAC_DisconnectChannelA(context);
            Cy_CSDIDAC_DisconnectChannelB(context);
            /* Wake-up the CSD HW block */
            (void)Cy_CSDIDAC_Wakeup(context);
        }
        else
        {
            result = CY_CSDIDAC_HW_LOCKED;
        }
    }

    return (result);
}


/*******************************************************************************
* Function Name: Cy_CSDIDAC_DeInit
****************************************************************************//**
*
* Stops the middleware operation and releases the CSD HW block.
*
* If any output channel is enabled, it will be disabled and disconnected.
* 
* After the CSDIDAC middleware is stopped, the CSD HW block may be 
* reconfigured by the application program or other middleware for any 
* other usage. 
* 
* When the middleware operation is stopped by the Cy_CSDIDAC_DeInit()
* function, a subsequent call of the Cy_CSDIDAC_Init() function repeats the
* initialization process. However, to implement time-multiplexed mode 
* (sharing the CSD HW Block between multiple middleware), 
* the Cy_CSDIDAC_Save() and Cy_CSDIDAC_Restore() functions should be used 
* instead of the Cy_CSDIDAC_DeInit() and Cy_CSDIDAC_Init() functions.
*
* \param context
* The pointer to the CSDIDAC context structure \ref cy_stc_csdidac_context_t.
*
* \return
* The function returns the status of its operation.
* * CY_CSDIDAC_SUCCESS   - De-Initialization successfully completed.
* * CY_CSDIDAC_BAD_PARAM - The input pointer is null. De-initialization
*                          is not completed.
* * CY_CSDIDAC_HW_LOCKED - The CSD HW block is already in use by another
*                          Middleware or application. CSDIDAC middleware 
*                          no long owns the specified CSD HW block. 
*                          In other words, de-initialization may have 
*                          been previously completed.
*
*******************************************************************************/
cy_en_csdidac_status_t Cy_CSDIDAC_DeInit(cy_stc_csdidac_context_t * context)
{
    return (Cy_CSDIDAC_Save(context));
}


/*******************************************************************************
* Function Name: Cy_CSDIDAC_WriteConfig
****************************************************************************//**
*
* Updates the CSDIDAC middleware with the desired configuration.
*
* This function sets the desired CSDIDAC middleware configuration.
* The function performs the following:
* * Verifies input parameters 
* * Verifies whether the CSD HW block is captured by the CSDIDAC middleware 
*   and that there are no active IDAC outputs.
* * Initializes the CSD HW block registers with data passed through the
*   config parameter of this function if the above verifications are
*   successful 
* * Disconnects outputs and sets the CSD HW block to the default state for
*   CSDIDAC operations. To enable output(s), the user should call
*   Cy_CSDIDAC_OutputEnable() later.
* * Returns status code regarding the function execution result 
*
* \param config
* The pointer to the CSDIDAC configuration structure to be updated.
*
* \param context
* The pointer to the CSDIDAC context structure \ref cy_stc_csdidac_context_t.
*
* \return
* The function returns the status of its operation.
* * CY_CSDIDAC_SUCCESS   - The operation is completed successfully.
* * CY_CSDIDAC_BAD_PARAM - A context pointer or config pointer
*                          is equal to NULL. The operation was not completed.
* * CY_CSDIDAC_HW_BUSY   - Any IDAC output is enabled. The operation can not 
*                          be completed.
* * CY_CSDIDAC_HW_LOCKED - The CSD HW block is not captured by CSDIDAC 
*                          middleware.
*
*******************************************************************************/
cy_en_csdidac_status_t Cy_CSDIDAC_WriteConfig(
                const cy_stc_csdidac_config_t * config,
                cy_stc_csdidac_context_t * context)
{
    cy_en_csdidac_status_t result = CY_CSDIDAC_BAD_PARAM;
    uint32_t swBypSelValue = 0u;
    
    if ((NULL != config) && (NULL != context))
    {
        if (CY_CSD_IDAC_KEY == Cy_CSD_GetLockStatus(context->cfgCopy.base, context->cfgCopy.csdCxtPtr))
        {
            if ((CY_CSDIDAC_DISABLE == context->channelStateA) && (CY_CSDIDAC_DISABLE == context->channelStateB))
            {
                /* Copy the configuration structure to the context */
                context->cfgCopy = *config;
                
                /* Configure CSDIDAC middleware with a new configuration */
                if ((NULL != context->cfgCopy.ptrPinA) ||  (CY_CSDIDAC_ENABLE == context->cfgCopy.busOnlyA))
                {
                    swBypSelValue |= CY_CSDIDAC_SW_BYPA_ENABLE;
                }
                if ((NULL != context->cfgCopy.ptrPinB) ||  (CY_CSDIDAC_ENABLE == context->cfgCopy.busOnlyB))
                {
                    swBypSelValue |= CY_CSDIDAC_SW_BYPB_ENABLE;
                }
                Cy_CSD_WriteReg(context->cfgCopy.base, CY_CSD_REG_OFFSET_SW_BYP_SEL, swBypSelValue);
                result = CY_CSDIDAC_SUCCESS;
            }
            else
            {
                result = CY_CSDIDAC_HW_BUSY;
            }
        }
        else
        {
            result = CY_CSDIDAC_HW_LOCKED;
        }
    }

    return (result);
}


/*******************************************************************************
* Function Name: Cy_CSDIDAC_Wakeup
****************************************************************************//**
*
* Provide a delay required for the CSD HW block to settle after a wakeup
* from Deep Sleep.
*
* This function provides a delay after exiting Deep Sleep. In Deep Sleep 
* power mode, the CSD HW is powered off and an extra delay is required 
* to establish correct operation of the CSD HW block.
*
* \param context
* The pointer to the CSDIDAC context structure \ref cy_stc_csdidac_context_t.
*
* \return
* The function returns the status of its operation.
* * CY_CSDIDAC_SUCCESS   - The operation is completed successfully.
* * CY_CSDIDAC_BAD_PARAM - A context pointer is equal to NULL.
*                          The operation was not completed.
*
*******************************************************************************/
cy_en_csdidac_status_t Cy_CSDIDAC_Wakeup(const cy_stc_csdidac_context_t * context)
{
    cy_en_csdidac_status_t result = CY_CSDIDAC_BAD_PARAM;

    if (NULL != context)
    {
        Cy_SysLib_DelayUs((uint16_t)context->cfgCopy.csdInitTime);
        result = CY_CSDIDAC_SUCCESS;
    }

    return (result);
}


/*******************************************************************************
* Function Name: Cy_CSDIDAC_DeepSleepCallback
****************************************************************************//**
*
* Callback to prepare the CSDIDAC before entering Deep Sleep.
*
* This function handles the Active to Deep Sleep power mode transition
* for the CSDIDAC middleware.
* Calling this function directly from the application program is not 
* recommended. Instead, Cy_SysPm_DeepSleep() should be used for 
* the Active to Deep Sleep power mode transition of the device.
*
* For proper operation of the CSDIDAC middleware during the Active to
* Deep Sleep mode transition, a callback to this API should be registered
* using the Cy_SysPm_RegisterCallback() function with CY_SYSPM_DEEPSLEEP
* type. After the callback is registered, this function is called by the
* Cy_SysPm_DeepSleep() function to prepare the middleware for the device
* power mode transition.
*
* When this function is called with CY_SYSPM_CHECK_READY as an input, this
* function returns CY_SYSPM_SUCCESS if no output is enabled. Otherwise,
* CY_SYSPM_FAIL is returned. If CY_SYSPM_FAIL status is returned, a device
* cannot change the power mode. To provide such a transition, the application
* program should disable all the enabled IDAC outputs.
*
* \param callbackParams
* Refer to the description of the cy_stc_syspm_callback_params_t type in the
* Peripheral Driver Library documentation.
*
* \param mode
* Refer to the description of the cy_en_syspm_callback_mode_t type in the
* Peripheral Driver Library documentation.
*
* \return
* Returns the status of the operation requested by the mode parameter:
* * CY_SYSPM_SUCCESS  - Deep Sleep power mode can be entered.
* * CY_SYSPM_FAIL     - Deep Sleep power mode cannot be entered.
*
*******************************************************************************/
cy_en_syspm_status_t Cy_CSDIDAC_DeepSleepCallback(
                cy_stc_syspm_callback_params_t * callbackParams,
                cy_en_syspm_callback_mode_t mode)
{
    cy_en_syspm_status_t retVal = CY_SYSPM_SUCCESS;
    cy_stc_csdidac_context_t * csdIdacCxt = (cy_stc_csdidac_context_t *) callbackParams->context;

    if (CY_SYSPM_CHECK_READY == mode)
    { /* Actions that should be done before entering the Deep Sleep mode */
        if ((CY_CSD_IDAC_KEY == Cy_CSD_GetLockStatus(csdIdacCxt->cfgCopy.base, csdIdacCxt->cfgCopy.csdCxtPtr)) &&
           ((CY_CSDIDAC_ENABLE == csdIdacCxt->channelStateA) || (CY_CSDIDAC_ENABLE == csdIdacCxt->channelStateB)))
        {
            retVal = CY_SYSPM_FAIL;
        }
    }

    return(retVal);
}


/*******************************************************************************
* Function Name: Cy_CSDIDAC_Save
****************************************************************************//**
*
* Saves the state of the CSDIDAC middleware so the functionality can be
* restored later.
*
* This function, along with Cy_CSDIDAC_Restore(), is specifically designed
* to support time multiplexing of the CSD HW block between multiple
* middleware. When the CSD HW block is shared by two or more middleware,
* this function can be used to save the current state of CSDIDAC middleware and
* the CSD HW block prior to releasing the CSD HW block for use by another
* middleware.
* 
* This function performs the following operations:
* * Saves the current configuration of the CSD HW block and CSDIDAC middleware.
* * Configures output pins to the default state and disconnects them from 
* the CSD HW block. Releases the CSD HW block.
*
* \param context
* The pointer to the CSDIDAC context structure \ref cy_stc_csdidac_context_t.
*
* \return
* The function returns the status of its operation.
* * CY_CSDIDAC_SUCCESS   - Save operation successfully completed.
* * CY_CSDIDAC_BAD_PARAM - Input pointer is null or invalid. The operation is
*                          not completed.
* * CY_CSDIDAC_HW_LOCKED - The CSD HW block is already in use by another 
*                          middleware. CSDIDAC middleware cannot save state 
*                          without initialization or restore operation.
*
*******************************************************************************/
cy_en_csdidac_status_t Cy_CSDIDAC_Save(cy_stc_csdidac_context_t * context)
{
    cy_en_csdidac_status_t result = CY_CSDIDAC_BAD_PARAM;
    cy_en_csd_status_t initStatus = CY_CSD_LOCKED;

    if (NULL != context)
    {
        /* Release the HW CSD block */
        initStatus = Cy_CSD_DeInit(context->cfgCopy.base, CY_CSD_IDAC_KEY, context->cfgCopy.csdCxtPtr);

        if (CY_CSD_SUCCESS == initStatus)
        {
            /* Disconnect output channels pins from analog busses */
            Cy_CSDIDAC_DisconnectChannelA(context);
            Cy_CSDIDAC_DisconnectChannelB(context);

            result = CY_CSDIDAC_SUCCESS;
        }
        else
        {
            result = CY_CSDIDAC_HW_LOCKED;
        }
    }

    return result;
}


/*******************************************************************************
* Function Name: Cy_CSDIDAC_Restore
****************************************************************************//**
*
* Resumes the middleware operation if the Cy_CSDIDAC_Save() function was
* called previously.
*
* This function, along with the Cy_CSDIDAC_Save() function, is specifically 
* designed for ease of use and supports time multiplexing of the CSD HW block 
* among multiple middleware. When the CSD HW block is shared by two or more
* middleware, this function can be used to restore the previous state of 
* the CSD HW block and the CSDIDAC middleware saved using the 
* Cy_CSDIDAC_Save() function. 
* 
* This function performs the part tasks of the Cy_CSDIDAC_Init() function, 
* namely captures the CSD HW block. Use the Cy_CSDIDAC_Save() and 
* Cy_CSDIDAC_Restore() functions to implement time-multiplexed mode 
* instead of using the Cy_CSDIDAC_DeInit() and Cy_CSDIDAC_Init() functions.
*
* \param context
* The pointer to the CSDIDAC middleware context structure \ref cy_stc_csdidac_context_t.
*
* \return
* The function returns the status of its operation.
* * CY_CSDIDAC_SUCCESS   - Restore operation successfully completed.
* * CY_CSDIDAC_BAD_PARAM - Input pointers are null or invalid; initialization
*                          incomplete.
* * CY_CSDIDAC_HW_BUSY   - The CSD HW block is already in use and the CSDIDAC
*                          middleware cannot restore the state 
*                          without an initialization.
* * CY_CSDIDAC_HW_LOCKED - The CSD HW block is acquired and locked 
*                          by another middleware or an application. 
*                          The CSDIDAC middleware must wait for the CSD HW block 
*                          to be released to acquire it for use.
*
*******************************************************************************/
cy_en_csdidac_status_t Cy_CSDIDAC_Restore(cy_stc_csdidac_context_t * context)
{
    uint32_t watchdogCounter;

    cy_en_csdidac_status_t result = CY_CSDIDAC_BAD_PARAM;
    cy_en_csd_key_t mvKey;
    cy_en_csd_status_t initStatus = CY_CSD_LOCKED;
    CSD_Type * ptrCsdBaseAdd = context->cfgCopy.base;
    cy_stc_csd_context_t * ptrCsdCxt = context->cfgCopy.csdCxtPtr;
    cy_stc_csd_config_t csdCfg = CY_CSDIDAC_CSD_CONFIG_DEFAULT;
    /* An approximate duration of the watchdog waiting loop in cycles*/
    const uint32_t intrInitLoopDuration = 5uL;
    /* An initial watchdog timeout in seconds */
    const uint32_t initWatchdogTimeS = 1uL;

    if (NULL != context)
    {
        /* Get the CSD HW block status */
        mvKey = Cy_CSD_GetLockStatus(ptrCsdBaseAdd, ptrCsdCxt);
        if(CY_CSD_NONE_KEY == mvKey)
        {
            initStatus = Cy_CSD_GetConversionStatus(ptrCsdBaseAdd, ptrCsdCxt);
            if(CY_CSD_BUSY == initStatus)
            {
                Cy_CSD_WriteReg(ptrCsdBaseAdd, CY_CSD_REG_OFFSET_INTR_MASK, CY_CSDIDAC_CSD_INTR_MASK_CLEAR_MSK);
                Cy_CSD_WriteReg(ptrCsdBaseAdd, CY_CSD_REG_OFFSET_SEQ_START, CY_CSDIDAC_FSM_ABORT);

                /* Initialize Watchdog Counter to prevent a hang */
                watchdogCounter = (initWatchdogTimeS * context->cfgCopy.periClk) / intrInitLoopDuration;
                while((CY_CSD_BUSY == initStatus) && (0u != watchdogCounter))
                {
                    initStatus = Cy_CSD_GetConversionStatus(ptrCsdBaseAdd, ptrCsdCxt);
                    watchdogCounter--;
                }
            }

            if ((NULL != context->cfgCopy.ptrPinA) ||  (CY_CSDIDAC_ENABLE == context->cfgCopy.busOnlyA))
            {
                csdCfg.swBypSel |= CY_CSDIDAC_SW_BYPA_ENABLE;
            }
            if ((NULL != context->cfgCopy.ptrPinB) ||  (CY_CSDIDAC_ENABLE == context->cfgCopy.busOnlyB))
            {
                csdCfg.swBypSel |= CY_CSDIDAC_SW_BYPB_ENABLE;
            }

            /* Capture the CSD HW block for the IDAC functionality */
            initStatus = Cy_CSD_Init(ptrCsdBaseAdd, &csdCfg, CY_CSD_IDAC_KEY, ptrCsdCxt);

            if (CY_CSD_SUCCESS == initStatus)
            {
                result = CY_CSDIDAC_SUCCESS;
            }
            else
            {
                result = CY_CSDIDAC_HW_BUSY;
            }
        }
        else
        {
            result = CY_CSDIDAC_HW_LOCKED;
        }
    }
    return (result);
}


/*******************************************************************************
* Function Name: Cy_CSDIDAC_OutputEnable
****************************************************************************//**
*
* Enables an IDAC output with a specified current.
*
* This function performs the following:
* * Verifies input parameters.
* * Identifies an LSB and an IDAC code required to generate the specified 
*   output current and configures the CSD HW block accordingly.
* * Configures and enables specified output of CSDIDAC and returns 
*   status code.
*
* \param ch
* The CSDIDAC supports total of two output (A and B), this parameter 
* specifies output to be enabled.
*
* \param current
* A current value for an IDAC output in nA with a sign. If the parameter is
* positive, then a sourcing current will be generated. If the parameter is
* negative, then the sinking current will be generated. The middleware will
* identify LSB and code values required to achieve the specified output current.
* The middleware will choose the minimum possible LSB to generate the current
* to minimize a quantization error. The user should note the quantization
* error in the output current based on LSB size (LSB should be 37.5/
* 75/300/600/2400/4800 nA). For instance, if this function
* is called to set 123456 nA, the actual output current will be rounded
* to nearest value of multiple to 2400 nA, i.e 122400 nA. An absolute 
* value of this parameter should be in the range from 0x00u 
* to \ref CY_CSDIDAC_MAX_CURRENT_NA.
*
* \param context
* The pointer to the CSDIDAC middleware context structure \ref cy_stc_csdidac_context_t.
*
* \return
* The function returns the status of its operation.
* * CY_CSDIDAC_SUCCESS   - The operation is performed successfully.
* * CY_CSDIDAC_BAD_PARAM - A context pointer is equal to NULL.
*                          The operation was not performed.
*
*******************************************************************************/
cy_en_csdidac_status_t Cy_CSDIDAC_OutputEnable(
                cy_en_csdidac_choice_t ch,
                int32_t current,
                cy_stc_csdidac_context_t * context)
{
    cy_en_csdidac_status_t retVal = CY_CSDIDAC_SUCCESS;
    cy_en_csdidac_polarity_t polarity= CY_CSDIDAC_SOURCE;
    cy_en_csdidac_lsb_t lsb;
    uint32_t tmpLsb;
    uint32_t absCurrent = (0 > current) ? (uint32_t)(-current) : (uint32_t)current;
    uint32_t code;

    if((NULL != context) && (CY_CSDIDAC_MAX_CURRENT_NA >= absCurrent))
    {
        /* Choose a desired current polarity */
        if (0 > current)
        {
            polarity = CY_CSDIDAC_SINK;
        }
        /* Choose IDAC LSB and calculate IDAC code with rounding */
        tmpLsb = absCurrent / CY_CSDIDAC_MAX_CODE;
        if (tmpLsb > CY_CSDIDAC_LSB_2400)
        {
            lsb = CY_CSDIDAC_LSB_4800_IDX;
            code = (absCurrent + (CY_CSDIDAC_LSB_4800 >> 1u)) / CY_CSDIDAC_LSB_4800;
        }
        else if (tmpLsb > CY_CSDIDAC_LSB_600)
        {
            lsb = CY_CSDIDAC_LSB_2400_IDX;
            code = (absCurrent + (CY_CSDIDAC_LSB_2400 >> 1u)) / CY_CSDIDAC_LSB_2400;
        }
        else if (tmpLsb > CY_CSDIDAC_LSB_300)
        {
            lsb = CY_CSDIDAC_LSB_600_IDX;
            code = (absCurrent + (CY_CSDIDAC_LSB_600 >> 1u)) / CY_CSDIDAC_LSB_600;
        }
        else if (tmpLsb > CY_CSDIDAC_LSB_75)
        {
            lsb = CY_CSDIDAC_LSB_300_IDX;
            code = (absCurrent + (CY_CSDIDAC_LSB_300 >> 1u)) / CY_CSDIDAC_LSB_300;
        }
        else if((tmpLsb * CY_CSDIDAC_CONST_10) > CY_CSDIDAC_LSB_37)
        {
            lsb = CY_CSDIDAC_LSB_75_IDX;
            code = (absCurrent + (CY_CSDIDAC_LSB_75 >> 1u)) / CY_CSDIDAC_LSB_75;
        }
        else
        {
            lsb = CY_CSDIDAC_LSB_37_IDX;
            code = ((absCurrent * CY_CSDIDAC_CONST_10) + (CY_CSDIDAC_LSB_37 >> 1u)) / CY_CSDIDAC_LSB_37;
        }
        if (code > CY_CSDIDAC_MAX_CODE)
        {
            code = CY_CSDIDAC_MAX_CODE;
        }

        /* Set desired IDAC(s) polarity, LSB and code in the CSD block and connect output(s) */
        retVal = Cy_CSDIDAC_OutputEnableExt(ch, polarity, lsb, code, context);
    }
    else
    {
        retVal = CY_CSDIDAC_BAD_PARAM;
    }

    return(retVal);
}


/*******************************************************************************
* Function Name: Cy_CSDIDAC_OutputEnableExt
****************************************************************************//**
*
* Enables an IDAC output with specified polarity, LSB, and IDAC code.
*
* This function performs the following:
* * Verifies input parameters.
* * Configures and enables specified output of CSDIDAC and returns status code.
*
* \param outputCh
* CSDIDAC supports total of two output, this parameter specifies output
* that needs to be enabled.
*
* \param polarity
* The polarity to be set for the specified IDAC.
*
* \param lsbIndex
* The LSB to be set for the specified IDAC.
*
* \param idacCode
* Code value for the specified IDAC. Should be in the range from 0 u
* to \ref CY_CSDIDAC_MAX_CODE.
*
* \param context
* The pointer to the CSDIDAC middleware context structure \ref cy_stc_csdidac_context_t.
*
* \return
* The function returns the status of its operation.
* * CY_CSDIDAC_SUCCESS   - The operation performed successfully.
* * CY_CSDIDAC_BAD_PARAM - A context pointer is equal to NULL or other
*                          input parameter is not valid.
*                          The operation was not performed.
*
*******************************************************************************/
cy_en_csdidac_status_t Cy_CSDIDAC_OutputEnableExt(
                cy_en_csdidac_choice_t outputCh,
                cy_en_csdidac_polarity_t polarity,
                cy_en_csdidac_lsb_t lsbIndex,
                uint32_t idacCode,
                cy_stc_csdidac_context_t * context)
{
    CSD_Type * ptrCsdBaseAdd = context->cfgCopy.base;
    uint32_t idacRegOffset;
    uint32_t idacRegValue;
    cy_en_csdidac_status_t retVal = CY_CSDIDAC_SUCCESS;

    if((NULL != context) && (CY_CSDIDAC_MAX_CODE >= idacCode))
    {
        if ((CY_CSDIDAC_A == outputCh) && (NULL == context->cfgCopy.ptrPinA) &&
                              (CY_CSDIDAC_ENABLE != context->cfgCopy.busOnlyA))
        {
            retVal = CY_CSDIDAC_BAD_PARAM;
        }
        if ((CY_CSDIDAC_B == outputCh) && (NULL == context->cfgCopy.ptrPinB) &&
                              (CY_CSDIDAC_ENABLE != context->cfgCopy.busOnlyB))
        {
            retVal = CY_CSDIDAC_BAD_PARAM;
        }

        if (CY_CSDIDAC_BAD_PARAM != retVal)
        {
            if (CY_CSDIDAC_A == outputCh)
            {
                /* Set IDAC A polarity, LSB and code in the context structure */
                context->polarityA = polarity;
                context->lsbA = lsbIndex;
                context->codeA = (uint8_t)idacCode;
                idacRegOffset = CY_CSD_REG_OFFSET_IDACA;
                context->channelStateA = CY_CSDIDAC_ENABLE;
            }
            else
            {
                /* Set IDAC B polarity, LSB and code in the context structure */
                context->polarityB = polarity;
                context->lsbB = lsbIndex;
                context->codeB = (uint8_t)idacCode;
                idacRegOffset = CY_CSD_REG_OFFSET_IDACB;
                context->channelStateB = CY_CSDIDAC_ENABLE;
            }
            idacRegValue = idacCode | (((uint32_t)polarity) << CY_CSDIDAC_POLARITY_POS);
            /* Set IDAC LSB. LSB value is equal lsbIndex divided by 2 */
            idacRegValue |= ((((uint32_t)lsbIndex) >> 1uL) << CY_CSDIDAC_LSB_POS);
            /* Set IDAC leg1 enabling bit */
            idacRegValue |= ((uint32_t)CY_CSDIDAC_LEG1_EN_MASK);
            /* Set IDAC leg2 enabling bit if the lsbIndex is even */
            if (0u != (lsbIndex % CY_CSDIDAC_CONST_2))
            {
                idacRegValue |= ((uint32_t)CY_CSDIDAC_LEG2_EN_MASK);
            }

            /* Write desired IDAC polarity, LSB and code in the CSD block */
            Cy_CSD_WriteReg(ptrCsdBaseAdd, idacRegOffset, idacRegValue);
            /* Connect pin if it is configured */
            if ((CY_CSDIDAC_A == outputCh) && (CY_CSDIDAC_ENABLE != context->cfgCopy.busOnlyA))
            {
                /* Connect AMuxBusA to the selected port */
                Cy_GPIO_SetHSIOM(context->cfgCopy.ptrPinA->ioPcPtr,
                                 (uint32_t)context->cfgCopy.ptrPinA->pin,
                                 HSIOM_SEL_AMUXA);
            }
            if ((CY_CSDIDAC_B == outputCh) && (CY_CSDIDAC_ENABLE != context->cfgCopy.busOnlyB))
            {
                /* Connect AMuxBusA to the selected port */
                Cy_GPIO_SetHSIOM(context->cfgCopy.ptrPinB->ioPcPtr,
                                 (uint32_t)context->cfgCopy.ptrPinB->pin,
                                 HSIOM_SEL_AMUXB);
            }
        }
    }
    else
    {
        retVal = CY_CSDIDAC_BAD_PARAM;
    }

    return(retVal);
}


/*******************************************************************************
* Function Name: Cy_CSDIDAC_DisconnectChannelA
****************************************************************************//**
*
* Disconnects the output channel A pin, if it is configured.
*
* \param context
* The pointer to the CSDIDAC middleware context structure \ref cy_stc_csdidac_context_t.
*
*******************************************************************************/
static void Cy_CSDIDAC_DisconnectChannelA(cy_stc_csdidac_context_t * context)
{
    CSD_Type * ptrCsdBaseAdd = context->cfgCopy.base;
    uint32_t idacRegOffset = CY_CSD_REG_OFFSET_IDACA;

    /* Disable desired IDAC */
    Cy_CSD_WriteReg(ptrCsdBaseAdd, idacRegOffset, 0uL);
    /* Disconnect AMuxBusA from the selected pin, if configured */
    if ((CY_CSDIDAC_ENABLE != context->cfgCopy.busOnlyA) && (NULL != context->cfgCopy.ptrPinA))
    {
        Cy_GPIO_SetHSIOM(context->cfgCopy.ptrPinA->ioPcPtr, (uint32_t)context->cfgCopy.ptrPinA->pin, HSIOM_SEL_GPIO);
    }
    /* Set IDAC state in the context structure to disabled */
    context->channelStateA = CY_CSDIDAC_DISABLE;


}


/*******************************************************************************
* Function Name: Cy_CSDIDAC_DisconnectChannelB
****************************************************************************//**
*
* Disconnects the output channel B pin, if it is configured.
*
* \param context
* The pointer to the CSDIDAC middleware context structure \ref cy_stc_csdidac_context_t.
*
*******************************************************************************/
static void Cy_CSDIDAC_DisconnectChannelB(cy_stc_csdidac_context_t * context)
{
    CSD_Type * ptrCsdBaseAdd = context->cfgCopy.base;
    uint32_t idacRegOffset = CY_CSD_REG_OFFSET_IDACB;

    /* Disable desired IDAC */
    Cy_CSD_WriteReg(ptrCsdBaseAdd, idacRegOffset, 0uL);
    /* Disconnect AMuxBusB from the selected pin, if configured */
    if ((CY_CSDIDAC_ENABLE != context->cfgCopy.busOnlyB) && (NULL != context->cfgCopy.ptrPinB))
    {
        Cy_GPIO_SetHSIOM(context->cfgCopy.ptrPinB->ioPcPtr, (uint32_t)context->cfgCopy.ptrPinB->pin, HSIOM_SEL_GPIO);
    }
    /* Set IDAC state in the context structure to disabled */
    context->channelStateB = CY_CSDIDAC_DISABLE;
}


/*******************************************************************************
* Function Name: Cy_CSDIDAC_OutputDisable
****************************************************************************//**
*
* Disables a specified IDAC output.
*
* The function performs the following:
* * Verifies input parameters.
* * Disables specified output of CSDIDAC and returns the status code.
*
* \param ch
* The channel to disconnect.
*
* \param context
* The pointer to the CSDIDAC middleware context structure \ref cy_stc_csdidac_context_t.
*
* \return
* The function returns the status of its operation.
* * CY_CSDIDAC_SUCCESS   - The operation performed successfully.
* * CY_CSDIDAC_BAD_PARAM - A context pointer is equal to NULL.
*                          The operation was not performed.
*
*******************************************************************************/
cy_en_csdidac_status_t Cy_CSDIDAC_OutputDisable(
                cy_en_csdidac_choice_t ch,
                cy_stc_csdidac_context_t * context)
{
    cy_en_csdidac_status_t retVal = CY_CSDIDAC_SUCCESS;

    if(NULL != context)
    {
        if (CY_CSDIDAC_A == ch)
        {
            Cy_CSDIDAC_DisconnectChannelA(context);
        }
        if (CY_CSDIDAC_B == ch)
        {
            Cy_CSDIDAC_DisconnectChannelB(context);
        }
    }
    else
    {
        retVal = CY_CSDIDAC_BAD_PARAM;
    }

    return(retVal);
}


/* [] END OF FILE */
