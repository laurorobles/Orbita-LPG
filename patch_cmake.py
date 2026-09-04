with open("CMakeLists.txt", "r") as f:
    text = f.read()

# Add binary data
bin_data = """juce_add_binary_data(OrbitaLPG_BinaryData
    HEADER_NAME "BinaryData.h"
    NAMESPACE "BinaryData"
    SOURCES
        assets/logo.png
)

target_link_libraries(OrbitaLPG PRIVATE"""
text = text.replace("target_link_libraries(OrbitaLPG PRIVATE", bin_data)

# Link binary data
text = text.replace("    juce::juce_gui_extra\n)", "    juce::juce_gui_extra\n    OrbitaLPG_BinaryData\n)")

# Bump version to 1.2.1
text = text.replace("VERSION 1.2.0", "VERSION 1.2.1")

with open("CMakeLists.txt", "w") as f:
    f.write(text)
