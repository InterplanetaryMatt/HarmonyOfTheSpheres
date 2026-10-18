#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cmath>
namespace {
constexpr double pi=3.14159265358979323846;
constexpr double periodYears[8]={0.2408467,0.61519726,1.0,1.8808158,11.862615,29.447498,84.016846,164.79132};
constexpr double rotationDays[8]={58.646,-243.025,0.99726968,1.02595675,0.41354,0.44401,-0.71833,0.67125}; // sidereal days, negative = retrograde
constexpr double eccentricity[8]={0.205630,0.006772,0.0167086,0.0934,0.0489,0.0565,0.0464,0.009456};
const char* names[8]={"Mercury","Venus","Earth","Mars","Jupiter","Saturn","Uranus","Neptune"};
// Kepler's equation E-e sin(E)=M, solved with Newton iterations.
double trueAnomaly(double orbitTurns, double e)
{
 const auto M=2*pi*(orbitTurns-std::floor(orbitTurns));
 double E=M;
 for(int i=0;i<8;++i) E -= (E-e*std::sin(E)-M)/(1-e*std::cos(E));
 return 2*std::atan2(std::sqrt(1+e)*std::sin(E/2),std::sqrt(1-e)*std::cos(E/2));
}
float param(const juce::AudioProcessorValueTreeState& apvts, const char* id) { return apvts.getRawParameterValue(id)->load(); }
}
juce::AudioProcessorValueTreeState::ParameterLayout PlanetProcessor::makeParameters()
{
 using F=juce::AudioParameterFloat; using B=juce::AudioParameterBool; using C=juce::AudioParameterChoice;
 std::vector<std::unique_ptr<juce::RangedAudioParameter>> p;
 p.push_back(std::make_unique<C>(juce::ParameterID{"mode",1},"Tuning mode",juce::StringArray{"True ratios","Octave-folded","Compressed","Orbital modulation"},1));
 p.push_back(std::make_unique<F>(juce::ParameterID{"earth",1},"Earth frequency (Hz)",juce::NormalisableRange<float>(55.f,880.f,1.f,0.5f),220.f));
 p.push_back(std::make_unique<F>(juce::ParameterID{"year",1},"One Earth year (seconds)",juce::NormalisableRange<float>(2.f,180.f,0.1f,0.5f),20.f));
 p.push_back(std::make_unique<F>(juce::ParameterID{"depth",1},"Orbital modulation depth",0.f,1.f,0.6f));
 p.push_back(std::make_unique<F>(juce::ParameterID{"ellipse",1},"Eccentricity exaggeration",1.f,3.f,1.f));
 p.push_back(std::make_unique<F>(juce::ParameterID{"gain",1},"Master gain",0.f,0.3f,0.09f));
 p.push_back(std::make_unique<C>(juce::ParameterID{"wave",1},"Waveform",juce::StringArray{"Sine","Triangle","Soft saw"},0));
 p.push_back(std::make_unique<B>(juce::ParameterID{"audio",1},"Internal sound on",true));
 p.push_back(std::make_unique<B>(juce::ParameterID{"midi",1},"MIDI notes out",true));
 p.push_back(std::make_unique<B>(juce::ParameterID{"gate",1},"Four pulses per orbit",true));
 p.push_back(std::make_unique<F>(juce::ParameterID{"duty",1},"Pulse length (% of beat)",juce::NormalisableRange<float>(5.f,95.f,1.f),45.f));
 p.push_back(std::make_unique<B>(juce::ParameterID{"spincc",1},"Rotation MIDI CC",true));
 p.push_back(std::make_unique<C>(juce::ParameterID{"spinccnum",1},"Rotation CC number",juce::StringArray{"CC 1 Mod wheel","CC 11 Expression","CC 74 Brightness","CC 71 Resonance"},2));
 p.push_back(std::make_unique<B>(juce::ParameterID{"bend",1},"Microtonal pitch bend",false));
 p.push_back(std::make_unique<B>(juce::ParameterID{"orbitcc11",1},"Orbital distance to CC11",true));
 p.push_back(std::make_unique<B>(juce::ParameterID{"orbitcc10",1},"Orbital position to CC10",true));
 for(int i=0;i<8;++i)
 {
  auto id=juce::String(i);
  p.push_back(std::make_unique<B>(juce::ParameterID{"on"+id,1},juce::String(names[i])+" on",true));
  p.push_back(std::make_unique<F>(juce::ParameterID{"vol"+id,1},juce::String(names[i])+" level",0.f,1.f,0.65f));
  p.push_back(std::make_unique<C>(juce::ParameterID{"ch"+id,1},juce::String(names[i])+" MIDI channel",juce::StringArray{"1","2","3","4","5","6","7","8","9","10","11","12","13","14","15","16"},i));
 }
 return {p.begin(),p.end()};
}
PlanetProcessor::PlanetProcessor(): AudioProcessor(BusesProperties().withOutput("Output",juce::AudioChannelSet::stereo(),true)),parameters(*this,nullptr,"PARAMETERS",makeParameters()) {activeNote.fill(-1); activeChannel.fill(1);}
void PlanetProcessor::prepareToPlay(double sr,int) {sampleRateHz=sr;phase.fill(0);smoothLevel.fill(0); activeNote.fill(-1);activeChannel.fill(1);heldNotes.fill(false);noteOrder.fill(0);lastExpression.fill(-1);lastPan.fill(-1);nextNoteOrder=0;referenceNote=-1;displayedNote.store(-1);ccElapsed=0;elapsedSeconds=0;orbitTime.store(0);wasSending=false;}
void PlanetProcessor::processBlock(juce::AudioBuffer<float>& buffer,juce::MidiBuffer& midi)
{
 juce::ScopedNoDenormals noDenormals;
 // Treat incoming MIDI as the performance gate and the Earth pitch reference.
 // Last-note priority: releasing the latest note falls back to the previous held key.
 for(const auto metadata : midi)
 {
  const auto msg=metadata.getMessage();
  if(msg.isNoteOn())
  {
   const int note=msg.getNoteNumber();
   heldNotes[(size_t)note]=true;
   noteOrder[(size_t)note]=++nextNoteOrder;
  }
  else if(msg.isNoteOff()) heldNotes[(size_t)msg.getNoteNumber()]=false;
  else if(msg.isAllNotesOff() || msg.isAllSoundOff()) heldNotes.fill(false);
 }
 referenceNote=-1;
 unsigned long long newest=0;
 for(int note=0;note<128;++note)
  if(heldNotes[(size_t)note] && noteOrder[(size_t)note]>newest)
   {newest=noteOrder[(size_t)note];referenceNote=note;}
 displayedNote.store(referenceNote,std::memory_order_relaxed);
 midi.clear(); // output MIDI only, rather than passing the played note through to Roland
 buffer.clear();
 const int n=buffer.getNumSamples();
 if(n<=0 || sampleRateHz<=0) return;
 const int mode=(int)param(parameters,"mode");
 const double earth= referenceNote>=0 ? 440.0*std::pow(2.0,(referenceNote-69)/12.0) : 440.0, year=param(parameters,"year");
 const double depth=param(parameters,"depth"), exaggeration=param(parameters,"ellipse");
 const double gain=param(parameters,"gain");
 const int wave=(int)param(parameters,"wave");
 const bool audio=param(parameters,"audio")>0.5f, sendMidi=param(parameters,"midi")>0.5f;
 const bool bending=param(parameters,"bend")>0.5f;
 const bool orbitCC11=param(parameters,"orbitcc11")>0.5f;
 const bool orbitCC10=param(parameters,"orbitcc10")>0.5f;
 const bool rhythm=param(parameters,"gate")>0.5f;
 const double duty=param(parameters,"duty")/100.0;
 const bool spinCC=param(parameters,"spincc")>0.5f;
 const int ccOptions[]={1,11,74,71}; const int spinNumber=ccOptions[juce::jlimit(0,3,(int)param(parameters,"spinccnum"))];
 ccElapsed+=(double)n/sampleRateHz;
 const bool sendCCThisBlock=ccElapsed>=0.005;
 if(sendCCThisBlock) ccElapsed=0;
 std::array<double,8> frequency{},amplitude{},pan{};
 for(int i=0;i<8;++i)
 {
  const auto id=juce::String(i);
  const bool enabled=parameters.getRawParameterValue("on"+id)->load()>0.5f;
  const double level=parameters.getRawParameterValue("vol"+id)->load();
  const double ratio=1.0/periodYears[i];
  double f=earth*ratio;
  if(mode==1 || mode==3) { while(f<90)f*=2; while(f>1200)f*=0.5; }
  if(mode==2) f=earth*std::pow(ratio,0.4);
  const double e=juce::jmin(0.8,eccentricity[i]*exaggeration);
  const double a=trueAnomaly(elapsedSeconds/(year*periodYears[i]),e);
  const double radius=(1-e*e)/(1+e*std::cos(a));
  // Orbital dynamics mapped to volume and stereo width. The mapping is a creative choice.
  const double closeness=juce::jlimit(0.0,1.0,(1.0+e-radius)/(2*juce::jmax(0.001,e)));
  const double mod=(mode==3)?(1-depth*0.65+depth*0.65*closeness):1.0;
  // Mean anomaly advances uniformly in time; four equal-time beats per orbital revolution.
  const double orbitTurns=elapsedSeconds/(year*periodYears[i]);
  const double beatFrac=4.0*orbitTurns-std::floor(4.0*orbitTurns);
  const bool pulseOn=!rhythm || beatFrac<duty;
  frequency[i]=f;
  amplitude[i]=(enabled && referenceNote>=0 && pulseOn?level:0)*mod;
  pan[i]=(mode==3)?depth*0.8*std::sin(a):0;
  // Independent channel mode sends each planet on channels 1-8, with +/-2 semitone pitch bend.
  const int channel=1+(int)parameters.getRawParameterValue("ch"+id)->load();
  const int note=juce::jlimit(0,127,(int)std::lround(69+12*std::log2(f/440.0)));
  const bool wants=sendMidi && enabled && referenceNote>=0 && pulseOn;
  if(activeNote[i]>=0 && (!wants || activeNote[i]!=note || activeChannel[i]!=channel || wasSending!=sendMidi))
  { midi.addEvent(juce::MidiMessage::noteOff(activeChannel[i],activeNote[i]),0); activeNote[i]=-1; }
  // Independent orbital controllers are not tied to tuning mode, pitch bend,
  // or the four-note rhythmic gate. Keep them flowing between note pulses.
  // CC11: proximity to Sun (0 farthest, 127 nearest) with depth control.
  // CC10: position around the orbit mapped to pan (left to right).
  if(sendMidi && enabled && referenceNote>=0 && sendCCThisBlock)
  {
   if(orbitCC11)
   {
    const double expressionLevel=1.0-depth*0.65+depth*0.65*closeness;
    const int expression=juce::jlimit(0,127,(int)std::lround(127*expressionLevel));
    if(expression!=lastExpression[i])
    {midi.addEvent(juce::MidiMessage::controllerEvent(channel,11,expression),0);lastExpression[i]=expression;}
   }
   else lastExpression[i]=-1;
   if(orbitCC10)
   {
    const int midiPan=juce::jlimit(0,127,(int)std::lround(64+depth*0.8*std::sin(a)*63));
    if(midiPan!=lastPan[i])
    {midi.addEvent(juce::MidiMessage::controllerEvent(channel,10,midiPan),0);lastPan[i]=midiPan;}
   }
   else lastPan[i]=-1;
  }
  else {lastExpression[i]=-1;lastPan[i]=-1;}
  // Sidereal spin mapped to a sine-wave CC. CC is channel-wide in MIDI 1.0.
  // 200 Hz update attempts; fast rotation can still be aliased when the simulated year is very short.
  if(sendMidi && enabled && referenceNote>=0 && spinCC && sendCCThisBlock)
  {
   const double spinTurns=(elapsedSeconds/year)*(365.25636/rotationDays[i]);
   const int spinValue=juce::jlimit(0,127,(int)std::lround(63.5+63.5*std::sin(2*pi*spinTurns)));
   midi.addEvent(juce::MidiMessage::controllerEvent(channel,spinNumber,spinValue),0);
  }
  if(wants && activeNote[i]<0)
  {
   if(bending)
   {
    const double semis=69+12*std::log2(f/440.0);
    const int pb=juce::jlimit(0,16383,(int)std::lround(8192+(semis-note)*(8192.0/2.0)));
    midi.addEvent(juce::MidiMessage::pitchWheel(channel,pb),0);
   }
   midi.addEvent(juce::MidiMessage::noteOn(channel,note,(juce::uint8)96),0);
   activeNote[i]=note; activeChannel[i]=channel;
  }
 }
 wasSending=sendMidi;
 if(audio)
 for(int s=0;s<n;++s)
 {
  float left=0,right=0;
  for(int i=0;i<8;++i)
  {
   smoothLevel[i]+=0.001*(amplitude[i]-smoothLevel[i]);
   phase[i]+=frequency[i]/sampleRateHz;
   phase[i]-=std::floor(phase[i]);
   const double t=phase[i];
   const double osc=wave==0?std::sin(2*pi*t):(wave==1?4*std::abs(t-0.5)-1:0.7*(2*t-1));
   const double v=osc*smoothLevel[i]*gain;
   left+=(float)(v*std::sqrt((1-pan[i])*0.5));
   right+=(float)(v*std::sqrt((1+pan[i])*0.5));
  }
  if(buffer.getNumChannels()>0) buffer.setSample(0,s,juce::jlimit(-1.f,1.f,left));
  if(buffer.getNumChannels()>1) buffer.setSample(1,s,juce::jlimit(-1.f,1.f,right));
 }
 elapsedSeconds+=(double)n/sampleRateHz; orbitTime.store(elapsedSeconds,std::memory_order_relaxed);
}
juce::AudioProcessorEditor* PlanetProcessor::createEditor(){return new PlanetEditor(*this);}
void PlanetProcessor::getStateInformation(juce::MemoryBlock& b)
{
 if(auto xml=parameters.copyState().createXml()) copyXmlToBinary(*xml,b);
}
void PlanetProcessor::setStateInformation(const void* data,int size)
{
 if(auto xml=getXmlFromBinary(data,size)) if(xml->hasTagName(parameters.state.getType())) parameters.replaceState(juce::ValueTree::fromXml(*xml));
}
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter(){return new PlanetProcessor();}
