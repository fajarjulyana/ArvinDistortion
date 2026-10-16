#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================

class BackgroundComponent : public Component
{
public:
    BackgroundComponent();
    void paint(Graphics &g) override;

private:
    Image backgroundImage;
};

//==============================================================================

class KnobLookAndFeel : public LookAndFeel_V4
{
public:
    KnobLookAndFeel(Image knobImage);

    void drawRotarySlider(Graphics &g, int x, int y, int width, int height,
                          float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                          Slider &slider) override;

private:
    Image knobImage;
};

//==============================================================================

class DistortionAudioProcessorEditor : public AudioProcessorEditor
{
public:
    DistortionAudioProcessorEditor(DistortionAudioProcessor &);
    ~DistortionAudioProcessorEditor() override;

    void paint(Graphics &) override;
    void resized() override;

private:
    DistortionAudioProcessor &processor;

    enum
    {
        editorWidth = 500,
        editorMargin = 10,
        editorPadding = 10,

        sliderTextEntryBoxWidth = 100,
        sliderTextEntryBoxHeight = 25,
        sliderHeight = 64, // Increased height for knob
        buttonHeight = 25,
        comboBoxHeight = 25,
        labelWidth = 100,
    };

    //======================================

    BackgroundComponent *backgroundComponent;
    OwnedArray<Slider> sliders;
    OwnedArray<ToggleButton> toggles;
    OwnedArray<ComboBox> comboBoxes;
    OwnedArray<Label> labels;
    Array<Component *> components;

    typedef AudioProcessorValueTreeState::SliderAttachment SliderAttachment;
    typedef AudioProcessorValueTreeState::ButtonAttachment ButtonAttachment;
    typedef AudioProcessorValueTreeState::ComboBoxAttachment ComboBoxAttachment;

    OwnedArray<SliderAttachment> sliderAttachments;
    OwnedArray<ButtonAttachment> buttonAttachments;
    OwnedArray<ComboBoxAttachment> comboBoxAttachments;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DistortionAudioProcessorEditor)
};
