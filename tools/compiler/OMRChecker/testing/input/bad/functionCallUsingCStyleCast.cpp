/*******************************************************************************
 * Copyright IBM Corp. and others 2016
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
 * Description: Calls an extensible class member function by using a
 *    c-style down cast of `this`, which is not allowed.
 */

#define OMR_EXTENSIBLE __attribute__((annotate("OMR_Extensible")))

namespace OMR
{

class OMR_EXTENSIBLE ExtClass
   {
   public:
   void functionCalled();   // function to be called
   void callingFunction();  // function that will make call
                            //    with c-style down casting 
   };

} // namespace OMR

namespace TR { class OMR_EXTENSIBLE ExtClass : public OMR::ExtClass {}; }

void OMR::ExtClass::functionCalled() {}

void OMR::ExtClass::callingFunction() { ((TR::ExtClass*)(this))->functionCalled(); }

