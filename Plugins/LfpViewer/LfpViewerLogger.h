#pragma once

#define PLUGIN_NAME "LFP Viewer"

#define LOGA(...) \
    OELogger::instance().LOGFile("[" PLUGIN_NAME "][action] ", __VA_ARGS__)

#define LOGB(...) \
    OELogger::instance().LOGFile("[" PLUGIN_NAME "][buffer] ", __VA_ARGS__)

#define LOGC(...) \
    OELogger::instance().LOGConsole("[" PLUGIN_NAME "] ", __VA_ARGS__)

#ifdef DEBUG
    #define LOGD(...) \
        OELogger::instance().LOGConsole("[" PLUGIN_NAME "][debug] ", __VA_ARGS__)
#else
    #define LOGD(...) \
        OELogger::instance().LOGFile("[" PLUGIN_NAME "][debug] ", __VA_ARGS__)
#endif

#define LOGDD(...) \
    OELogger::instance().LOGFile("[" PLUGIN_NAME "][ddebug] ", __VA_ARGS__)

#define LOGE(...) \
    OELogger::instance().LOGError("[" PLUGIN_NAME "] ***ERROR*** ", __VA_ARGS__)