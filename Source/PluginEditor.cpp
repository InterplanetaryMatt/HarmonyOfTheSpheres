#include "PluginEditor.h"
#include <cmath>

namespace {
constexpr double pi = 3.14159265358979323846;
constexpr double years[] = {.2408467,.61519726,1.,1.8808158,11.862615,29.447498,84.016846,164.79132};
constexpr double ecc[] = {.205630,.006772,.0167086,.0934,.0489,.0565,.0464,.009456};
constexpr const char* names[] = {"Mercury","Venus","Earth","Mars","Jupiter","Saturn","Uranus","Neptune"};
const juce::Colour palette[] = {juce::Colour(0xffd7b58e),juce::Colour(0xffebc889),juce::Colour(0xff61b7ff),juce::Colour(0xfff38b63),juce::Colour(0xffd6aa81),juce::Colour(0xffdfc79c),juce::Colour(0xff9ad6dd),juce::Colour(0xff578fe8)};
double raw(const PlanetProcessor& p, const juce::String& id) { return p.parameters.getRawParameterValue(id)->load(); }
double anomaly(double turns, double e) {
 const double M=2*pi*(turns-std::floor(turns)); double E=M;
 for(int j=0;j<9;++j) E-=(E-e*std::sin(E)-M)/(1-e*std::cos(E));
 return E;
}
}
void OrbitDisplay::paint(juce::Graphics& g)
{
 auto bounds=getLocalBounds().toFloat().reduced(12);
 g.setColour(juce::Colour(0xff091427));g.fillRoundedRectangle(bounds,18);
 const auto c=bounds.getCentre();
 const float maxR=0.46f*juce::jmin(bounds.getWidth(),bounds.getHeight());
 g.setColour(juce::Colour(0xffe8b05b).withAlpha(.15f));g.fillEllipse(c.x-17,c.y-17,34,34);
 g.setColour(juce::Colour(0xffffce72));g.fillEllipse(c.x-8,c.y-8,16,16);
 const double time=processor.getOrbitTime(), year=juce::jmax(0.1,raw(processor,"year"));
 const double exaggeration=raw(processor,"ellipse");
 for(int i=7;i>=0;--i) {
  const float a=maxR*(.18f+.105f*(i+1));
  const double e=juce::jlimit(0.,.8,ecc[i]*exaggeration);
  const float b=a*(float)std::sqrt(1-e*e);
  // Sun is located at the focus, not the geometric centre.
  const float cx=c.x+(float)(a*e);
  auto orbit=juce::Rectangle<float>(cx-a,c.y-b,2*a,2*b);
  g.setColour(palette[i].withAlpha(.28f));g.drawEllipse(orbit,1.f);
  const double E=anomaly(time/(year*years[i]),e);
  const float x=cx-a*(float)std::cos(E), y=c.y-b*(float)std::sin(E);
  const bool isOn=raw(processor,"on"+juce::String(i))>.5;
  const float r=i<4?4.f:6.f;
  if(isOn) {g.setColour(palette[i].withAlpha(.18f));g.fillEllipse(x-r*2.4f,y-r*2.4f,4.8f*r,4.8f*r);}
  g.setColour(isOn?palette[i]:juce::Colours::grey);g.fillEllipse(x-r,y-r,r*2,r*2);
 }
 g.setColour(juce::Colours::white.withAlpha(.65f));g.setFont(13.f);
 g.drawFittedText("Keplerian elliptical orbits  •  visual sizes not to scale",bounds.toNearestInt().removeFromBottom(24),juce::Justification::centred,1);
}
PlanetEditor::PlanetEditor(PlanetProcessor& p):AudioProcessorEditor(p),processor(p),orbits(p)
{
 setOpaque(true); setSize(1060,750);
 addAndMakeVisible(orbits);
 title.setText("HARMONY OF THE SPHERES",juce::dontSendNotification);title.setFont(juce::Font(23.f,juce::Font::bold));title.setColour(juce::Label::textColourId,juce::Colour(0xffe7f1ff));addAndMakeVisible(title);
 subtitle.setText("ORBITAL SONIFICATION • 8 INDEPENDENT MIDI CHANNELS",juce::dontSendNotification);subtitle.setColour(juce::Label::textColourId,juce::Colour(0xff90a8c4));addAndMakeVisible(subtitle);
 auto& st=p.parameters;
 auto makeChoice=[&](juce::ComboBox& box,const juce::String& id,int count) { for(int i=0;i<count;++i) box.addItem(id.startsWith("ch")?juce::String(i+1):id=="mode"?(juce::StringArray{"True ratios","Octave folded","Compressed","Orbital modulation"})[i]:(juce::StringArray{"Sine","Triangle","Soft saw"})[i],i+1);addAndMakeVisible(box);choices.push_back(std::make_unique<ChoiceAttachment>(st,id,box));};
 makeChoice(mode,"mode",4); makeChoice(wave,"wave",3);
 auto makeSlider=[&](juce::Slider& s,const juce::String& id){s.setSliderStyle(juce::Slider::LinearHorizontal);s.setTextBoxStyle(juce::Slider::TextBoxRight,false,63,22);addAndMakeVisible(s);sliders.push_back(std::make_unique<SliderAttachment>(st,id,s));};
 makeSlider(earth,"earth");makeSlider(year,"year");makeSlider(depth,"depth");makeSlider(ellipse,"ellipse");makeSlider(gain,"gain");
 for(auto* pair : {&audio,&midi,&bend}) {addAndMakeVisible(*pair);}
 audio.setButtonText("Internal audio");midi.setButtonText("MIDI output");bend.setButtonText("Pitch bend + orbit CC");
 buttons.push_back(std::make_unique<ButtonAttachment>(st,"audio",audio));buttons.push_back(std::make_unique<ButtonAttachment>(st,"midi",midi));buttons.push_back(std::make_unique<ButtonAttachment>(st,"bend",bend));
 const juce::StringArray labels{"Earth pitch (Hz)","Year length (s)","Orbit depth","Ellipse exaggeration","Master level"};
 for(int i=0;i<5;++i){controlLabels[i].setText(labels[i],juce::dontSendNotification); controlLabels[i].setColour(juce::Label::textColourId,juce::Colours::lightgrey);addAndMakeVisible(controlLabels[i]);}
 for(int i=0;i<8;++i){
  auto id=juce::String(i);planetName[i].setText(names[i],juce::dontSendNotification);planetName[i].setColour(juce::Label::textColourId,palette[i]);addAndMakeVisible(planetName[i]);
  enabled[i].setButtonText("On");addAndMakeVisible(enabled[i]);buttons.push_back(std::make_unique<ButtonAttachment>(st,"on"+id,enabled[i]));
  makeChoice(channel[i],"ch"+id,16);makeSlider(level[i],"vol"+id);
 }
}
void PlanetEditor::paint(juce::Graphics& g)
{
 g.fillAll(juce::Colour(0xff070e19));
 g.setColour(juce::Colour(0xffa8c1df));g.setFont(14.f);
 g.drawText("PLANET",580,118,115,22,juce::Justification::centredLeft);
 g.drawText("ENABLE",701,118,75,22,juce::Justification::centredLeft);
 g.drawText("MIDI CHANNEL",786,118,110,22,juce::Justification::centredLeft);
 g.drawText("LEVEL",915,118,110,22,juce::Justification::centredLeft);
 g.setColour(juce::Colour(0xff2b405c));g.drawHorizontalLine(144,578,1033);
}
void PlanetEditor::resized()
{
 title.setBounds(22,14,480,34);subtitle.setBounds(24,52,530,23);
 orbits.setBounds(8,98,560,560);
 mode.setBounds(20,667,235,30);wave.setBounds(266,667,128,30);
 audio.setBounds(20,708,135,24);midi.setBounds(160,708,130,24);bend.setBounds(295,708,225,24);
 for(int i=0;i<8;++i){const int y=151+i*48;planetName[i].setBounds(584,y,110,30);enabled[i].setBounds(704,y,70,30);channel[i].setBounds(788,y,100,30);level[i].setBounds(900,y,132,30);}
 const std::array<juce::Slider*,5> ss{&earth,&year,&depth,&ellipse,&gain};
 for(int i=0;i<5;++i){int y=551+i*36;controlLabels[i].setBounds(583,y,162,30);ss[i]->setBounds(750,y,275,30);}
}
