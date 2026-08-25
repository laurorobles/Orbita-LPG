import re

with open("Source/PluginEditor.cpp", "r") as f:
    c = f.read()

c = c.replace("noteBtn.setBounds(    sRow1.removeFromRight(50).reduced(0,1));", "noteBtn.setBounds(    sRow1.removeFromRight(70).reduced(0,1));")
c = c.replace("copyNextBtn.setBounds(sRow1.removeFromRight(90).reduced(0,1));", "copyNextBtn.setBounds(sRow1.removeFromRight(80).reduced(0,1));")
c = c.replace("copyLastBtn.setBounds(sRow1.removeFromRight(90).reduced(0,1));", "copyLastBtn.setBounds(sRow1.removeFromRight(80).reduced(0,1));")

with open("Source/PluginEditor.cpp", "w") as f:
    f.write(c)
