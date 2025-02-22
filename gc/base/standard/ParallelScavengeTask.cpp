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

#include "omrcfg.h"

#if defined(OMR_GC_MODRON_SCAVENGER)

#include "ModronAssertions.h"

#include "EnvironmentStandard.hpp"
#include "Scavenger.hpp"

#include "ParallelScavengeTask.hpp"

void
MM_ParallelScavengeTask::run(MM_EnvironmentBase *envBase)
{
	MM_EnvironmentStandard *env = MM_EnvironmentStandard::getEnvironment(envBase);
	_collector->workThreadGarbageCollect(env);
}

void
MM_ParallelScavengeTask::mainSetup(MM_EnvironmentBase *env)
{
	uintptr_t calculatedAliasThreshold = (uintptr_t)(getThreadCount() * env->getExtensions()->aliasInhibitingThresholdPercentage);
	_collector->setAliasThreshold(calculatedAliasThreshold);
}

void
MM_ParallelScavengeTask::setup(MM_EnvironmentBase *env)
{
	if (env->isMainThread()) {
		Assert_MM_true(_cycleState == env->_cycleState);
	} else {
		Assert_MM_true(NULL == env->_cycleState);
		env->_cycleState = _cycleState;
	}
}

void
MM_ParallelScavengeTask::cleanup(MM_EnvironmentBase *env)
{
	if (env->isMainThread()) {
		Assert_MM_true(_cycleState == env->_cycleState);
	} else {
		env->_cycleState = NULL;
	}
}

#if defined(J9MODRON_TGC_PARALLEL_STATISTICS)
void
MM_ParallelScavengeTask::synchronizeGCThreads(MM_EnvironmentBase *envBase, const char *id)
{
	MM_EnvironmentStandard *env = MM_EnvironmentStandard::getEnvironment(envBase);
	OMRPORT_ACCESS_FROM_OMRPORT(envBase->getPortLibrary());
	uint64_t startTime = omrtime_hires_clock();
	MM_ParallelTask::synchronizeGCThreads(env, id);
	uint64_t endTime = omrtime_hires_clock();

	env->_scavengerStats.addToSyncStallTime(startTime, endTime);
}

bool
MM_ParallelScavengeTask::synchronizeGCThreadsAndReleaseMain(MM_EnvironmentBase *envBase, const char *id)
{
	MM_EnvironmentStandard *env = MM_EnvironmentStandard::getEnvironment(envBase);
	OMRPORT_ACCESS_FROM_OMRPORT(envBase->getPortLibrary());
	uint64_t startTime = omrtime_hires_clock();
	bool result = MM_ParallelTask::synchronizeGCThreadsAndReleaseMain(env, id);
	uint64_t endTime = omrtime_hires_clock();

	if (result) {
		/* Released thread will exit and execute critical section, recored the start time */
		_syncCriticalSectionStartTime = endTime;
		_syncCriticalSectionDuration = 0;
	}

	/* _syncCriticalSectionDuration must be set now, this thread's stall time is at least the duration of critical section */
	Assert_MM_true((endTime - startTime) >= _syncCriticalSectionDuration);
	env->_scavengerStats.addToSyncStallTime(startTime, endTime, _syncCriticalSectionDuration);

	return result;
}

bool
MM_ParallelScavengeTask::synchronizeGCThreadsAndReleaseSingleThread(MM_EnvironmentBase *envBase, const char *id)
{
	MM_EnvironmentStandard *env = MM_EnvironmentStandard::getEnvironment(envBase);
	OMRPORT_ACCESS_FROM_OMRPORT(envBase->getPortLibrary());
	uint64_t startTime = omrtime_hires_clock();
	bool result = MM_ParallelTask::synchronizeGCThreadsAndReleaseSingleThread(env, id);
	uint64_t endTime = omrtime_hires_clock();

	if (result) {
		/* Released thread will exit and execute critical section, recored the start time */
		_syncCriticalSectionStartTime = endTime;
		_syncCriticalSectionDuration = 0;
	}

	/* _syncCriticalSectionDuration must be set now, this thread's stall time is at least the duration of critical section */
	Assert_MM_true((endTime - startTime) >= _syncCriticalSectionDuration);
	env->_scavengerStats.addToSyncStallTime(startTime, endTime, _syncCriticalSectionDuration);

	return result;
}

void
MM_ParallelScavengeTask::addToNotifyStallTime(MM_EnvironmentBase *env, uint64_t startTime, uint64_t endTime)
{
	env->_scavengerStats.addToNotifyStallTime(startTime, endTime);
}

#endif /* J9MODRON_TGC_PARALLEL_STATISTICS */

#endif /* defined(OMR_GC_MODRON_SCAVENGER) */
