/*******************************************************************************
 * Copyright IBM Corp. and others 1991
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

/* windows.h defined uintptr_t.  Ignore its definition */
#define UDATA UDATA_win32_
#include <windows.h>
#undef UDATA	/* this is safe because our UDATA is a typedef, not a macro */
#include "omrportpriv.h"
#include "omrportptb.h"


/**
 * @internal
 * @brief  Per Thread Buffer Support
 *
 * Free all memory associated with a per thread buffer, including any memory it may
 * have allocated.
 *
 * @param[in] portLibrary The port library.
 * @param[in] ptBuffers pointer to the PortlibPTBuffers struct that contains the buffers
 */
void
omrport_free_ptBuffer(struct OMRPortLibrary *portLibrary, PortlibPTBuffers_t ptBuffer)
{
	if (NULL != ptBuffer) {
		if (NULL != ptBuffer->errorMessageBuffer) {
			portLibrary->mem_free_memory(portLibrary, ptBuffer->errorMessageBuffer);
			ptBuffer->errorMessageBufferSize = 0;
		}
		if (NULL != ptBuffer->reportedMessageBuffer) {
			portLibrary->mem_free_memory(portLibrary, ptBuffer->reportedMessageBuffer);
			ptBuffer->reportedMessageBufferSize = 0;
		}

		portLibrary->mem_free_memory(portLibrary, ptBuffer);
	}
}

/* These functions don't belong here, but they don't belong anywhere else either. */
/**
 * @internal
 * Converts the UTF8 encoded string to Unicode. Use the provided buffer if possible
 * or allocate memory as necessary. Return the new string.
 *
 * @param[in] portLibrary The port library
 * @param[in] string The UTF8 encoded string
 * @param[in] unicodeBuffer The unicode buffer to attempt to use
 * @param[in] unicodeBufferSize The number of bytes available in the unicode buffer
 *
 * @return	the pointer to the Unicode string, or NULL on failure.
 *
 * If the returned buffer is not the same as @ref unicodeBuffer, the returned buffer needs to be freed
 * 	using @ref omrmem_free_memory
 */
wchar_t *
port_convertFromUTF8(OMRPortLibrary *portLibrary, const char *string, wchar_t *unicodeBuffer, uintptr_t unicodeBufferSize)
{
	wchar_t *unicodeString;
	uintptr_t length;

	if (NULL == string) {
		return NULL;
	}
	length = (uintptr_t)strlen(string);
	if (length < unicodeBufferSize) {
		unicodeString = unicodeBuffer;
	} else {
		unicodeString = portLibrary->mem_allocate_memory(portLibrary, (length + 1) * 2, OMR_GET_CALLSITE(), OMRMEM_CATEGORY_PORT_LIBRARY);
		if (NULL == unicodeString) {
			return NULL;
		}
	}
	if (0 == MultiByteToWideChar(OS_ENCODING_CODE_PAGE, OS_ENCODING_MB_FLAGS, string, -1, unicodeString, (int)length + 1)) {
		portLibrary->error_set_last_error(portLibrary, GetLastError(), OMRPORT_ERROR_OPFAILED);
		if (unicodeString != unicodeBuffer) {
			portLibrary->mem_free_memory(portLibrary, unicodeString);
		}
		return NULL;
	}

	return unicodeString;
}
/**
 * @internal
 * Converts the Unicode string to UTF8 encoded data in the provided buffer.
 *
 * @param[in] portLibrary The port library
 * @param[in] unicodeString The unicode buffer to convert
 * @param[in] utf8Buffer The buffer to store the UTF8 encoded bytes into
 * @param[in] size The size of utf8Buffer
 *
 * @return 0 on success, -1 on failure.
 */
int32_t
port_convertToUTF8(OMRPortLibrary *portLibrary, const wchar_t *unicodeString, char *utf8Buffer, uintptr_t size)
{
	if (0 == WideCharToMultiByte(OS_ENCODING_CODE_PAGE, OS_ENCODING_WC_FLAGS, unicodeString, -1, utf8Buffer, (int)size, NULL, NULL)) {
		portLibrary->error_set_last_error(portLibrary, GetLastError(), OMRPORT_ERROR_OPFAILED); /* continue */
		return -1;
	}
	return 0;
}

