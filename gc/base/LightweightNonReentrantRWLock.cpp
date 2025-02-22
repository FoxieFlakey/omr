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

#include <assert.h>

#include "AtomicOperations.hpp"
#include "LightweightNonReentrantRWLock.hpp"

intptr_t
MM_LightweightNonReentrantRWLock::initialize(uintptr_t spinCount)
{
	intptr_t ret = LWRW_OK;
	_spinCount = spinCount;

#if defined(J9MODRON_USE_CUSTOM_READERWRITERLOCK)
	_status = LWRW_READER_MODE;
	MM_AtomicOperations::writeBarrier();
#else /* defined(J9MODRON_USE_CUSTOM_READERWRITERLOCK) */
	if (J9THREAD_RWMUTEX_OK != omrthread_rwmutex_init( &_rwmutex, 0, "MM_LightweightNonReentrantRWLock::_rwmutex" )) {
		ret = LWRW_FAILED_INIT;
	}
#endif /* defined(J9MODRON_USE_CUSTOM_READERWRITERLOCK) */
	return ret;
}

intptr_t
MM_LightweightNonReentrantRWLock::tearDown()
{
	intptr_t ret = LWRW_OK;
#if !defined(J9MODRON_USE_CUSTOM_READERWRITERLOCK)
	ret = omrthread_rwmutex_destroy(_rwmutex);
#endif /* !defined(J9MODRON_USE_CUSTOM_READERWRITERLOCK) */
	return ret;
}

intptr_t
MM_LightweightNonReentrantRWLock::enterRead()
{
	intptr_t ret = LWRW_OK;
#if defined(J9MODRON_USE_CUSTOM_READERWRITERLOCK)
	uint32_t oldValue;
	uint32_t newValue;

	for (;;) {
		/* check waiting writers */
		oldValue = (_status & LWRW_MASK_WAITINGWRITERS);
		/* check reader mode */
		oldValue |= LWRW_READER_MODE;
		/* increment reader count */
		newValue = oldValue + LWRW_INCREMENTAL_BASE_READERS;
		if (LWRW_MASK_WAITINGWRITERS <= (newValue&LWRW_MASK_WAITINGWRITERS)) {
			// reach max reader count
			assert(false);
		}

		uint32_t retValue = MM_AtomicOperations::lockCompareExchangeU32((volatile uint32_t*)&_status, oldValue, newValue);
		if (oldValue == retValue) {
			break;
		} else if ((LWRW_READER_MODE == (retValue & LWRW_READER_MODE)) && 0 == (retValue & LWRW_MASK_READERS)) {
			/* not writer mode and no waiting writers */
			continue;
		}

		MM_AtomicOperations::yieldCPU();
		/* begin tight loop */
		for (uintptr_t spinCount = _spinCount; spinCount > 0; spinCount--)	{
			MM_AtomicOperations::nop();
		} /* end tight loop */
	}

	/* MM_AtomicOperations::readBarrier() might be good enough */
	MM_AtomicOperations::readWriteBarrier();
#else /* defined(J9MODRON_USE_CUSTOM_READERWRITERLOCK) */
	ret = omrthread_rwmutex_enter_read(_rwmutex);
#endif /* defined(J9MODRON_USE_CUSTOM_READERWRITERLOCK) */
	return ret;
}

intptr_t
MM_LightweightNonReentrantRWLock::exitRead()
{
	intptr_t ret = LWRW_OK;
#if defined(J9MODRON_USE_CUSTOM_READERWRITERLOCK)
	uint32_t oldValue;
	uint32_t newValue;
	for (;;) {
		oldValue = _status;
		/* decrement reader count */
		newValue = oldValue - LWRW_INCREMENTAL_BASE_READERS;
		uint32_t retValue = MM_AtomicOperations::lockCompareExchangeU32((volatile uint32_t*)&_status, oldValue, newValue);
		if (oldValue == retValue) {
			break;
		}
	}
#else /* defined(J9MODRON_USE_CUSTOM_READERWRITERLOCK) */
	ret = omrthread_rwmutex_exit_read(_rwmutex);
#endif /* defined(J9MODRON_USE_CUSTOM_READERWRITERLOCK) */

	return ret;
}

intptr_t
MM_LightweightNonReentrantRWLock::enterWrite()
{
	intptr_t ret = LWRW_OK;
#if defined(J9MODRON_USE_CUSTOM_READERWRITERLOCK)
	uint32_t oldValue;
	uint32_t newValue;
	uint32_t retValue;

	oldValue = LWRW_READER_MODE;
	newValue = LWRW_WRITER_MODE;
	retValue = MM_AtomicOperations::lockCompareExchangeU32((volatile uint32_t*)&_status, oldValue, newValue);
	if (oldValue != retValue) {
		/* the lock is hold by readers or another writer */
		/* increment writer waiting count */
		do {
			oldValue = retValue;
			newValue = oldValue + LWRW_INCREMENTAL_BASE_WAITINGWRITERS;
			retValue = MM_AtomicOperations::lockCompareExchangeU32((volatile uint32_t*)&_status, oldValue, newValue);
		} while (oldValue != retValue);

		retValue = newValue;
		for(;;) {
			/* check no reader */
			oldValue = (retValue & LWRW_MASK_READERS);
			/* check reader mode */
			oldValue |= LWRW_READER_MODE;
			/* decrement writer waiting count */
			newValue = oldValue - LWRW_INCREMENTAL_BASE_WAITINGWRITERS;
			/* set writer mode */
			newValue &= LWRW_MASK_MODE;
			retValue = MM_AtomicOperations::lockCompareExchangeU32((volatile uint32_t*)&_status, oldValue, newValue);
			if (oldValue == retValue) {
				break;
			}

			MM_AtomicOperations::yieldCPU();
			/* begin tight loop */
			for (uintptr_t spinCount = _spinCount; spinCount > 0; spinCount--)	{
				MM_AtomicOperations::nop();
			} /* end tight loop */

		}
	}

	MM_AtomicOperations::readWriteBarrier();
#else /* defined(J9MODRON_USE_CUSTOM_READERWRITERLOCK) */
	ret = omrthread_rwmutex_enter_write(_rwmutex);
#endif /* defined(J9MODRON_USE_CUSTOM_READERWRITERLOCK) */
	return ret;
}

intptr_t
MM_LightweightNonReentrantRWLock::exitWrite()
{
	intptr_t ret = LWRW_OK;
#if defined(J9MODRON_USE_CUSTOM_READERWRITERLOCK)
	MM_AtomicOperations::writeBarrier();

	uint32_t oldValue;
	uint32_t retValue;

	do {
		oldValue = _status;
		/* back to read mode */
		uint32_t newValue = oldValue | LWRW_READER_MODE;
		retValue = MM_AtomicOperations::lockCompareExchangeU32((volatile uint32_t*)&_status, oldValue, newValue);
	} while (oldValue != retValue);
#else /* defined(J9MODRON_USE_CUSTOM_READERWRITERLOCK) */
	ret = omrthread_rwmutex_exit_write(_rwmutex);
#endif /* defined(J9MODRON_USE_CUSTOM_READERWRITERLOCK) */

	return ret;
}

