/*******************************************************************************
 * Copyright IBM Corp. and others 2018
 *
 * This program and the accompanying materials are made available under
 * the terms of the Eclipse Public License 2.0 which accompanies this
 * distribution and is available at https://www.eclipse.org/legal/epl-2.0/
 * or the Apache License, Version 2.0 which accompanies this distribution and
 * is available at https://www.apache.org/licenses/LICENSE-2.0.
 *
 * This Source Code may also be made available under the following
 * Secondary Licenses when the conditions for such availability set
 * forth in the Eclipse Public License, v. 2.0 are satisfied: GNU
 * General Public License, version 2 with the GNU Classpath
 * Exception [1] and GNU General Public License, version 2 with the
 * OpenJDK Assembly Exception [2].
 *
 * [1] https://www.gnu.org/software/classpath/license.html
 * [2] https://openjdk.org/legal/assembly-exception.html
 *
 * SPDX-License-Identifier: EPL-2.0 OR Apache-2.0 OR GPL-2.0 WITH Classpath-exception-2.0 OR LicenseRef-GPL-2.0 WITH Assembly-exception
 *******************************************************************************/

#define _GNU_SOURCE

#include <sys/ucontext.h>
#include <unistd.h>

#include "omrport.h"
#include "omrsignal_context.h"

#define NGPRS 32
#define NFPRS 32

/* AArch64 Signal Context Reserved Space Size */
#define RESERVED_SPACE_SZ 4096

void
fillInUnixSignalInfo(struct OMRPortLibrary *portLibrary, void *contextInfo, struct OMRUnixSignalInfo *signalInfo)
{
	signalInfo->platformSignalInfo.context = (ucontext_t *)contextInfo;
	/* module info is filled on demand */
}

uint32_t
infoForSignal(struct OMRPortLibrary *portLibrary, struct OMRUnixSignalInfo *info, int32_t index, const char **name, void **value)
{
	*name = "";

	switch (index) {

	case OMRPORT_SIG_SIGNAL_TYPE:
	case 0:
		*name = "J9Generic_Signal_Number";
		*value = &info->portLibrarySignalType;
		return OMRPORT_SIG_VALUE_32;

	case OMRPORT_SIG_SIGNAL_PLATFORM_SIGNAL_TYPE:
	case 1:
		*name = "Signal_Number";
		if( NULL != info->sigInfo) {
			*value = &info->sigInfo->si_signo;
			return OMRPORT_SIG_VALUE_32;
		}
		break;

	case OMRPORT_SIG_SIGNAL_ERROR_VALUE:
	case 2:
		*name = "Error_Value";
		*value = &info->sigInfo->si_errno;
		return OMRPORT_SIG_VALUE_32;

	case OMRPORT_SIG_SIGNAL_CODE:
	case 3:
		*name = "Signal_Code";
		if( NULL != info->sigInfo) {
			*value = &info->sigInfo->si_code;
			return OMRPORT_SIG_VALUE_32;
		}
		break;

	case OMRPORT_SIG_SIGNAL_HANDLER:
	case 4:
		*name = "Handler1";
		*value = &info->handlerAddress;
		return OMRPORT_SIG_VALUE_ADDRESS;

	case 5:
		*name = "Handler2";
		*value = &info->handlerAddress2;
		return OMRPORT_SIG_VALUE_ADDRESS;
	
	case OMRPORT_SIG_SIGNAL_INACCESSIBLE_ADDRESS:
	case 6:
		/* si_code > 0 indicates that the signal was generated by the kernel */
		if ( NULL != info->sigInfo && 0 < info->sigInfo->si_code ) {
			if ((info->sigInfo->si_signo == SIGBUS) || (info->sigInfo->si_signo == SIGSEGV)) {
				*name = "InaccessibleAddress";
				*value = &info->sigInfo->si_addr;
				return OMRPORT_SIG_VALUE_ADDRESS;
			}
		}
		break;

	// case OMRPORT_SIG_SIGNAL_ADDRESS:
	// case 7:
	// 	*name = "fault_address";
	// 	*value = &((struct sigcontext *)&info->platformSignalInfo.context->uc_mcontext)->fault_address;
	// 	return OMRPORT_SIG_VALUE_ADDRESS;


	default:
		break;
	}
	
	return OMRPORT_SIG_VALUE_UNDEFINED;
}


/**
 * Walk the 4k reserve space to get the floating point registers since the layout differ 
 * per-chip
 * @param	magic_to_find	input	magic number #define in the linux kernel (sigcontext.h)\
 * that defines which struct the reserved area is at a given space
 * @param	reserved_space	input	array of bytes that holds all the struct
 * @see linux/arch/arm64/include/uapi/asm/sigcontext.h for the defines and information to why we need
 * to walk the space

 */
void*
walkReserveSpace(uint32_t magic_to_find, uint8_t *reserved_space)
{
	if ( NULL == reserved_space ) {
		return NULL;
	}
	
	int i = 0;
	while ( RESERVED_SPACE_SZ > i ) {
		struct _aarch64_ctx *header = (struct _aarch64_ctx *)&reserved_space[i];
		if ( NULL == header || 0 == header->size ) {
			return NULL;
		} else if ( magic_to_find == header->magic ) {
			return (void*)&reserved_space[i];
		}
		i += header->size;
	}
	return NULL;
}

