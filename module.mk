################################################################################
# \file module.mk
# \version 2.0
#
# \brief
# IDAC functionality using the CSD hardware, constructed to work on top of the 
# CSD driver.
#
################################################################################
# \copyright
# Copyright 2018-2019, Cypress Semiconductor Corporation.  All rights reserved.
# You may use this file only in accordance with the license, terms, conditions,
# disclaimers, and limitations in the end user license agreement accompanying
# the software package with which this file was provided.
################################################################################

ifeq ($(WHICHFILE),true)
$(info Processing $(lastword $(MAKEFILE_LIST)) file from directory $(PWD))
$(info Path: $(MAKEFILE_LIST))
endif

#
# Needed by describe goal processing
#
ifeq ($(MAKECMDGOALS),describe)
ifndef PLATFORMS_VERSION
PLATFORMS_VERSION=1.0
endif
include $(CYSDK)/libraries/platforms-$(PLATFORMS_VERSION)/common/swcomps.mk
endif

#
# The internal tag name of the software component
#
MIDDLEWARE_CSDIDAC_NAME=MIDDLEWARE_CSDIDAC

#
# If defined, the list of legal PLATFORM values for this component.
# If not defined, this component is valid for all values of PLATFORM
#
CY_SUPPORTED_PLATFORM_LIST=PSOC6_DUAL_CORE PSoC6_cm4_dual PSoC6_cm0p PSOC6_SINGLE_CORE PSoC6_cm4_single $(PSOC_EXTRA_PLATFORMS)

#
# A list of UDD devices that this component can be used with.
#
CY_SUPPORTED_DEVICE_LIST=$(CY_DEVICES_WITH_CAPSENSE)

#
# If defined, the list of legal TOOLCHAIN values for this component.  If not
# defined, this component is valid for all values of TOOLCHAIN
#
#CY_SUPPORTED_TOOLCHAIN_LIST=

#
# Used by the IDE to group and categorize components.
#
CY_COMP_CATEGORY=Middleware

#
# The displayed human readable name of the component
#
CY_COMP_NAME_DISPLAY=CSD IDAC

#
# The name in the form of an identifier ([a-z_][a-z0-9_]*).
# Used to generate directories in the IDE.
# 
CY_COMP_NAME_ID=middlewareCsdIdac

#
# The human readable description of the component
#
CY_COMP_DESCR=IDAC functionality using the CSD hardware, constructed to work on top of the CSD driver.

#
# The type of component ...
#   link - means link the source code from the IDE project to the SDK
#   copy - means copy the source code into the IDE project
#
CY_COMP_TYPE=link

#
# Defines if the component can change based on which artifact is being used
#
CY_COMP_PER_ARTIFACT=false

#
# The list of components this component is dependent on. Path is relative the
# the SDK's libraries folder.
#
CY_COMP_DEPS=

#
# Used by the build recipe for an ELF file to add this component
# to the list of components that must be included
#
$(CYARTIFACT)_OBJ_COMP_TAG_LIST += $(MIDDLEWARE_CSDIDAC_NAME)

#
# Defines the series of needed make variables that include this component in the
# build process.  Also defines the describe target if we are describing a component
#
$(MIDDLEWARE_CSDIDAC_NAME)_INCLUDES=\
	-I$(CY_PSOC_LIB_COMP_MIDDLEWARE)/csdidac

$(eval $(call \
	CY_DECLARE_SWCOMP_OBJECT,$(MIDDLEWARE_CSDIDAC_NAME),\
	$(lastword $(MAKEFILE_LIST)),\
	cy_csdidac.c\
	cy_csdidac.h))

