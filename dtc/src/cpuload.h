/*
  DMX Total Control
  cpuload.h

  Copyright (c) DMX Total Control contributors

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0.txt

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
*/

#ifndef CPULOAD_H
#define CPULOAD_H

#include <QtGlobal>

/** The processor time this process has used so far, all its threads
 *  together, in milliseconds. -1 where it can't be known.
 *  (Apart from the rest: the system headers it needs define macros, see
 *  near and far on Windows) */
qint64 processCpuTime();

#endif
