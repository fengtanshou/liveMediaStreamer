/*
 *  CustomScheduler.hh - This class rewrites the deEventLoop method of the live555 scheduler
 *  Copyright (C) 2014  Fundació i2CAT, Internet i Innovació digital a Catalunya
 *
 *  This file is part of liveMediaStreamer.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  Authors:  David Cassany <david.cassany@i2cat.net>,
 *
 */

#ifndef _CUSTOM_SCHEDULER_HH
#define _CUSTOM_SCHEDULER_HH

#include <BasicUsageEnvironment.hh>

#define GRANULARITY 1000 //in microseconds

class CustomScheduler : public BasicTaskScheduler {
public:
    static CustomScheduler* createNew(unsigned maxSchedulerGranularity = GRANULARITY);
    void doEventLoop(char volatile* watchVariable);
    
private:
    CustomScheduler(unsigned maxSchedulerGranularity) : BasicTaskScheduler(maxSchedulerGranularity){};
};

#endif