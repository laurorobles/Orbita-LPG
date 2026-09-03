import re

with open("Source/PluginEditor.cpp", "r") as f:
    c = f.read()

# Remove the broken injection
c = c.replace('    creditLabel.setBounds(getWidth() - 215, getHeight() - 18, 210, 15);\n}\n\nvoid OrbitaLPGAudioProcessorEditor::selectTrack(int t) {', 'void OrbitaLPGAudioProcessorEditor::selectTrack(int t) {')

# Inject into resized() correctly
c = c.replace(
'''        for (int i = 0; i < 4; ++i) {
            auto cell = ei.removeFromLeft(kw); auto kb = cell.withSizeKeepingCentre(38, 38);
            echoKnobs[i].slider.setBounds(kb); echoKnobs[i].label.setBounds(kb.getX()-8, kb.getY()-14, kb.getWidth()+16, 12);
        }
    }
}''',
'''        for (int i = 0; i < 4; ++i) {
            auto cell = ei.removeFromLeft(kw); auto kb = cell.withSizeKeepingCentre(38, 38);
            echoKnobs[i].slider.setBounds(kb); echoKnobs[i].label.setBounds(kb.getX()-8, kb.getY()-14, kb.getWidth()+16, 12);
        }
    }
    
    creditLabel.setBounds(getWidth() - 215, getHeight() - 18, 210, 15);
}'''
)

with open("Source/PluginEditor.cpp", "w") as f:
    f.write(c)

