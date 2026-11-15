#include "PluginEditor.h"
#include <cmath>

namespace
{
juce::Colour colour(juce::uint32 hex) { return juce::Colour(hex); }
juce::Font font(const juce::String& name,float size,int style)
{
#if JUCE_MAJOR_VERSION >= 8
    return juce::Font(juce::FontOptions(name,size,style));
#else
    return juce::Font(name,size,style);
#endif
}
void text(juce::Graphics& g, const juce::String& s, float x, float y, float w, float h, float size = 10, bool bold = true)
{
    g.setFont(font("Arial", size, bold ? juce::Font::bold : juce::Font::plain));
    g.drawText(s, juce::Rectangle<float>(x,y,w,h), juce::Justification::centred, false);
}
void screw(juce::Graphics& g, float x, float y)
{
    g.setColour(juce::Colours::white.withAlpha(0.42f)); g.fillEllipse(x-15,y-14,31,31);
    g.setGradientFill(juce::ColourGradient(colour(0xff080b0c),x-8,y-12,colour(0xff353b3c),x+9,y+12,false));
    g.fillEllipse(x-15,y-15,30,30);
    g.setGradientFill(juce::ColourGradient(colour(0xff191f20),x-3,y-6,colour(0xff020404),x+4,y+5,true));
    g.fillEllipse(x-6,y-6,12,12);
}
void lamp(juce::Graphics& g, juce::Rectangle<float> r, bool on)
{
    const auto c=r.getCentre();
    if(on) { g.setGradientFill(juce::ColourGradient(colour(0xff9b26ff).withAlpha(0.35f),c.x,c.y,juce::Colours::transparentBlack,c.x+r.getWidth()*0.85f,c.y,true));g.fillEllipse(r.expanded(r.getWidth()*0.4f)); }
    g.setColour(juce::Colours::black.withAlpha(0.3f));g.fillEllipse(r.translated(2,3));
    g.setGradientFill(juce::ColourGradient(colour(0xff72469d),r.getX(),r.getY(),colour(0xff260438),r.getRight(),r.getBottom(),false));g.fillEllipse(r);
    auto inner=r.reduced(5);
    if(on) { auto jewel=juce::ColourGradient(colour(0xffdf9aff),c.x-10,c.y-13,colour(0xff490078),c.x+24,c.y+20,true);jewel.addColour(0.36,colour(0xffb839ff));jewel.addColour(0.74,colour(0xff7900ce));g.setGradientFill(jewel);g.fillEllipse(inner);g.setColour(colour(0xffdd8fff).withAlpha(0.23f));g.drawEllipse(inner.reduced(1),2); }
    else { g.setColour(colour(0xff240046));g.fillEllipse(inner); }
    g.setColour(juce::Colours::white.withAlpha(on?0.83f:0.12f));g.fillEllipse(r.getX()+13,r.getY()+12,r.getWidth()*0.30f,r.getHeight()*0.19f);
    if(on){g.setColour(colour(0xffdc8fff).withAlpha(0.36f));g.fillEllipse(c.x+4,c.y+11,13,7);}
}
}

