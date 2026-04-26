#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class CustomLookAndFeel : public juce::LookAndFeel_V4
{
public:
    CustomLookAndFeel(juce::Image& knobImageToUse) : knobImage(knobImageToUse) {}

    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
                          float rotaryStartAngle, float rotaryEndAngle, juce::Slider& slider) override;

    int getNumFramesInAnimation() const { return numFramesInAnimation; }

private:
    juce::Image& knobImage;
    const int numFramesInAnimation = 1984 / 64;  // Sesuaikan dengan jumlah frame pada sprite sheet knob animasi
};

class ArvinDistortionAudioProcessorEditor : public juce::AudioProcessorEditor,
                                    public juce::Slider::Listener
{
public:
    ArvinDistortionAudioProcessorEditor(ArvinDistortionAudioProcessor&);
    ~ArvinDistortionAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

    // Callback for slider value changes
    void sliderValueChanged(juce::Slider* slider) override;

private:
    ArvinDistortionAudioProcessor& audioProcessor;

    juce::Image backgroundImage;
    juce::Image knobImage1;
    juce::Image knobImage2;
    juce::Slider knobSlider1;
    juce::Slider knobSlider2;

    std::unique_ptr<CustomLookAndFeel> customLookAndFeel1;
    std::unique_ptr<CustomLookAndFeel> customLookAndFeel2;

    int currentAnimationFrame;  // Untuk melacak frame animasi saat ini

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ArvinDistortionAudioProcessorEditor)
};
