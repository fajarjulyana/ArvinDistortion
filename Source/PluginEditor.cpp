#include "PluginProcessor.h"
#include "PluginEditor.h"

void CustomLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
                                         float rotaryStartAngle, float rotaryEndAngle, juce::Slider& slider)
{
    // Ambil ukuran satu frame animasi
    int frameWidth = 64;
    int frameHeight = 64;

    // Hitung frame animasi yang akan digambar
    int frameToDraw = static_cast<int>(sliderPos * numFramesInAnimation) % numFramesInAnimation;

    // Gambar frame animasi saat ini
    g.drawImage(knobImage, x, y, width, height,
                0, frameToDraw * frameHeight, frameWidth, frameHeight);
}

ArvinDistortionAudioProcessorEditor::ArvinDistortionAudioProcessorEditor(ArvinDistortionAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p), currentAnimationFrame(0)
{
    setSize(200, 359);

    // Load the background image
    backgroundImage = juce::ImageCache::getFromMemory(BinaryData::background_png, BinaryData::background_pngSize);

    // Load the knob images
    knobImage1 = juce::ImageCache::getFromMemory(BinaryData::knob_png, BinaryData::knob_pngSize);
    knobImage2 = juce::ImageCache::getFromMemory(BinaryData::knob_png, BinaryData::knob_pngSize);

    // Setup knob sliders with custom LookAndFeel
    addAndMakeVisible(knobSlider1);
    knobSlider1.setSliderStyle(juce::Slider::Rotary);
    knobSlider1.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    knobSlider1.addListener(this);

    addAndMakeVisible(knobSlider2);
    knobSlider2.setSliderStyle(juce::Slider::Rotary);
    knobSlider2.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    knobSlider2.addListener(this);

    // Pass knobImage reference to CustomLookAndFeel
    customLookAndFeel1 = std::make_unique<CustomLookAndFeel>(knobImage1);
    knobSlider1.setLookAndFeel(customLookAndFeel1.get());

    // Pass knobImage reference to CustomLookAndFeel for the second knob
    customLookAndFeel2 = std::make_unique<CustomLookAndFeel>(knobImage2);
    knobSlider2.setLookAndFeel(customLookAndFeel2.get());
}

ArvinDistortionAudioProcessorEditor::~ArvinDistortionAudioProcessorEditor()
{
    knobSlider1.setLookAndFeel(nullptr);
    knobSlider2.setLookAndFeel(nullptr);
}

void ArvinDistortionAudioProcessorEditor::paint(juce::Graphics& g)
{
    // Draw the background image
    if (!backgroundImage.isNull())
        g.drawImage(backgroundImage, getLocalBounds().toFloat());

    // Draw other UI components on top of the background as needed
    g.setColour(juce::Colours::white);
    g.setFont(15.0f);
    g.drawFittedText("Hello World!", getLocalBounds(), juce::Justification::centred, 1);
}

void ArvinDistortionAudioProcessorEditor::resized()
{
    // Setel ulang batasan (bounds) knobSlider1
    knobSlider1.setBounds(20, 40, 64, 64);

    // Setel ulang batasan (bounds) knobSlider2
    knobSlider2.setBounds(110, 40, 64, 64);
}

void ArvinDistortionAudioProcessorEditor::sliderValueChanged(juce::Slider* slider)
{
    // Ganti frame animasi saat nilai slider berubah
    currentAnimationFrame = static_cast<int>(slider->getValue() * customLookAndFeel1->getNumFramesInAnimation()) %
                            customLookAndFeel1->getNumFramesInAnimation();

    // Repaint agar tampilan diperbarui
    repaint();
}