void EQP1ALookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                                              float pos, float start, float end, juce::Slider& slider)
{
    const float r = static_cast<float>(juce::jmin(width, height)) * 0.46f;
    const float cx = static_cast<float>(x) + static_cast<float>(width) * 0.5f;
    const float cy = static_cast<float>(y) + static_cast<float>(height) * 0.5f;
    g.setColour(juce::Colours::black.withAlpha(0.38f));
    g.fillEllipse(cx-r+2,cy-r+5,r*2+2,r*2+2);
    g.setColour(colour(0xff24202c));g.fillEllipse(cx-r-1,cy-r-1,r*2+2,r*2+2);
    auto resin=juce::ColourGradient(colour(0xffaf40ff),cx-r*0.55f,cy-r*0.75f,
                                   colour(0xff200034),cx+r*0.8f,cy+r*0.9f,false);
    resin.addColour(0.34,colour(0xff7e12d5));resin.addColour(0.65,colour(0xff51008e));
    g.setGradientFill(resin);g.fillEllipse(cx-r,cy-r,r*2,r*2);
    g.setColour(colour(0xffdd9cff).withAlpha(0.8f));g.drawEllipse(cx-r+1.4f,cy-r+1.4f,r*2-2.8f,r*2-2.8f,1.1f);
    g.setColour(colour(0xff26003f).withAlpha(0.75f));g.drawEllipse(cx-r+4,cy-r+4,r*2-8,r*2-8,1.4f);
    juce::Graphics::ScopedSaveState save(g);
    g.addTransform(juce::AffineTransform::rotation(start+pos*(end-start)).translated(cx,cy));
    // Narrow concave channels follow the fin; the stationary resin skirt catches the light.
    for(float side : {-1.0f,1.0f})
    {
        juce::Path flute;
        flute.startNewSubPath(side*r*0.25f,-r*0.88f);
        flute.cubicTo(side*r*0.73f,-r*0.63f,side*r*0.77f,r*0.46f,side*r*0.30f,r*0.85f);
        flute.cubicTo(side*r*0.47f,r*0.25f,side*r*0.47f,-r*0.32f,side*r*0.25f,-r*0.88f);
        flute.closeSubPath();g.setColour(colour(0xff25003e));g.fillPath(flute);
        juce::Path rib;rib.startNewSubPath(side*r*0.55f,-r*0.67f);
        rib.cubicTo(side*r*0.77f,-r*0.27f,side*r*0.73f,r*0.32f,side*r*0.48f,r*0.67f);
        g.setColour(colour(0xffc45cff).withAlpha(side<0?0.56f:0.22f));g.strokePath(rib,juce::PathStrokeType(1.0f));
    }
    juce::Path fin;
    fin.startNewSubPath(-r*0.07f,-r*0.94f);
    fin.cubicTo(r*0.12f,-r*0.97f,r*0.18f,-r*0.70f,r*0.19f,-r*0.25f);
    fin.lineTo(r*0.21f,r*0.58f);
    fin.quadraticTo(r*0.24f,r*0.86f,r*0.04f,r*0.91f);
    fin.quadraticTo(-r*0.18f,r*0.94f,-r*0.20f,r*0.68f);
    fin.lineTo(-r*0.19f,-r*0.60f);
    fin.quadraticTo(-r*0.19f,-r*0.89f,-r*0.07f,-r*0.94f);fin.closeSubPath();
    auto ridge=juce::ColourGradient(colour(0xffe3a2ff),-r*0.20f,0,colour(0xff3c0065),r*0.23f,0,false);
    ridge.addColour(0.23,colour(0xffb14cff));ridge.addColour(0.40,colour(0xff7b13c0));
    ridge.addColour(0.63,colour(0xff6500a8));g.setGradientFill(ridge);g.fillPath(fin);
    g.setColour(colour(0xffcc82ff).withAlpha(0.72f));g.strokePath(fin,juce::PathStrokeType(0.8f));
    g.setColour(colour(0xfff8efff));g.fillRoundedRectangle(-r*0.09f,-r*0.87f,r*0.075f,r*0.49f,0.7f);
    if(slider.hasKeyboardFocus(true)) {g.setColour(colour(0xffefd4ff));g.drawEllipse(-r-3,-r-3,r*2+6,r*2+6,1);}
}

void EQP1ALookAndFeel::drawToggleButton(juce::Graphics& g, juce::ToggleButton& b, bool hover, bool down)
{
    const auto c=b.getLocalBounds().toFloat().getCentre();
    const bool right=b.getToggleState()!=static_cast<bool>(b.getProperties().getWithDefault("reverse",false));
    const float bx=c.x+3, tip=bx+(right?12.0f:-12.0f);
    g.setColour(juce::Colours::black.withAlpha(0.28f));g.fillEllipse(bx-12,c.y-11,27,28);
    g.setGradientFill(juce::ColourGradient(colour(0xff555b5e),bx-8,c.y-12,colour(0xff020506),bx+9,c.y+13,false));
    g.fillEllipse(bx-13,c.y-13,26,26);
    g.setColour(colour(0xff090c0d));g.fillEllipse(bx-10,c.y-10,20,20);
    g.setGradientFill(juce::ColourGradient(colour(0xff4c555b),tip,c.y-7,colour(0xff080b0d),tip,c.y+7,false));
    g.fillRoundedRectangle(juce::jmin(tip,bx)-5,c.y-7,std::abs(tip-bx)+10,14,6);
    g.setColour(colour(0xff030607));g.fillEllipse(tip-8,c.y-8,16,16);
    g.setGradientFill(juce::ColourGradient(colour(0xff515a60),tip-3,c.y-5,colour(0xff101416),tip+4,c.y+5,false));
    g.fillEllipse(tip-5,c.y-5.5f,10,11);
    if(hover||down||b.hasKeyboardFocus(true)){g.setColour(colour(0xffb84dff).withAlpha(0.65f));g.drawEllipse(bx-15,c.y-15,30,30,1);}
}

