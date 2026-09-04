with open("Source/PluginEditor.cpp", "r") as f:
    text = f.read()

text = text.replace('#include "PluginEditor.h"', '#include "PluginEditor.h"\n#include "BinaryData.h"')

with open("Source/PluginEditor.cpp", "w") as f:
    f.write(text)
