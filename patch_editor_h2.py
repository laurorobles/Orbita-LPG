with open("Source/PluginEditor.h", "r") as f:
    h = f.read()

h = h.replace('#include "UIComponents.h"', '#include "UIComponents.h"\n#include "LicenseManager.h"')

with open("Source/PluginEditor.h", "w") as f:
    f.write(h)