static constexpr int xs[]={360,510,435,610,710,785,860,1010,1010};
static constexpr int ys[]={105,105,245,245,105,245,105,105,245};
FaceplateComponent::FaceplateComponent(EQP1AProcessor& p):processor(p){setLookAndFeel(&look);
 const char* names[]={"Low Boost","Low Attenuation","Low Frequency","Bandwidth","High Boost","High Frequency","High Attenuation","Attenuation Frequency","Output"};
 const char* params[]={"lowBoost","lowAtten","lowFreq","bandwidth","highBoost","highFreq","highAtten","attenFreq","output"};
 for(size_t i=0;i<knobs.size();++i){auto& s=knobs[i];addAndMakeVisible(s);s.setName(names[i]);s.setTitle(names[i]);if(i==8)s.setTooltip("Output after FIX: final level -12 to +12 dB");s.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);s.setTextBoxStyle(juce::Slider::NoTextBox,false,0,0);s.setRotaryParameters(juce::MathConstants<float>::pi*7/6,juce::MathConstants<float>::pi*17/6,true);s.setPopupDisplayEnabled(true,true,this);s.setDoubleClickReturnValue(true,i==2?4:i==3?5:i==5?7:i==7?1:0);
 s.textFromValueFunction=[i](double v){int n=juce::roundToInt(v);if(i==2){return juce::String(20+10*juce::jlimit(0,8,n))+" Hz";}if(i==5){n=juce::jlimit(0,21,n);return juce::String(n<20?n+1:n==20?30:40)+" kHz";}if(i==7){const char* a[]={"5 kHz","10 kHz","20 kHz"};return juce::String(a[juce::jlimit(0,2,n)]);}return juce::String(v,1)+(i==8?" dB":"");};
 knobAttachments[i]=std::make_unique<SA>(processor.apvts,params[i],s);}
 addAndMakeVisible(switches[0]);switches[0].setName("Power");switches[0].getProperties().set("reverse",true);switchAttachments[0]=std::make_unique<BA>(processor.apvts,"power",switches[0]);addAndMakeVisible(switches[1]);switches[1].setName("FIX");switches[1].setTooltip("FIX: match output RMS to input level");switches[1].getProperties().set("reverse",true);switchAttachments[1]=std::make_unique<BA>(processor.apvts,"fix",switches[1]);startTimerHz(15);}
