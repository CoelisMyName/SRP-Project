#ifndef SRP_PROJECT_GLOBAL_H
#define SRP_PROJECT_GLOBAL_H

#include <jni.h>
#include "MyMLLiveModel.h"

extern JavaVM *g_jvm;

extern MyMLLiveModel *g_model;

extern char g_external_path[1024];

#endif
