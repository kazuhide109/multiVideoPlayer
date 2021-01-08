#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
     ofBackground(0,0,0);
     ofSetVerticalSync(true);
     ofSetWindowTitle("MultiVideoPlayer");
     ofSetFrameRate(60);
     
     ofSetLogLevel(OF_LOG_VERBOSE);
     
     //サウンドセットアップ
     ofSoundStreamSettings streamSettings;
     streamSettings.numInputChannels = 0;
     streamSettings.numOutputChannels = 2;
     streamSettings.sampleRate = 48000; //make sure you pass the correct sample rate.
     streamSettings.bufferSize = 256;
     streamSettings.numBuffers = 1;
     stream.setup(streamSettings);
     
     //ファイルパスの指定
     string path = "moviesHapNoaudio";
     string soundsPath = "soundsFiles";
     bool bLoadAsync = false;
     ofDirectory dir(path);
     dir.listDir();
     size = dir.size();
     sounds.resize(size);
     vector<string> sfName;
     for(int i = 0; i < size; i++){
          //ビデオの読み込み
          string fName = dir.getName(i);
          hapPlayer[i].load(path + "/" + fName);
          cout << "H: " << path + "/" + fName << endl;;
          hapPlayer[i].setPosition(1);
          hapPlayer[i].play();
          hapPlayer[i].setLoopState(OF_LOOP_NONE);
          hapPlayer[i].setVolume(0.0);
          for(int k=0; k<4; k++){
               fName.pop_back();
          }
          sfName.push_back(fName);
     }
     ofDirectory dirS(soundsPath);
     dirS.listDir();
     for(int i = 0; i < size; i++){
           //サウンドの読み込み
           sounds[i] = make_unique<ofxSoundPlayerObject>();
           sounds[i]->setLoop(true);
           if(bLoadAsync){
                sounds[i]->loadAsync(ofToDataPath(soundsPath + "/" + sfName[i] + ".wav"), true);
                cout << "NO" << endl;
           }else{
                sounds[i]->load(ofToDataPath(soundsPath + "/" + sfName[i] + ".wav"));
                cout << "K: " << soundsPath + "/" + sfName[i] + ".wav" << endl;;
                sounds[i]->play();
                sounds[i]->connectTo(mixer);
           }
           playersVolume.push_back(1);
           volumeGroup.add(playersVolume.back().set("Player " + ofToString(i), 0.0, 0, 1));
     }
     
     mixer.setMasterVolume(5.0);
     stream.setOutput(mixer);
     
     //Gui
     volumeGroup.setName("VOLUME");
     gui.setup(volumeGroup);
     gui.setPosition(20, 80);
     ofAddListener(volumeGroup.parameterChangedE(), this, &ofApp::updateVolume);
     playersVolume[0].set(0.5);
     
     //変数の初期化
     select = 0;
     isDebugDraw = true;
     
}

//--------------------------------------------------------------
void ofApp::update(){
     for(int i=0; i<size; i++){
          //          hapPlayer[i].update();
          if(i <= select){
               if(int(hapPlayer[i].getPosition()) == 1){
                    cout << "DONE " << i << endl;
                    hapPlayer[i].play();
                    sounds[i]->play();
               }
          }
     }
     
}

//--------------------------------------------------------------
void ofApp::draw(){
     //グリッド上の動画配置関数
     struct {
          ofRectangle operator()(int i, int select)
          {
               ofRectangle box;
               int gridNum = 0;
               for(int k=0; k<sqrt(NUMBEROFIMAGES); k++){
                    gridNum = k;
                    if(k*k > select) break;
               }
               int j =0;
               int n =1;
               if(i>gridNum*1 -1) j= 1;
               if(i>gridNum*2 -1) j= 2;
               if(i>gridNum*3 -1) j= 3;
               int nj =j;
               if(select < gridNum*(gridNum-1)){
                    n = 2;
                    nj = 1+j*2;
               }
               box.set(ofGetWidth()/ gridNum*(i% gridNum), ofGetHeight()/ gridNum/n*(nj), ofGetWidth()/ gridNum, ofGetHeight()/ gridNum);
               return box;
          };
     } drawRect;
     
     //動画の描画
     for(int i=0; i<select+1; i++){
          if (hapPlayer[i].isLoaded())
          {
               // Draw the video frame
               ofSetColor(255, 255, 255);
               ofRectangle drawBox = drawRect(i,select);
               hapPlayer[i].draw(drawBox.x, drawBox.y, drawBox.width, drawBox.height);
               
          }else{
               if (hapPlayer[i].getError().length()){
                    ofDrawBitmapStringHighlight(hapPlayer[i].getError(), 20, 20);
               }
               else{
                    ofDrawBitmapStringHighlight("Movie is loading...", 20, 20);
               }
          }
     }
     
     //デバック
     if(isDebugDraw){
          ofShowCursor();
          stringstream mess;
          mess << "FPS: " << ofGetFrameRate() << "\n";
          mess << "Number of Images: " << select << "\n";
          mess << "Clip Volume: " << sounds[select]->getVolume() << "\n";
          mess << "Position: " << hapPlayer[select].getPosition() << "\n";
          ofDrawBitmapStringHighlight((mess.str()), 20, 20);
          
          gui.draw();
          
          if(sounds.size()){
               sounds[select]->drawDebug(gui.getShape().getMaxX(), gui.getShape().y);
          }
     }else{
          ofHideCursor();
     }
     
}

void ofApp::exit(){
     for(int i=0; i<size; i++){
          hapPlayer[i].stop();
          hapPlayer[i].close();
     }
     stream.close();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
     switch(key){
          case OF_KEY_RIGHT:
               select++;
               if(select >= size) select = 0;
               break;
          case OF_KEY_LEFT:
               select--;
               if(select <0) select = size-1;
               break;
          case ' ':
               for(int i=0; i<size; i++){
                    if(!hapPlayer[i].isPaused()){
                         hapPlayer[i].setPaused(true);
                         sounds[i]->setPaused(true);
                    }
                    else{
                         hapPlayer[i].setPaused(false);
                         sounds[i]->setPaused(false);
                    }
               }
               break;
               
          case OF_KEY_COMMAND:
               isDebugDraw = !isDebugDraw;
               break;
     }
     
}

//--------------------------------------------------------------
void ofApp::updateVolume(ofAbstractParameter& p){
     for (int i = 0; i < playersVolume.size(); i++) {
          mixer.setConnectionVolume(i, playersVolume[i]);
     }
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){
     
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){
     
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){
     
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
     
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){
     
}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){
     
}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){
     
}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){
     
}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){
     
}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 
     
}
