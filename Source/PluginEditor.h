#pragma once
#include "PluginProcessor.h"
#include <juce_gui_basics/juce_gui_basics.h>

inline constexpr int kBaseWidth = 1334, kBaseHeight = 340;

class EQP1ALookAndFeel final : public juce::LookAndFeel_V4
{
public:
    void drawRotarySlider(juce::Graphics&, int, int, int, int, float, float, float, juce::Slider&) override;
    void drawToggleButton(juce::Graphics&, juce::ToggleButton&, bool, bool) override;
};

class FaceplateComponent final : public juce::Component, private juce::Timer
{
public:
    explicit FaceplateComponent(EQP1AProcessor&);
    ~FaceplateComponent() override;
    void paint(juce::Graphics&) override;
    void resized() override;
private:
    void timerCallback() override;

    EQP1AProcessor& processor;
    EQP1ALookAndFeel look;
    std::array<juce::Slider,8> knobs;
    std::array<juce::ToggleButton,2> switches;
    using SA = juce::AudioProcessorValueTreeState::SliderAttachment;
    using BA = juce::AudioProcessorValueTreeState::ButtonAttachment;
    std::array<std::unique_ptr<SA>,8> knobAttachments;
    std::array<std::unique_ptr<BA>,2> switchAttachments;
    float shownGr = 0, shownOut = -100;
    juce::TooltipWindow tooltips { this, 500 };
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FaceplateComponent)
};

class EQP1AEditor final : public juce::AudioProcessorEditor
{
public:
    explicit EQP1AEditor(EQP1AProcessor&);
    void resized() override;
private:
    EQP1AProcessor& processor;
    FaceplateComponent faceplate;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EQP1AEditor)
};
