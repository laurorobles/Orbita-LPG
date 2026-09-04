with open("Source/PluginEditor.cpp", "r") as f:
    text = f.read()

# Replace image loading
bad_loading = """    juce::File possiblePaths[] = {
        juce::File::getSpecialLocation(juce::File::currentExecutableFile).getParentDirectory().getChildFile("assets/logo.png"),
        juce::File::getCurrentWorkingDirectory().getChildFile("assets/logo.png"),
        juce::File("/Users/babyonk1/Desktop/ExtasisRecords/Orbita-LPG-JUCE/assets/logo.png")
    };
    
    for (const auto& f : possiblePaths) {
        if (f.existsAsFile()) {
            logoImage = juce::ImageCache::getFromFile(f);
            if (logoImage.isValid()) break;
        }
    }"""

good_loading = """    logoImage = juce::ImageCache::getFromMemory(BinaryData::logo_png, BinaryData::logo_pngSize);"""

text = text.replace(bad_loading, good_loading)

# Replace logo size
text = text.replace("float logoSize = maxR * 0.55f;", "float logoSize = maxR * 1.9f;")

with open("Source/PluginEditor.cpp", "w") as f:
    f.write(text)