uint32_t
infoForFPR(struct OMRPortLibrary *portLibrary, struct OMRUnixSignalInfo *info, int32_t index, const char **name, void **value)
{
	*name = "";
	
	/**
	 * per documentation these are 128 bits registers, can be used as 64, 32 and 16 bit floats
	 * (16 bit floats CANNOT be used for arithmetic)
	 */
	 
	if (( 0 <= index ) && ( NFPRS > index )) {
		struct sigcontext *const context = (struct sigcontext *)&info->platformSignalInfo.context->uc_mcontext;
		struct fpsimd_context *const fp_regs = walkReserveSpace(FPSIMD_MAGIC, (uint8_t *)context->__reserved);
		
		const char *n_fpr[NFPRS] = {
			"V0",
			"V1",
			"V2",
			"V3",
			"V4",
			"V5",
			"V6",
			"V7",
			"V8",
			"V9",
			"V10",
			"V11",
			"V12",
			"V13",
			"V14",
			"V15",
			"V16",
			"V17",
			"V18",
			"V19",
			"V20",
			"V21",
			"V22",
			"V23",
			"V24",
			"V25",
			"V26",
			"V27",
			"V28",
			"V29",
			"V30",
			"V31"
		};
	
		*name = n_fpr[index];
		if( NULL != context ) {
			*value = &fp_regs->vregs[index];
			return OMRPORT_SIG_VALUE_FLOAT_64;
		}
	}
	
	return OMRPORT_SIG_VALUE_UNDEFINED;
}

uint32_t
infoForGPR(struct OMRPortLibrary *portLibrary, struct OMRUnixSignalInfo *info, int32_t index, const char **name, void **value)
{
	*name = "";
	
	if (( 0 <= index ) && ( NGPRS > index )) {
		struct sigcontext *const context = (struct sigcontext *)&info->platformSignalInfo.context->uc_mcontext;
		const char *n_gpr[NGPRS] = {
			"R0",
			"R1",
			"R2",
			"R3",
			"R4",
			"R5",
			"R6",
			"R7",
			"R8",
			"R9",
			"R10",
			"R11",
			"R12",
			"R13",
			"R14",
			"R15",
			"R16",
			"R17",
			"R18",
			"R19",
			"R20",
			"R21",
			"R22",
			"R23",
			"R24",
			"R25",
			"R26",
			"R27",
			"R28",
			"R29",
			"R30",
			"R31"
		};
	
		*name = n_gpr[index];
		if( NULL != context ) {
			*value = &context->regs[index];
			return OMRPORT_SIG_VALUE_ADDRESS;
		}
	}
	
	return OMRPORT_SIG_VALUE_UNDEFINED;

}

uint32_t
infoForControl(struct OMRPortLibrary *portLibrary, struct OMRUnixSignalInfo *info, int32_t index, const char **name, void **value)
{
	struct sigcontext *const context = (struct sigcontext *)&info->platformSignalInfo.context->uc_mcontext;
	*name = "";
	
	switch ( index ) {
		
	case OMRPORT_SIG_CONTROL_PC:
	case 0:
		*name = "PC";
		if( NULL != context ) {
			*value = &context->pc;
			return OMRPORT_SIG_VALUE_ADDRESS;
		}
		break;
		
	case OMRPORT_SIG_CONTROL_SP:
	case 1:
		*name = "SP";
		if( NULL != context ) {
			*value = &context->sp;
			return OMRPORT_SIG_VALUE_ADDRESS;
		}
		break;
		
	case 2:
		*name = "PSTATE";
		if( NULL != context ) {
			*value = &context->pstate;
			return OMRPORT_SIG_VALUE_ADDRESS;
		}
		break;
		
	default:
		break;
	}
	
	return OMRPORT_SIG_VALUE_UNDEFINED;
}

uint32_t
infoForModule(struct OMRPortLibrary *portLibrary, struct OMRUnixSignalInfo *info, int32_t index, const char **name, void **value)
{
	struct sigcontext *context = (struct sigcontext *)&info->platformSignalInfo.context->uc_mcontext;
	*name = "";
	int dl_result = 0;
	Dl_info *dl_info = NULL;
	
	if( NULL != context ) {
		dl_info = &(info->platformSignalInfo.dl_info);
		dl_result = dladdr((void *)context->pc, dl_info);
	}
	
	switch (index) {
		
	case OMRPORT_SIG_MODULE_NAME:
	case 0:
		*name = "Module";
		if ( 0 != dl_result ) {
			*value = (void *)(dl_info->dli_fname);
			return OMRPORT_SIG_VALUE_STRING;
		}
		break;
		
	case 1:
		*name = "Module_base_address";
		if ( 0 != dl_result ) {
			*value = (void *)&(dl_info->dli_fbase);
			return OMRPORT_SIG_VALUE_ADDRESS;
		}
		break;
		
	case 2:
		*name = "Symbol";
		if ( 0 != dl_result ) {
			if (dl_info->dli_sname != NULL) {
				*value = (void *)(dl_info->dli_sname);
				return OMRPORT_SIG_VALUE_STRING;
			}
		}
		break;
		
	case 3:
		*name = "Symbol_address";
		if ( 0 != dl_result ) {
			*value = (void *)&(dl_info->dli_saddr);
			return OMRPORT_SIG_VALUE_ADDRESS;
		}
		break;
		
	default:
		break;
	}
	
	return OMRPORT_SIG_VALUE_UNDEFINED;
}
