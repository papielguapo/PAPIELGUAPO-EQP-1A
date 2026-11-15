#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "CalibrationData.h"
#include "InteractionData.h"
#include "InteractionMath.h"
juce::AudioProcessorValueTreeState::ParameterLayout EQP1AProcessor::makeParameters(){
 juce::AudioProcessorValueTreeState::ParameterLayout p;
 auto f=[&](const char* id,const char* name,float lo,float hi,float value){p.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{id,1},name,juce::NormalisableRange<float>(lo,hi,.01f),value));};
 f("lowBoost","Low Boost",0,10,0);f("lowAtten","Low Attenuation",0,10,0);
 p.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{"lowFreq",1},"Low Frequency",juce::StringArray{"20","30","40","50","60","70","80","90","100"},4));
 f("bandwidth","Bandwidth",0,10,5);f("highBoost","High Boost",0,10,0);
 p.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{"highFreq",1},"High Frequency",juce::StringArray{"1","2","3","4","5","6","7","8","9","10","11","12","13","14","15","16","17","18","19","20","30","40"},7));
 f("highAtten","High Attenuation",0,10,0);
 p.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{"attenFreq",1},"Attenuation Frequency",juce::StringArray{"5","10","20"},1));

 p.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{"fix",1},"FIX Auto Level",true));
 p.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{"power",1},"Power",true));
 p.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{"bypass",1},"Bypass",false));return p;
}
EQP1AProcessor::EQP1AProcessor():AudioProcessor(BusesProperties().withInput("Input",juce::AudioChannelSet::stereo(),true).withOutput("Output",juce::AudioChannelSet::stereo(),true)),apvts(*this,nullptr,"EQP1A",makeParameters()){}
static const char* ids[]={"lowBoost","lowAtten","lowFreq","bandwidth","highBoost","highFreq","highAtten","attenFreq"};

namespace {
struct Axis { int lo,hi; double mix; };
Axis axis(double v,int count) {
    v=juce::jlimit(0.,double(count-1),v);
    int lo=int(v);return {lo,juce::jmin(lo+1,count-1),v-lo};
}
Axis frequencyAxis(double hz,const double* frequencies,int count) {
    if(hz<=frequencies[0])return {0,0,0};
    for(int i=0;i<count-1;++i)if(hz<=frequencies[i+1])
        return {i,i+1,std::log(hz/frequencies[i])/std::log(frequencies[i+1]/frequencies[i])};
    return {count-1,count-1,0};
}
void transform(double* c,double referenceRate,double sampleRate,double shift) {
    if(referenceRate==sampleRate && std::abs(shift-1.)<1e-12)return;
    const double k=2*referenceRate,k2=k*k;
    double n2=c[0]-c[1]+c[2], n1=2*k*(c[0]-c[2])*shift, n0=k2*(c[0]+c[1]+c[2])*shift*shift;
    double d2=1-c[3]+c[4],d1=2*k*(1-c[4])*shift,d0=k2*(1+c[3]+c[4])*shift*shift;
    const double q=2*sampleRate,q2=q*q,norm=d2*q2+d1*q+d0;
    c[0]=(n2*q2+n1*q+n0)/norm;c[1]=2*(n0-n2*q2)/norm;c[2]=(n2*q2-n1*q+n0)/norm;
    c[3]=2*(d0-d2*q2)/norm;c[4]=(d2*q2-d1*q+d0)/norm;
}
}
void EQP1AProcessor::prepareToPlay(double sr,int) {
    rate=sr;calibrationIndex=0;
    for(int i=1;i<4;++i)if(std::abs(std::log(sr/EQPCalibration::rates[i].sr))<std::abs(std::log(sr/EQPCalibration::rates[calibrationIndex].sr)))calibrationIndex=i;
    setLatencySamples(EQPCalibration::rates[calibrationIndex].latency);
    energyPole=std::exp(-1/(sr*.3));gainPole=std::exp(-1/(sr*.15));hpPole=std::exp(-juce::MathConstants<double>::twoPi/sr);
    for(size_t i=0;i<smooth.size();++i){smooth[i].reset(sr,.035);smooth[i].setCurrentAndTargetValue(apvts.getRawParameterValue(ids[i])->load());}
    fixBlend.reset(sr,.035);fixBlend.setCurrentAndTargetValue(apvts.getRawParameterValue("fix")->load()>.5f?1.f:0.f);
    active.reset(sr,.015);active.setCurrentAndTargetValue(apvts.getRawParameterValue("power")->load()>.5f && apvts.getRawParameterValue("bypass")->load()<.5f?1.f:0.f);
    reset();update();
}
void EQP1AProcessor::reset() {
    inputEnergy=outputEnergy=0;matchGain=1;historyIndex=0;
    lowGroup={};highGroup={};legacyLow={};legacyHigh={};legacyAtten={};highBlend=0;for(auto& channel:interactionFilters)for(auto& filter:channel)filter={};interactionGain=1;
    for(auto& c:colourHistory)c.fill(0);for(auto& c:dryHistory)c.fill(0);
    hpInput.fill(0);hpOutput.fill(0);update();
}
bool EQP1AProcessor::isBusesLayoutSupported(const BusesLayout& l)const {
    return (l.getMainOutputChannelSet()==juce::AudioChannelSet::mono()||l.getMainOutputChannelSet()==juce::AudioChannelSet::stereo())&&l.getMainInputChannelSet()==l.getMainOutputChannelSet();
}

