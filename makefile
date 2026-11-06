# SPDX-License-Identifier: Apache-2.0 OR GPL-2.0-or-later
# Copyright (c) 2009-2026 John R. Smythe Jr.
#

#
#	Global Definitions
#

include		../rules.mak

#
#	Unit and Common Header Definitions
#

CHDRS =	$(HDR)/mem_io.h

#
#	Unit's Library Definitions
#

LIBRARIES =

# Core mio has no messaging dependency; NOW/NOWSECS handlers still require now.

#
#	Product Definitions
#


#
#	Global Targets
#

include		../targets.mak

#
#	Unit's Targets
#


#
#	Include	Object Dependency Files
#

include	.depend
