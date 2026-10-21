/*
  DMX Total Control
  cpuload.cpp

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

#include "cpuload.h"

#if defined(Q_OS_WIN)
#include <windows.h>
#elif defined(Q_OS_UNIX)
#include <sys/resource.h>
#endif

qint64 processCpuTime()
{
#if defined(Q_OS_WIN)
    FILETIME creation, exit, kernel, user;
    if (GetProcessTimes(GetCurrentProcess(), &creation, &exit, &kernel, &user) == FALSE)
        return -1;
    // (in units of 100 ns)
    const quint64 kernelTime = (quint64(kernel.dwHighDateTime) << 32) | kernel.dwLowDateTime;
    const quint64 userTime = (quint64(user.dwHighDateTime) << 32) | user.dwLowDateTime;
    return qint64((kernelTime + userTime) / 10000);
#elif defined(Q_OS_UNIX)
    struct rusage usage;
    if (getrusage(RUSAGE_SELF, &usage) != 0)
        return -1;
    return qint64(usage.ru_utime.tv_sec + usage.ru_stime.tv_sec) * 1000 +
           qint64(usage.ru_utime.tv_usec + usage.ru_stime.tv_usec) / 1000;
#else
    return -1;
#endif
}
