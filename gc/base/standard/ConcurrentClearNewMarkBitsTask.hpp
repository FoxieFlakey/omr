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

#if !defined(CONCURRENTCLEARNEWMARKBITSTASK_HPP_)
#define CONCURRENTCLEARNEWMARKBITSTASK_HPP_

#include "omrcfg.h"
#include "omrmodroncore.h"

#include "ParallelTask.hpp"

class MM_ConcurrentGC;
class MM_EnvironmentBase;
class MM_ParallelDispatcher;

/**
 * @todo Provide class documentation
 * @ingroup GC_Modron_Standard
 */
class MM_ConcurrentClearNewMarkBitsTask : public MM_ParallelTask
{
private:
	MM_ConcurrentGC *_collector;

public:
	virtual UDATA getVMStateID() { return OMRVMSTATE_GC_CONCURRENT_MARK_CLEAR_NEW_MARKBITS; };
	
	virtual void run(MM_EnvironmentBase *env);

	/**
	 * Create a ConcurrentClearNewMarkBitsTask object
	 */
	MM_ConcurrentClearNewMarkBitsTask(MM_EnvironmentBase *env, MM_ParallelDispatcher *dispatcher, MM_ConcurrentGC *collector) :
		MM_ParallelTask(env, dispatcher),
		_collector(collector)
	{
		_typeId = __FUNCTION__;
	};
};

#endif /* CONCURRENTCLEARNEWMARKBITSTASK_HPP_ */
