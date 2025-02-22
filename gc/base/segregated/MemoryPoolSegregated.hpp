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

#if !defined(MEMORYPOOLSEGREGATED_HPP_)
#define MEMORYPOOLSEGREGATED_HPP_

#include "omrcfg.h"
#include "modronopt.h"

#include "AllocationContextSegregated.hpp"
#include "Debug.hpp"
#include "EnvironmentBase.hpp"
#include "GlobalAllocationManagerSegregated.hpp"
#include "Heap.hpp"
#include "MemoryPool.hpp"
#include "SegregatedAllocationTracker.hpp"

#if defined(OMR_GC_SEGREGATED_HEAP)

class MM_MemoryPoolSegregated : public MM_MemoryPool
{
	/*
	 * Data members
	 */
private:	
	MM_RegionPoolSegregated *_regionPool;
protected:
public:
	MM_GlobalAllocationManagerSegregated *_globalAllocationManager;
	MM_GCExtensionsBase* _extensions;
	volatile uintptr_t _bytesInUse;
	
	/*
	 * Function members
	 */
public:
	MM_RegionPoolSegregated *getRegionPool() const { return _regionPool; }
	
	void report(MM_EnvironmentBase *env);

	uintptr_t getBytesInUse()
	{
		uintptr_t pessimisticBytesInUse = _bytesInUse + OMR_MIN(_extensions->allocationTrackerMaxTotalError, (_extensions->allocationTrackerMaxThreshold * _extensions->currentEnvironmentCount));
		uintptr_t activeMemorySize = _extensions->getHeap()->getActiveMemorySize();
		
		/* It's possible that our pessimistic bytes in use approximation goes over the heap size since we assume all threads
		 * are on the verge of flushing.
		 */
		if (pessimisticBytesInUse > activeMemorySize) {
			pessimisticBytesInUse = activeMemorySize;
		}
		return pessimisticBytesInUse;
	}
	
	static MM_MemoryPoolSegregated *newInstance(MM_EnvironmentBase *env, MM_RegionPoolSegregated *regionPool, uintptr_t minimumFreeEntrySize, MM_GlobalAllocationManagerSegregated *gam);

	MM_SegregatedAllocationTracker* createAllocationTracker(MM_EnvironmentBase* env);

	uintptr_t getApproximateActiveFreeMemorySize();
	virtual uintptr_t getApproximateFreeMemorySize();
	virtual uintptr_t getActualFreeMemorySize();
	virtual uintptr_t getActualFreeEntryCount();
	uintptr_t debugGetActualFreeMemorySize();
	uintptr_t debugCountFreeBytes();
	virtual void setFreeMemorySize(uintptr_t freeMemorySize);
	virtual void setFreeEntryCount(uintptr_t entryCount);
	
	virtual void *allocateObject(MM_EnvironmentBase *env, MM_AllocateDescription *allocDesc);
	virtual void *allocateArrayletLeaf(MM_EnvironmentBase *env, MM_AllocateDescription *allocDesc);
#if defined(OMR_GC_THREAD_LOCAL_HEAP)
	using MM_MemoryPool::allocateTLH;
	virtual void *allocateTLH(MM_EnvironmentBase *env, uintptr_t maxSizeInBytesRequired, void * &addrBase, void * &addrTop);
#endif /* OMR_GC_THREAD_LOCAL_HEAP */

	virtual void reset(Cause cause = any);

	virtual void addRange(MM_EnvironmentBase *env, void *prevFreeEntry, uintptr_t prevFreeEntrySize, 
	void *currentFreeEntry, uintptr_t currentFreeEntrySize);
	virtual void insertRange(MM_EnvironmentBase *env, void *prevFreeListEntry, uintptr_t prevFreeListEntrySize,
	void *expandRangeBase, void *expandRangeTop, void *nextFreeListEntry, uintptr_t nextFreeListEntrySize);
	virtual void buildRange(MM_EnvironmentBase *env, void *expandRangeBase, void *expandRangeTop);
	virtual void expandWithRange(MM_EnvironmentBase *env, uintptr_t expandSize, void *lowAddr, void *highAddr, bool canCoalesce);
	virtual void *contractWithRange(MM_EnvironmentBase *env, uintptr_t expandSize, void *lowAddr, void *highAddr);

	virtual bool abandonHeapChunk(void *addrBase, void *addrTop);

	MMINLINE uintptr_t verbose(MM_EnvironmentBase *env) { return _extensions->verbose; }
	MMINLINE uintptr_t debug(MM_EnvironmentBase *env) { return _extensions->debug; }

	void moveInUseToSweep(MM_EnvironmentBase *env);
	void flushCachedFullRegions(MM_EnvironmentBase *env);
	
	uintptr_t getUsedAndLiveObjectsSize(MM_EnvironmentBase *env, bool unmark_objects, bool unmark_live_alloc, 
	uintptr_t *allocGarbage, uintptr_t *live, uintptr_t *allocLive);

protected:
	bool initialize(MM_EnvironmentBase *env);
	virtual void tearDown(MM_EnvironmentBase *env);
	
	MM_MemoryPoolSegregated(MM_EnvironmentBase *env, MM_RegionPoolSegregated *regionPool, uintptr_t minimumFreeEntrySize, MM_GlobalAllocationManagerSegregated *globalAllocationManager) 
		: MM_MemoryPool(env, minimumFreeEntrySize)
		, _regionPool(regionPool)
		, _globalAllocationManager(globalAllocationManager)
		, _extensions(NULL)
		, _bytesInUse(0)
	{
		_typeId = __FUNCTION__;
	}
	
private:
	uintptr_t *allocateContiguous(MM_EnvironmentBase *env, MM_AllocateDescription *allocDesc, MM_AllocationContextSegregated *ac);
	void *allocateChunkedArray(MM_EnvironmentBase *env, MM_AllocateDescription *allocDesc, MM_AllocationContextSegregated *ac);

};

#endif /* OMR_GC_SEGREGATED_HEAP */

#endif /* MEMORYPOOLSEGREGATED_HPP_ */
