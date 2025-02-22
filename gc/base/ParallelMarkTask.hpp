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


/**
 * @file
 * @ingroup GC_Modron_Standard
 */

#if !defined(PARALLELMARKTASK_HPP_)
#define PARALLELMARKTASK_HPP_

#include "omrcfg.h"

#include "CycleState.hpp"
#include "ParallelTask.hpp"

class MM_EnvironmentBase;
class MM_MarkingScheme;
class MM_ParallelDispatcher;

/**
 * @todo Provide class documentation
 * @ingroup GC_Modron_Standard
 */
class MM_ParallelMarkTask : public MM_ParallelTask
{
public:
	enum MarkAction {
		MARK_ALL = 1,
		MARK_ROOTS,
	};

private:
	MM_MarkingScheme *_markingScheme;
	const bool _initMarkMap;
	MM_CycleState *_cycleState;  /**< Collection cycle state active for the task */
	const MarkAction _action;
	
public:
	virtual uintptr_t getVMStateID();
	
	virtual void run(MM_EnvironmentBase *env);
	virtual void setup(MM_EnvironmentBase *env);
	virtual void cleanup(MM_EnvironmentBase *env);
	
#if defined(J9MODRON_TGC_PARALLEL_STATISTICS)
	virtual void synchronizeGCThreads(MM_EnvironmentBase *env, const char *id);
	virtual bool synchronizeGCThreadsAndReleaseMain(MM_EnvironmentBase *env, const char *id);
	virtual bool synchronizeGCThreadsAndReleaseSingleThread(MM_EnvironmentBase *env, const char *id);
#endif /* J9MODRON_TGC_PARALLEL_STATISTICS */

	/**
	 * Create a ParallelMarkTask object.
	 */
	MM_ParallelMarkTask(MM_EnvironmentBase *env,
			MM_ParallelDispatcher *dispatcher, 
			MM_MarkingScheme *markingScheme, 
			bool initMarkMap,
			MM_CycleState *cycleState,
			MarkAction action = MARK_ALL) :
		MM_ParallelTask(env, dispatcher)
		,_markingScheme(markingScheme)
		,_initMarkMap(initMarkMap)
		,_cycleState(cycleState)
		,_action(action)
	{
		_typeId = __FUNCTION__;
	};
};

#endif /* PARALLELMARKTASK_HPP_ */
