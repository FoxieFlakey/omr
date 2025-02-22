###############################################################################
# Copyright IBM Corp. and others 2017
#
# This program and the accompanying materials are made available under
# the terms of the Eclipse Public License 2.0 which accompanies this
# distribution and is available at https://www.eclipse.org/legal/epl-2.0/
# or the Apache License, Version 2.0 which accompanies this distribution and
# is available at https://www.apache.org/licenses/LICENSE-2.0.
#
# This Source Code may also be made available under the following
# Secondary Licenses when the conditions for such availability set
# forth in the Eclipse Public License, v. 2.0 are satisfied: GNU
# General Public License, version 2 with the GNU Classpath
# Exception [1] and GNU General Public License, version 2 with the
# OpenJDK Assembly Exception [2].
#
# [1] https://www.gnu.org/software/classpath/license.html
# [2] https://openjdk.org/legal/assembly-exception.html
#
# SPDX-License-Identifier: EPL-2.0 OR Apache-2.0 OR GPL-2.0 WITH Classpath-exception-2.0 OR LicenseRef-GPL-2.0 WITH Assembly-exception
###############################################################################

omr_add_tracegen(omrti.tdf)
omr_add_tracegen(omrvm.tdf)

add_subdirectory(startup)

omr_assert(
	TEST OMR_CORE_GLUE_TARGET
	MESSAGE "OMR_CORE_GLUE_TARGET must be set. Try using the example with OMR_EXAMPLE=ON"
)

omr_assert(
	TEST OMR_RAS_GLUE_TARGET
	MESSAGE "OMR_RAS_GLUE_TARGET must be set. Try using the example with OMR_EXAMPLE=ON"
)

omr_add_library(omrcore STATIC
	OMR_Agent.cpp
	OMR_MethodDictionary.cpp
	OMR_Profiler.cpp
	OMR_Runtime.cpp
	OMR_TI.cpp
	OMR_TIMemorySize.cpp
	OMR_VM.cpp
	OMR_VMThread.cpp
	${CMAKE_CURRENT_BINARY_DIR}/ut_omrti.c
	${CMAKE_CURRENT_BINARY_DIR}/ut_omrvm.c
)

target_include_directories(omrcore
	PUBLIC
		.
	PRIVATE
		${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(omrcore
	PUBLIC
		omr_base
	PRIVATE
		${OMR_CORE_GLUE_TARGET}
		${OMR_RAS_GLUE_TARGET}
)

if(OMR_GC)
	add_dependencies(omrcore
		omrgc_tracegen
		omrgc_hookgen
	)

	target_include_directories(omrcore
		PRIVATE
			$<TARGET_PROPERTY:${OMR_GC_LIB},INTERFACE_INCLUDE_DIRECTORIES>
			$<TARGET_PROPERTY:${OMR_GC_GLUE_TARGET},INTERFACE_INCLUDE_DIRECTORIES>
	)
endif(OMR_GC)

target_enable_ddr(omrcore)
ddr_set_add_targets(omrddr omrcore)
