/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2024 Open Ephys

    ------------------------------------------------------------------

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

/*
This header contains all the headers needed by processor nodes.
Should be included in the source files which declare a processor class.
*/

#include "../../JuceLibraryCode/JuceHeader.h"
#include "../../Source/Processors/Events/Event.h"
#include "../../Source/Processors/Events/Spike.h"
#include "../../Source/Processors/GenericProcessor/GenericProcessor.h"
#include "../../Source/TestableExport.h"
#include "../../Source/Utils/BroadcastParser.h"
#include "DspLib.h"

#ifndef PROCESSOR_NAME
    #define PROCESSOR_NAME "Unknown"
#endif

#define LOGA(...) \
    getOELogger().LOGFile("[" PROCESSOR_NAME "][action] ", __VA_ARGS__)

#define LOGB(...) \
    getOELogger().LOGFile("[" PROCESSOR_NAME "][buffer] ", __VA_ARGS__)

#define LOGC(...) \
    getOELogger().LOGConsole("[" PROCESSOR_NAME "] ", __VA_ARGS__)

#ifdef DEBUG
    #define LOGD(...) \
        getOELogger().LOGConsole("[" PROCESSOR_NAME "][debug] ", __VA_ARGS__)
#else
    #define LOGD(...) \
        getOELogger().LOGFile("[" PROCESSOR_NAME "][debug] ", __VA_ARGS__)
#endif

#define LOGDD(...) \
    getOELogger().LOGFile("[" PROCESSOR_NAME "][ddebug] ", __VA_ARGS__)

#define LOGE(...) \
    getOELogger().LOGError("[" PROCESSOR_NAME "] ***ERROR*** ", __VA_ARGS__)