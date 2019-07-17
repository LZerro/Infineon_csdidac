# Cypress CSDIDAC Middleware Library 2.0

### What's Included?

Please refer to the [README.md](./README.md) and the [API Reference Guide](https://cypresssemiconductorco.github.io/csdidac/csdidac_api_reference_manual/html/index.html) for a complete description of the CSDIDAC Middleware.
The revision history of the CSDIDAC Middleware is also available on the [API Reference Guide Changelog](https://cypresssemiconductorco.github.io/csdidac/csdidac_api_reference_manual/html/index.html#group_csdidac_changelog).
New in this release:
* Added the new option of joining two IDAC channels 
* Updated CSDIDAC configuration structure to be aligned with the list of parameters in ModusToolbox CSD personality


### Defect Fixes
* Fixing a compilation error for non CSDIDAC-capable devices: CSDIDAC MW sources are enclosed with the conditional compilation
* Renamed periClk to cpuClk in CSDIDAC configuration structure

### Supported Software and Tools
This version of the CSDIDAC Middleware was validated for compatibility with the following Software and Tools:

| Software and Tools                                      | Version |
| :---                                                    | :----:  |
| ModusToolbox Software Environment                       | 2.0     |
| - ModusToolbox Device Configurator                      | 2.0     |
| - ModusToolbox CSD Personality in Device Configurator   | 2.0     |
| PSoC6 Peripheral Driver Library (PDL)                   | 1.2.0   |
| GCC Compiler                                            | 7.2.1   |
| IAR Compiler                                            | 8.32    |
| ARM Compiler 6                                          | 6.11    |
| MBED OS                                                 | 5.13.1  |
| FreeRTOS                                                | 10.0.1  |

### More information
The following resources contain more information:
* [CSDIDAC Middleware RELEASE.md](./RELEASE.md)
* [CSDIDAC Middleware API Reference Guide](https://cypresssemiconductorco.github.io/csdidac/csdidac_api_reference_manual/html/index.html)
* [ModusToolbox Software Environment, Quick Start Guide, Documentation, and Videos](https://www.cypress.com/products/modustoolbox-software-environment)
* [CSDIDAC Middleware Code Example for MBED OS](https://github.com/cypresssemiconductorco)
* [CSDIDAC Middleware Code Examples at GITHUB](https://github.com/cypresssemiconductorco)
* [ModusToolbox Device Configurator Tool Guide](https://www.cypress.com/ModusToolboxDeviceConfig)
* [CapSense Middleware API Reference Guide](https://cypresssemiconductorco.github.io/capsense/capsense_api_reference_manual/html/index.html)
* [CSDADC Middleware API Reference Guide](https://cypresssemiconductorco.github.io/csdadc/csdadc_api_reference_manual/html/index.html)
* [PSoC 6 Technical Reference Manual](https://www.cypress.com/documentation/technical-reference-manuals/psoc-6-mcu-psoc-63-ble-architecture-technical-reference)
* [PSoC 63 with BLE Datasheet Programmable System-on-Chip datasheet](http://www.cypress.com/ds218787)
* [Cypress Semiconductor](http://www.cypress.com)
  
---
© Cypress Semiconductor Corporation, 2019.
