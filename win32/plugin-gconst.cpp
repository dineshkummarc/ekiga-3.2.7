#define PPLUGIN_STATIC_LOAD(serviceName, serviceType) \
  class PPlugin_##serviceType##_##serviceName##_Registration; \
  extern PPlugin_##serviceType##_##serviceName##_Registration PPlugin_##serviceType##_##serviceName##_Registration_Instance; \
  PPlugin_##serviceType##_##serviceName##_Registration const *PPlugin_##serviceType##_##serviceName##_Registration_Static_Library_Loader = &PPlugin_##serviceType##_##serviceName##_Registration_Instance;

#include "ptbuildopts.h"

#if P_AUDIO
  PPLUGIN_STATIC_LOAD(WindowsMultimedia, PSoundChannel);
#endif

#if P_DIRECTSHOW
  PPLUGIN_STATIC_LOAD(DirectShow, PVideoInputDevice);
#endif

#if P_VFW_CAPTURE
  PPLUGIN_STATIC_LOAD(VideoForWindows, PVideoInputDevice);
#endif

