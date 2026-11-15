#include "PluginProcessor.h"
#include "PluginEditor.h"
void EQP1AProcessor::Filter::design(int type,double hz,double db,double q,double sr) {
 hz=juce::jlimit(5.,sr*.45,hz);double A=std::pow(10.,db/40.),w=juce::MathConstants<double>::twoPi*hz/sr,c=std::cos(w),s=std::sin(w),al=s/(2*q),a0;
 if(type==0){a0=1+al/A;b0=1+al*A;b1=-2*c;b2=1-al*A;a1=-2*c;a2=1-al/A;}
 else {double t=2*std::sqrt(A)*al;
 if(type==1){b0=A*((A+1)-(A-1)*c+t);b1=2*A*((A-1)-(A+1)*c);b2=A*((A+1)-(A-1)*c-t);a0=(A+1)+(A-1)*c+t;a1=-2*((A-1)+(A+1)*c);a2=(A+1)+(A-1)*c-t;}
 else {b0=A*((A+1)+(A-1)*c+t);b1=-2*A*((A-1)+(A+1)*c);b2=A*((A+1)+(A-1)*c-t);a0=(A+1)-(A-1)*c+t;a1=2*((A-1)-(A+1)*c);a2=(A+1)-(A-1)*c-t;}}
 b0/=a0;b1/=a0;b2/=a0;a1/=a0;a2/=a0;
}
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
void EQP1AProcessor::prepareToPlay(double sr,int){rate=sr;energyPole=std::exp(-1.0/(sr*.3));gainPole=std::exp(-1.0/(sr*.15));for(size_t i=0;i<smooth.size();++i){smooth[i].reset(sr,.035);smooth[i].setCurrentAndTargetValue(apvts.getRawParameterValue(ids[i])->load());}fixBlend.reset(sr,.035);fixBlend.setCurrentAndTargetValue(apvts.getRawParameterValue("fix")->load()>.5f?1.f:0.f);active.reset(sr,.015);active.setCurrentAndTargetValue(apvts.getRawParameterValue("power")->load()>.5f && apvts.getRawParameterValue("bypass")->load()<.5f?1.f:0.f);reset();update();}
void EQP1AProcessor::reset(){inputEnergy=outputEnergy=0;matchGain=1;for(auto& ch:filters)for(auto& f:ch)f.clear();}
bool EQP1AProcessor::isBusesLayoutSupported(const BusesLayout& l)const{return (l.getMainOutputChannelSet()==juce::AudioChannelSet::mono()||l.getMainOutputChannelSet()==juce::AudioChannelSet::stereo())&&l.getMainInputChannelSet()==l.getMainOutputChannelSet();}
void EQP1AProcessor::update(){
 static constexpr double lf[]={20,30,40,50,60,70,80,90,100},hf[]={1000,2000,3000,4000,5000,6000,7000,8000,9000,10000,11000,12000,13000,14000,15000,16000,17000,18000,19000,20000,30000,40000},af[]={5000,10000,20000};
 auto interp=[](const double* a,int n,float x){x=juce::jlimit(0.f,float(n-1),x);int i=int(x);return a[i]*std::pow(a[juce::jmin(i+1,n-1)]/a[i],x-i);};
 double low=interp(lf,9,smooth[2].getCurrentValue()),high=interp(hf,22,smooth[5].getCurrentValue()),cut=interp(af,3,smooth[7].getCurrentValue());
 for(auto& ch:filters){ch[0].design(1,low,1.35*smooth[0].getCurrentValue(),.72,rate);ch[1].design(1,low*2.1,-1.5*smooth[1].getCurrentValue(),.54,rate);ch[2].design(0,high,1.8*smooth[4].getCurrentValue(),2.8*std::pow(.16,smooth[3].getCurrentValue()/10.),rate);ch[3].design(2,cut,-1.6*smooth[6].getCurrentValue(),.6,rate);}
}
void EQP1AProcessor::processBlock(juce::AudioBuffer<float>& b,juce::MidiBuffer&){
 juce::ScopedNoDenormals no;for(int c=getTotalNumInputChannels();c<b.getNumChannels();++c)b.clear(c,0,b.getNumSamples());
 for(size_t i=0;i<smooth.size();++i)smooth[i].setTargetValue(apvts.getRawParameterValue(ids[i])->load());
 fixBlend.setTargetValue(apvts.getRawParameterValue("fix")->load()>.5f?1.f:0.f);
 active.setTargetValue(apvts.getRawParameterValue("power")->load()>.5f && apvts.getRawParameterValue("bypass")->load()<.5f?1.f:0.f);
 for(int i=0;i<b.getNumSamples();++i){for(auto& s:smooth)s.getNextValue();if((i&15)==0)update();float wet=active.getNextValue();double dry[2]{},eq[2]{},inPower=0,outPower=0;int channels=juce::jmin(2,b.getNumChannels());
 for(int c=0;c<channels;++c){double x=b.getSample(c,i);if(!std::isfinite(x))x=0;dry[c]=x;double y=x;for(auto& f:filters[size_t(c)])y=f.tick(y);eq[c]=y;inPower+=x*x;outPower+=y*y;}
 inputEnergy=energyPole*inputEnergy+(1-energyPole)*inPower;outputEnergy=energyPole*outputEnergy+(1-energyPole)*outPower;
 double target=matchGain;if(inputEnergy>1e-12 && outputEnergy>1e-16)target=juce::jlimit(.0158489319,63.09573445,std::sqrt(inputEnergy/outputEnergy));matchGain=gainPole*matchGain+(1-gainPole)*target;
 double compensation=1+fixBlend.getNextValue()*(matchGain-1);for(int c=0;c<channels;++c)b.setSample(c,i,float(dry[c]+wet*(eq[c]*compensation-dry[c])));
 }
}
void EQP1AProcessor::processBlockBypassed(juce::AudioBuffer<float>&,juce::MidiBuffer&){reset();}
juce::AudioProcessorParameter* EQP1AProcessor::getBypassParameter()const{return apvts.getParameter("bypass");}
void EQP1AProcessor::getStateInformation(juce::MemoryBlock& b){auto state=apvts.copyState();auto xml=state.createXml();copyXmlToBinary(*xml,b);}
void EQP1AProcessor::setStateInformation(const void* d,int n){if(auto xml=getXmlFromBinary(d,n))if(xml->hasTagName(apvts.state.getType())){auto state=juce::ValueTree::fromXml(*xml);apvts.replaceState(state);}}
juce::AudioProcessorEditor* EQP1AProcessor::createEditor(){return new EQP1AEditor(*this);}
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter(){return new EQP1AProcessor;}