namespace {
struct Stencil { int count=0; std::array<int,4> index{}; std::array<double,4> weight{}; };
Stencil linearStencil(Axis a) {
    Stencil s;s.count=2;s.index[0]=a.lo;s.index[1]=a.hi;s.weight[0]=1-a.mix;s.weight[1]=a.mix;return s;
}
Stencil cubicStencil(double value,int count) {
    value=juce::jlimit(0.,double(count-1),value);Stencil s;s.count=juce::jmin(4,count);
    const int start=juce::jlimit(0,count-s.count,int(value)-1);
    for(int i=0;i<s.count;++i){s.index[size_t(i)]=start+i;double weight=1;for(int j=0;j<s.count;++j)if(j!=i)weight*=(value-(start+j))/double(i-j);s.weight[size_t(i)]=weight;}
    return s;
}
void analogCoefficients(double* c,double n2,double n1,double n0,double d2,double d1,double d0,double sr) {
    double k=2*sr,k2=k*k,norm=d2*k2+d1*k+d0;
    c[0]=(n2*k2+n1*k+n0)/norm;c[1]=2*(n0-n2*k2)/norm;c[2]=(n2*k2-n1*k+n0)/norm;
    c[3]=2*(d0-d2*k2)/norm;c[4]=(d2*k2-d1*k+d0)/norm;
}
}
void EQP1AProcessor::update() {
    const auto& data=EQPCalibration::rates[calibrationIndex];const auto& interactions=EQPInteractions::rates[calibrationIndex];
    static constexpr double lf[]={20,30,60,100},hf[]={3000,4000,5000,8000,10000,12000,16000};
    Axis l=frequencyAxis(20+10*smooth[2].getCurrentValue(),lf,4),lb=axis(smooth[0].getCurrentValue(),11),la=axis(smooth[1].getCurrentValue(),11);
    double choice=smooth[5].getCurrentValue();double highHz=choice<=19?(choice+1)*1000:choice<=20?20000+(choice-19)*10000:30000+(choice-20)*10000;
    highHz=juce::jmin(highHz,rate*.45);Axis h=frequencyAxis(highHz,hf,7),af=axis(smooth[7].getCurrentValue(),3);
    double highShift=1;
    if(highHz<hf[0]||highHz>hf[6]){double anchor=highHz<hf[0]?hf[0]:hf[6];highShift=std::tan(juce::MathConstants<double>::pi*highHz/rate)/std::tan(juce::MathConstants<double>::pi*juce::jmin(anchor,rate*.45)/rate);}
    auto set=[&](Group& group,int section,double* c,double shift=1.) {
        transform(c,data.sr,rate,shift);
        for(auto& channel:group.channels){auto& filter=channel[size_t(section)];filter.b0=c[0];filter.b1=c[1];filter.b2=c[2];filter.a1=c[3];filter.a2=c[4];}
    };
    // Convex interpolation of stable low-band biquads avoids cancellation between parallel paths.
    double low[5]{};Axis axes[]{l,lb,la};
    for(int slot=0;slot<8;++slot){int ix[3]{};double weight=1;for(int j=0;j<3;++j){bool upper=(slot&(1<<j))!=0;ix[j]=upper?axes[j].hi:axes[j].lo;weight*=upper?axes[j].mix:1-axes[j].mix;}if(weight==0)continue;auto* source=data.low[ix[0]][ix[1]][ix[2]];for(int j=0;j<5;++j)low[j]+=weight*source[j];}
    lowGroup.sections=1;set(lowGroup,0,low);
    auto blend=[&](const Stencil* stencils,int kind,double* result,int values) {
        std::fill(result,result+values,0.);int total=1;for(int j=0;j<5;++j)total*=stencils[j].count;
        for(int vertex=0;vertex<total;++vertex){int code=vertex,ix[5]{};double weight=1;for(int j=0;j<5;++j){int k=code%stencils[j].count;code/=stencils[j].count;ix[j]=stencils[j].index[size_t(k)];weight*=stencils[j].weight[size_t(k)];}if(std::abs(weight)<1e-14)continue;
            const float* source=kind==0?interactions.low[ix[0]][ix[1]][ix[2]][ix[3]][ix[4]]:interactions.high[ix[0]][ix[1]][ix[2]][ix[3]][ix[4]];
            for(int j=0;j<values;++j)result[j]+=weight*source[j];
        }
    };

    double hbValue=smooth[4].getCurrentValue()/2.5,bwValue=smooth[3].getCurrentValue()/5.,haValue=smooth[6].getCurrentValue()/2.5;
    auto configureLegacy=[&](LegacyGroup& group,const Axis* axes,int dimensions,int sections,int kind,double shift) {
        auto previous=group.branches;group.count=1<<dimensions;group.sections=sections;
        for(int slot=0;slot<group.count;++slot){int indices[3]{};double weight=1;for(int j=0;j<dimensions;++j){bool upper=(slot&(1<<j))!=0;indices[j]=upper?axes[j].hi:axes[j].lo;weight*=upper?axes[j].mix:1-axes[j].mix;}int key=indices[0]*10000+indices[1]*100+indices[2];LegacyBranch next;for(const auto& old:previous)if(old.key==key){next=old;break;}next.key=key;next.weight=weight;const double* source=kind==0?data.high[indices[0]][indices[1]][indices[2]]:kind==1?data.atten[indices[0]][indices[1]]:data.low[indices[0]][indices[1]][indices[2]];for(int section=0;section<sections;++section){double c[5];std::copy(source+section*5,source+section*5+5,c);transform(c,data.sr,rate,shift);for(auto& channel:next.channels){auto& f=channel[size_t(section)];f.b0=c[0];f.b1=c[1];f.b2=c[2];f.a1=c[3];f.a2=c[4];}}group.branches[size_t(slot)]=next;}
    };
    Axis legacyHighAxes[]{h,axis(smooth[4].getCurrentValue(),11),axis(smooth[3].getCurrentValue()/2.,6)},legacyAttenAxes[]{af,axis(smooth[6].getCurrentValue(),11)};
    configureLegacy(legacyLow,axes,3,1,2,1.);
    configureLegacy(legacyHigh,legacyHighAxes,3,2,0,highShift);configureLegacy(legacyAtten,legacyAttenAxes,2,1,1,1.);
    auto coupling=[](double value){double x=juce::jlimit(0.,1.,(value-7.5)/2.);return x*x*(3-2*x);};
    highBlend=coupling(smooth[4].getCurrentValue())*coupling(smooth[6].getCurrentValue());
    if(highHz<hf[0]||highHz>hf[6])highBlend=0;
    if(highBlend>0){
        Stencil highAxes[]{linearStencil(h),cubicStencil(hbValue,5),cubicStencil(bwValue,3),linearStencil(af),cubicStencil(haValue,5)};
        double joint[12],sections[3][5];blend(highAxes,1,joint,12);
        bool valid=EQPInteractionMath::toSections(joint,data.sr,sections);
        if(!valid){Stencil linearAxes[]{linearStencil(h),linearStencil(axis(hbValue,5)),linearStencil(axis(bwValue,3)),linearStencil(af),linearStencil(axis(haValue,5))};blend(linearAxes,1,joint,12);valid=EQPInteractionMath::toSections(joint,data.sr,sections);}
        if(valid){highGroup.sections=3;for(int j=0;j<3;++j)set(highGroup,j,sections[j]);}else highBlend=0;
    }
    // Broad loading interaction between low and high attenuation, expressed as stable fixed-Q sections.
    Stencil lowAxes[]{linearStencil(l),cubicStencil(smooth[0].getCurrentValue()/2.5,5),cubicStencil(smooth[1].getCurrentValue()/2.5,5),linearStencil(af),cubicStencil(haValue,5)};
    double corrections[32];blend(lowAxes,0,corrections,32);interactionGain=std::pow(10.,juce::jlimit(-24.,24.,corrections[0])/20.);
    for(int band=0;band<31;++band){double fc=juce::jmin(20000.,20*std::pow(2.,band/3.));fc=juce::jmin(fc,.45*rate);double centre=2*rate*std::tan(juce::MathConstants<double>::pi*fc/rate),a=std::pow(10.,juce::jlimit(-24.,24.,corrections[band+1])/40.),c[5];analogCoefficients(c,1,centre/1.8*a,centre*centre,1,centre/1.8/a,centre*centre,rate);for(auto& channel:interactionFilters){auto& filter=channel[size_t(band)];filter.b0=c[0];filter.b1=c[1];filter.b2=c[2];filter.a1=c[3];filter.a2=c[4];}}
}
void EQP1AProcessor::processAudio(juce::AudioBuffer<float>& b,bool forcedBypass) {
    juce::ScopedNoDenormals no;
    for(int c=getTotalNumInputChannels();c<b.getNumChannels();++c)b.clear(c,0,b.getNumSamples());
    bool moving=false;
    for(size_t i=0;i<smooth.size();++i){smooth[i].setTargetValue(apvts.getRawParameterValue(ids[i])->load());moving=moving||smooth[i].isSmoothing();}
    fixBlend.setTargetValue(apvts.getRawParameterValue("fix")->load()>.5f?1.f:0.f);
    active.setTargetValue(!forcedBypass && apvts.getRawParameterValue("power")->load()>.5f && apvts.getRawParameterValue("bypass")->load()<.5f?1.f:0.f);
    const auto& data=EQPCalibration::rates[calibrationIndex];
    const int channels=juce::jmin(2,b.getNumChannels());
    for(int i=0;i<b.getNumSamples();++i) {
        if(moving){for(auto& s:smooth)s.getNextValue();if((i&15)==0)update();}
        const double wet=active.getNextValue();double dry[2]{},equalized[2]{},inPower=0,outPower=0;
        for(int c=0;c<channels;++c) {
            double x=b.getSample(c,i);if(!std::isfinite(x))x=0;
            auto ch=size_t(c);dryHistory[ch][size_t(historyIndex)]=x;dry[c]=dryHistory[ch][size_t((historyIndex-data.latency)&255)];
            double low=legacyLow.tick(x,c);double independent=legacyAtten.tick(legacyHigh.tick(low,c),c);double modern=highGroup.tick(low,c);double y=(highBlend*modern+(1-highBlend)*independent)*interactionGain;for(auto& filter:interactionFilters[ch])y=filter.tick(y);
            colourHistory[ch][size_t(historyIndex)]=y;y=0;
            for(int k=0;k<data.kernelSize;++k)y+=data.kernel[k]*colourHistory[ch][size_t((historyIndex-k)&255)];
            double hp=(1+hpPole)*.5*(y-hpInput[ch])+hpPole*hpOutput[ch];hpInput[ch]=y;hpOutput[ch]=hp;
            equalized[c]=hp;inPower+=dry[c]*dry[c];outPower+=hp*hp;
        }
        historyIndex=(historyIndex+1)&255;
        inputEnergy=energyPole*inputEnergy+(1-energyPole)*inPower;outputEnergy=energyPole*outputEnergy+(1-energyPole)*outPower;
        double target=matchGain;if(inputEnergy>1e-12&&outputEnergy>1e-16)target=juce::jlimit(.0158489319,63.09573445,std::sqrt(inputEnergy/outputEnergy));
        matchGain=gainPole*matchGain+(1-gainPole)*target;double compensation=1+fixBlend.getNextValue()*(matchGain-1);
        for(int c=0;c<channels;++c)b.setSample(c,i,float(dry[c]+wet*(equalized[c]*compensation-dry[c])));
    }
    if(moving)update();
}
void EQP1AProcessor::processBlock(juce::AudioBuffer<float>& b,juce::MidiBuffer&){processAudio(b,false);}
void EQP1AProcessor::processBlockBypassed(juce::AudioBuffer<float>& b,juce::MidiBuffer&){processAudio(b,true);}
juce::AudioProcessorParameter* EQP1AProcessor::getBypassParameter()const{return apvts.getParameter("bypass");}
void EQP1AProcessor::getStateInformation(juce::MemoryBlock& b){auto state=apvts.copyState();auto xml=state.createXml();copyXmlToBinary(*xml,b);}
void EQP1AProcessor::setStateInformation(const void* d,int n){if(auto xml=getXmlFromBinary(d,n))if(xml->hasTagName(apvts.state.getType())){auto state=juce::ValueTree::fromXml(*xml);apvts.replaceState(state);}}
juce::AudioProcessorEditor* EQP1AProcessor::createEditor(){return new EQP1AEditor(*this);}
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter(){return new EQP1AProcessor;}