FaceplateComponent::~FaceplateComponent(){stopTimer();setLookAndFeel(nullptr);}
void FaceplateComponent::timerCallback(){repaint();}
void FaceplateComponent::resized(){for(size_t i=0;i<knobs.size();++i)knobs[i].setBounds(xs[i]-34,ys[i]-34,68,68);switches[0].setBounds(204,190,50,36);switches[1].setBounds(1169,230,50,36);}
void FaceplateComponent::paint(juce::Graphics& g){
    g.fillAll(colour(0xff08090b));
    const juce::Rectangle<float> plate(5,4,1322,330);
    juce::Path outline;outline.addRoundedRectangle(plate,6);
    {
        juce::Graphics::ScopedSaveState save(g);g.reduceClipRegion(outline);
        auto aluminium=juce::ColourGradient(colour(0xffe4e6e9),0,4,colour(0xff8a9095),0,334,false);
        aluminium.addColour(0.08,colour(0xffe4e6e9));aluminium.addColour(0.35,colour(0xffdfe3e6));
        aluminium.addColour(0.64,colour(0xffd9dee1));aluminium.addColour(0.92,colour(0xffd1d6da));
        aluminium.addColour(0.96,colour(0xffa0a5aa));
        g.setGradientFill(aluminium);g.fillRect(plate);
        for(int y=5;y<334;++y)
        {
            const float alpha=y%4==0?0.28f:y%4==1?0.10f:y%4==2?0.025f:0.045f;
            g.setColour((y%4<2?juce::Colours::white:juce::Colours::black).withAlpha(alpha));
            g.drawHorizontalLine(y,5,1327);
        }
        // Subtle broad machining highlights retain the horizontal grain at any scale.
        for(int i=0;i<12;++i){g.setColour(juce::Colours::white.withAlpha(0.012f));g.fillRect(5.0f,9.0f+static_cast<float>(i)*19,1322.0f,7.0f);}
    }
    g.setColour(juce::Colours::white.withAlpha(0.6f));g.drawRoundedRectangle(plate.reduced(0.5f),5,1);
    g.setColour(juce::Colours::black.withAlpha(0.25f));g.drawRoundedRectangle(plate.reduced(2),4,1);
    for(float x:{59.0f,1275.0f}){g.setColour(juce::Colours::black.withAlpha(0.25f));g.drawVerticalLine(static_cast<int>(x),6,332);g.setColour(juce::Colours::white.withAlpha(0.24f));g.drawVerticalLine(static_cast<int>(x+1),6,332);}
    for(float x:{16.0f,1285.0f})for(float y:{28.0f,287.0f})
    {
        g.setColour(juce::Colours::white.withAlpha(0.8f));g.fillRoundedRectangle(x-1,y-1,33,24,12);
        g.setColour(colour(0xff474d50));g.fillRoundedRectangle(x,y,31,22,11);
        g.setColour(colour(0xff020506));g.fillRoundedRectangle(x+1,y+1,29,20,10);
    }
    for(auto point:{juce::Point<float>(165,53),{165,286},{1117,53},{1117,286}})screw(g,point.x,point.y);

 g.setColour(colour(0xff15191b));text(g,"EQP-1A",66,150,100,25,19);text(g,"PROGRAM EQ",66,181,100,15,10);
 lamp(g,{206,70,60,60},processor.apvts.getRawParameterValue("power")->load()>.5f);g.setColour(colour(0xff15191b));text(g,"POWER",194,145,80,22,14);text(g,"ON",174,200,35,15,11);text(g,"OFF",258,200,35,15,11);
 const char* titles[]={"BOOST","ATTEN","LOW FREQ / Hz","BANDWIDTH","BOOST","HIGH FREQ / kHz","ATTEN","ATTEN SEL / kHz","OUTPUT / dB"};
 for(int i=0;i<9;++i){float cx=float(xs[i]),cy=float(ys[i]);int steps=i==2?8:i==5?21:i==7?2:i==8?4:5;
 for(int t=0;t<=steps;++t){float a=juce::MathConstants<float>::pi*(7.f/6.f+5.f/3.f*float(t)/float(steps));float dx=std::sin(a),dy=-std::cos(a);g.setColour(colour(0xff22292b));g.fillEllipse(cx+dx*43-1.5f,cy+dy*43-1.5f,3,3);juce::String s;
 if(i==2){if(t%2==0)s=juce::String(20+10*t);}else if(i==5){if(t==0||t==4||t==9||t==14||t==21)s=juce::String(t<20?t+1:40);}else if(i==7){const char* z[]={"5","10","20"};s=z[t];}else if(i==8){int gain=-12+t*6;s=(gain>0?"+":"")+juce::String(gain);}else s=juce::String(t*2);if(i==3&&(t==0||t==steps))s=t==0?"SHARP":"BROAD";
 text(g,s,cx+dx*57-22,cy+dy*57-7,44,14,(i==3&&(t==0||t==steps))?8.0f:10.0f);}
 text(g,titles[i],cx-65,ys[i]<150?18.0f:cy+67.0f,130,18,11);
 }
 g.setColour(colour(0xff353b3e));

 for(int i : {2,5,7,8}) { auto value=knobs[size_t(i)].getTextFromValue(knobs[size_t(i)].getValue());text(g,value,float(xs[i]-35),float(ys[i]+46),70,15,10); }
 g.setColour(colour(0xff15191b));text(g,"ON",1138,240,30,15,10);text(g,"OFF",1218,240,35,15,10);text(g,"FIX",1152,276,85,18,12);
    // Hand-drawn vector script, independent of installed fonts.
    juce::Path signature;
    signature.startNewSubPath(0,22);signature.lineTo(12,2);signature.cubicTo(25,-3,25,7,13,12);signature.lineTo(7,12);
    signature.startNewSubPath(14,18);signature.cubicTo(21,6,29,9,22,18);signature.cubicTo(18,24,13,22,18,16);signature.lineTo(28,12);signature.lineTo(24,20);signature.lineTo(34,14);
    signature.startNewSubPath(29,29);signature.lineTo(39,10);signature.cubicTo(49,5,48,15,40,19);signature.lineTo(36,18);signature.lineTo(49,13);
    signature.startNewSubPath(51,10);signature.lineTo(46,20);signature.quadraticTo(49,23,57,14);
    signature.startNewSubPath(54,6);signature.lineTo(55,4);
    signature.startNewSubPath(54,17);signature.cubicTo(66,16,67,9,61,11);signature.cubicTo(50,17,56,26,68,17);
    signature.cubicTo(82,1,80,-3,73,8);signature.lineTo(68,19);signature.quadraticTo(68,23,78,16);
    signature.cubicTo(86,6,93,10,86,18);signature.cubicTo(79,26,77,16,84,13);signature.lineTo(91,11);signature.lineTo(81,29);signature.cubicTo(75,37,70,29,88,22);
    signature.lineTo(95,13);signature.lineTo(91,20);signature.quadraticTo(92,24,101,14);signature.lineTo(98,20);signature.quadraticTo(101,22,108,14);
    signature.cubicTo(116,7,122,10,115,18);signature.cubicTo(108,26,106,17,112,13);signature.lineTo(120,11);signature.lineTo(116,20);signature.lineTo(127,13);
    signature.startNewSubPath(121,29);signature.lineTo(132,10);signature.cubicTo(142,5,142,15,134,19);signature.lineTo(129,18);signature.lineTo(143,13);
    signature.cubicTo(153,6,156,12,148,19);signature.cubicTo(140,25,137,17,146,12);signature.quadraticTo(149,17,155,13);
    const auto transform=juce::AffineTransform::scale(0.80f,0.80f).translated(1136,157);
    g.setColour(juce::Colours::white.withAlpha(0.28f));g.strokePath(signature,juce::PathStrokeType(3.0f,juce::PathStrokeType::curved,juce::PathStrokeType::rounded),transform.followedBy(juce::AffineTransform::translation(1,1)));
    g.setColour(colour(0xff111719));g.strokePath(signature,juce::PathStrokeType(3.0f,juce::PathStrokeType::curved,juce::PathStrokeType::rounded),transform);
    juce::Path flourish;flourish.startNewSubPath(1147,188);flourish.lineTo(1272,180);flourish.lineTo(1222,191);flourish.lineTo(1236,183);
    g.strokePath(flourish,juce::PathStrokeType(1.6f));
}
EQP1AEditor::EQP1AEditor(EQP1AProcessor& p):AudioProcessorEditor(p),processor(p),faceplate(p){addAndMakeVisible(faceplate);setResizable(true,true);setResizeLimits(934,238,2668,680);getConstrainer()->setFixedAspectRatio(double(kBaseWidth)/kBaseHeight);int w=934;setSize(w,juce::roundToInt(double(w)*kBaseHeight/kBaseWidth));}
void EQP1AEditor::resized(){faceplate.setBounds(0,0,kBaseWidth,kBaseHeight);faceplate.setTransform(juce::AffineTransform::scale(float(getWidth())/kBaseWidth));processor.editorWidth=getWidth();}
