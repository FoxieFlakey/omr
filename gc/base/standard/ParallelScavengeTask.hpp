/*******************************************************************************
 * Copyright IBM Corp. and others 2015
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

/**
 * @file
 * @ingroup GC_Modron_Standard
 */

#if !defined(GC_BASE_STANDARD_PARALLELSCAVENGETASK_HPP_)
#define GC_BASE_STANDARD_PARALLELSCAVENGETASK_HPP_

#include "omrcfg.h"

#if defined(OMR_GC_MODRON_SCAVENGER)

#include "modronopt.h"
#include "omrmodroncore.h"

#include "CycleState.hpp"
#include "ParallelTask.hpp"

class MM_EnvironmentBase;
class MM_ParallelDispatcher;

/**
 * @todo Provide class documentation
 * @ingroup GC_Modron_Standard
 */
class MM_ParallelScavengeTask : public MM_ParallelTask
{
protected:
	MM_Scavenger *_collector;
	MM_CycleState *_cycleState;  /**< Collection cycle state active for the task */
	uintptr_t _recommendedThreads;  /**< Collector recommended threads for the task */

public:
	virtual UDATA getVMStateID() { return OMRVMSTATE_GC_SCAVENGE; };

	virtual void run(MM_EnvironmentBase *env);
	virtual void setup(MM_EnvironmentBase *env);
	virtual void cleanup(MM_EnvironmentBase *env);
	virtual void mainSetup(MM_EnvironmentBase *env);

#if defined(J9MODRON_TGC_PARALLEL_STATISTICS)
	/**
	 * Override to collect stall time statistics.
	 * @see MM_ParallelTask::synchronizeGCThreads
	 */
	virtual void synchronizeGCThreads(MM_EnvironmentBase *env, const char *id);

	/**
	 * Override to collect stall time statistics.
	 * @see MM_ParallelTask::synchronizeGCThreadsAndReleaseMain
	 */
	virtual bool synchronizeGCThreadsAndReleaseMain(MM_EnvironmentBase *env, const char *id);

	/**
	 * Override to collect stall time statistics.
	 * @see MM_ParallelTask::synchronizeGCThreadsAndReleaseSingleThread
	 */
	virtual bool synchronizeGCThreadsAndReleaseSingleThread(MM_EnvironmentBase *env, const char *id);

	virtual void addToNotifyStallTime(MM_EnvironmentBase *env, uint64_t startTime, uint64_t endTime);
#endif /* J9MODRON_TGC_PARALLEL_STATISTICS */

	virtual uintptr_t getRecommendedWorkingThreads() { return _recommendedThreads; };

	/**
	 * Create a ParallelScavengeTask object.
	 */
	MM_ParallelScavengeTask(MM_EnvironmentBase *env, MM_ParallelDispatcher *dispatcher, MM_Scavenger *collector,MM_CycleState *cycleState, uintptr_t recommendedThreads) :
		MM_ParallelTask(env, dispatcher)
		,_collector(collector)
		,_cycleState(cycleState)
		,_recommendedThreads(recommendedThreads)
	{
		_typeId = __FUNCTION__;
	};
};

#endif /* defined(OMR_GC_MODRON_SCAVENGER) */
#endif /* GC_BASE_STANDARD_PARALLELSCAVENGETASK_HPP_ */
