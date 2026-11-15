#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <array>
#include <atomic>
#include <cmath>

class EQP1AProcessor final : public juce::AudioProcessor
{
public:
    EQP1AProcessor();
    void prepareToPlay(double, int) override;
    void releaseResources() override {}
    void reset() override;
    bool isBusesLayoutSupported(const BusesLayout&) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    void processBlockBypassed(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    juce::AudioProcessorParameter* getBypassParameter() const override;
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return "EQP-1A"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 1.0; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return "Default"; }
    void changeProgramName(int, const juce::String&) override {}
    void getStateInformation(juce::MemoryBlock&) override;
    void setStateInformation(const void*, int) override;
    static juce::AudioProcessorValueTreeState::ParameterLayout makeParameters();
    juce::AudioProcessorValueTreeState apvts;
    std::atomic<int> editorWidth { 1334 }, editorHeight { 340 };
private:
    struct Filter {
        double b0=1,b1=0,b2=0,a1=0,a2=0,z1=0,z2=0;
        double tick(double x) { double y=b0*x+z1;z1=b1*x-a1*y+z2;z2=b2*x-a2*y;return y; }
        void clear(){ z1=z2=0; }
    };
    struct Branch {
        std::array<std::array<Filter,2>,2> channels;
        int key=-1;
        double weight=0;
    };
    struct Group {
        std::array<Branch,8> branches;
        int count=0, sections=1;
        double tick(double x,int channel) noexcept {
            double sum=0;
            for(int i=0;i<count;++i) {
                auto& branch=branches[size_t(i)];
                double y=x;
                for(int j=0;j<sections;++j)y=branch.channels[size_t(channel)][size_t(j)].tick(y);
                sum+=branch.weight*y;
            }
            return sum;
        }
    };
    Group lowGroup,highGroup,attenGroup;
    std::array<juce::SmoothedValue<float>,8> smooth;
    juce::SmoothedValue<float> active,fixBlend;
    std::array<std::array<double,256>,2> colourHistory{},dryHistory{};
    std::array<double,2> hpInput{},hpOutput{};
    int calibrationIndex=1,historyIndex=0;
    double rate=44100,hpPole=0,inputEnergy=0,outputEnergy=0,matchGain=1,energyPole=0,gainPole=0;
    void update();
    void processAudio(juce::AudioBuffer<float>&,bool);
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EQP1AProcessor)
};
