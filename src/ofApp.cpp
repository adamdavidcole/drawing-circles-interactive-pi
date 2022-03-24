#include <iostream>
#include "ofApp.h"

using namespace std;

void log(string s) {
    cout << s << endl;
}

void logFloatArr(float arr[], int arrSize) {
    cout << "[ ";
    for (int i = 0; i < arrSize; i++) {
        cout << to_string(arr[i]);
        
        if (i != arrSize - 1) cout << ", ";
    }
    cout << " ]" << endl;
}

//--------------------------------------------------------------
void ofApp::setup(){
    int width = 1024;
    int height = 768;
    sampleRate = 44100;
    channels = 2;

    ofSetFrameRate(60);
    ofSetLogLevel(OF_LOG_VERBOSE);
//    vidGrabber.setDesiredFrameRate(30);
//    vidGrabber.initGrabber(640, 480);
    vidRecorder.setFfmpegLocation("/opt/homebrew/Cellar/ffmpeg/4.4.1_5/bin/ffmpeg"); // use this is you have ffmpeg installed in your data folder

    fileName = "testMovie";
    fileExt = ".mov"; // ffmpeg uses the extension to determine the container type. run 'ffmpeg -formats' to see supported formats

    // override the default codecs if you like
    // run 'ffmpeg -codecs' to find out what your implementation supports (or -formats on some older versions)
    vidRecorder.setVideoCodec("libx264");
    vidRecorder.setVideoBitrate("100000k");
    vidRecorder.setAudioCodec("mp3");
    vidRecorder.setAudioBitrate("192k");

    ofAddListener(vidRecorder.outputFileCompleteEvent, this, &ofApp::recordingComplete);
//
// NOTE: sound recording still doesn't work
//    soundStream.listDevices();
//    soundStream.setDeviceID(11);
//    ofSoundStreamSettings settings;
//    auto devices = soundStream.getMatchingDevices("Speakers");
////    if(!devices.empty()){
//        settings.setInDevice(devices[0]);
////    }
//    settings.sampleRate = sampleRate;
//    settings.numOutputChannels = 2;
//    settings.numInputChannels = 0;
//    settings.bufferSize = 256;
//    settings.setInListener(this);
//    soundStream.setup(settings);


//    ofSoundStreamSetup(this, 0, channels, sampleRate, 256, 4);
//    soundStream.setOutput(this);


    ofSetWindowShape(width, height);
    bRecording = false;
    
//    ofFbo recordFbo;
//    ofPixels recordPixels;
    
    recordFbo.allocate(width, height, GL_RGB);
    recordFbo.begin();
    ofClear(0, 0, 0, 255);
    recordFbo.end();


    
    ofBackground(12, 6, 0);
    ofSetBackgroundAuto(true);
    ofEnableBlendMode(OF_BLENDMODE_ADD);
    ofHideCursor();
    
    
    beat.load("circles.mp3");
    beat.play();
}

//--------------------------------------------------------------
void ofApp::update(){
    ofSoundUpdate();
    
    if (bRecording){
//        imgScreenshot.grabScreen(0, 0, ofGetWidth(), ofGetHeight());
//        recordPixels = imgScreenshot.getPixels();
        recordFbo.readToPixels(recordPixels);

        bool success = vidRecorder.addFrame(recordPixels);
        if (!success) {
            ofLogWarning("This frame was not added!");
        } else {
            log("Frame successfuly added");
        }
    }
    
//    vidGrabber.update();
//    if(vidGrabber.isFrameNew() && bRecording){
//        bool success = vidRecorder.addFrame(vidGrabber.getPixels());
//        if (!success) {
//            ofLogWarning("This frame was not added!");
//        } else {
//            log("Frame successfuly added");
//        }
//    }

    // Check if the video recorder encountered any error while writing video frame or audio smaples.
    if (vidRecorder.hasVideoError()) {
        ofLogWarning("The video recorder failed to write some frames!");
    }

    if (vidRecorder.hasAudioError()) {
        ofLogWarning("The video recorder failed to write some audio samples!");
    }
    
    // (5) grab the fft, and put in into a "smoothed" array,
    //        by taking maximums, as peaks and then smoothing downward
    float * val = ofSoundGetSpectrum(nBandsToGet);        // request 128 values for fft
    for (int i = 0; i < nBandsToGet; i++) {

        // let the smoothed value sink to zero:
        fftSmoothed[i] *= 0.96f;

        // take the max, either the smoothed or the incoming:
        if (fftSmoothed[i] < val[i]) fftSmoothed[i] = val[i];

    }

    // Update rolling average of FFT bucket (beat)
    for (int i = 0; i <= rollingAvgSize - 2; i++) {
        rollingAvgArr[i] = rollingAvgArr[i + 1]; //move all element to the right except last one
    }
    float fftVal = CLAMP(fftSmoothed[1], 0, 1.0);
    rollingAvgArr[rollingAvgSize - 1] = fftVal;
    
    // Update rolling average of FFT bucket (vocals/melody)
    float fftVal2Total = 0; //CLAMP(fftSmoothed[1], 0, 1.0);
    int rangeStart = 10;
    int rangeEnd = 18;
    for (int i = 10; i < 18; i++) {
        fftVal2Total += fftSmoothed[i];
    }
    float fftVal2Avg = fftVal2Total / (rangeEnd - rangeStart);
    rollingAvgArr2[rollingAvgSize - 1] = fftVal2Avg;
}

//--------------------------------------------------------------
void ofApp::draw(){
    if (bRecording) {
        recordFbo.begin();
        ofClear(0, 0, 0, 255);
    }

    stringstream ss;
    ss << "video queue size: " << vidRecorder.getVideoQueueSize() << endl
    << "audio queue size: " << vidRecorder.getAudioQueueSize() << endl
    << "FPS: " << ofGetFrameRate() << endl
    << (bRecording?"pause":"start") << " recording: r" << endl
    << (bRecording?"close current video file: c":"") << endl;
    
    int segments = 2000;
    int radius = 300;
    int shapeCount = 10;
    
    float gapX = ofMap(ofGetMouseX(), 0, ofGetWidth(), ofGetWidth() / 20, -ofGetWidth() / 20);
    float gapY = ofMap(ofGetMouseY(), 0, ofGetHeight(), ofGetHeight() / 20, -ofGetHeight() / 20);
    
    float shift = 0.75;
    
    float xCenter = ofGetWidth() / 2;
    float yCenter = 4.0 * (ofGetHeight() / 9.0);
    
    ofNoFill();
    ofSetColor(255,0,0);
    ofEnableBlendMode(OF_BLENDMODE_ADD);
    
    
    float sum = 0;
    float sum2 = 0;
    for (int i = 0; i < rollingAvgSize; i++) {
        sum += rollingAvgArr[i];
        sum2 += rollingAvgArr2[i];
    }
    float fftAvg = sum / rollingAvgSize;
    float fftAvg2 = sum2 / rollingAvgSize;
    
    
    
    float normalizedMouseX = (float)ofGetMouseX() / (float)ofGetWidth();
    float normalizedMouseY = (float)ofGetMouseY() / (float)ofGetHeight();
    
    // repeat for each concentric shape
    for (int j = 0; j < shapeCount; j++) {
        // ofset effects by shape count and time
        float multYShift = ofMap(j, 0, shapeCount, PI/10.0, 0);
        float multY = ofMap(sin((ofGetElapsedTimef()/9.0)+multYShift), -1.0, 1.0, 495.0, 505.0);
        
        // pulsating radius factor
        float rWave = 10 * sin((ofGetElapsedTimef() * 1.5) + multYShift);
        float r = ofMap(j, 0.0, shapeCount, radius + rWave, - 20);
        
        // average FFT bands from center (vocals)
        float fftAvgClamped2 = CLAMP(fftAvg2, 0, 0.003);
        float colorFacotr = ofMap(fftAvgClamped2, 0.0, 0.003, 0.0, 225.0);
        // dont start color modulation until 10 seconds in
        if (ofGetElapsedTimef() < 9.500) colorFacotr = 0;
        
        // set colors
        float red = ofMap(j, 0.0, shapeCount, 0.0, 160.0);
        float green = ofMap(j, 0.0, shapeCount, 0.0, 1.0) + colorFacotr;
        float blue = ofMap(j, 0.0, shapeCount, 0.0, 1.0) + + colorFacotr/2;
        
        // fft value from front of bands (beat)
        float fftAvgClamped = CLAMP(fftAvg, 0, 1.0);
        // determine alpha of shape, more visible at center shape and back shapes
        float maxAlpha2 = ofMap(fftAvgClamped, 0, 1.0, 0, 150);
        float alphaShift = ofMap(j, 0, shapeCount, PI/3.0, 0);
        float alpha = ofMap(j, 0.0, shapeCount, -20, maxAlpha2);
        
        if (j == shapeCount - 1) {
            // increase background color brightness based on alpha of "closest" cirlce
            float redBackground = MAX(ofMap(alpha, 0, 100.0, 0, 10.0), 0.0);
            float greenBackground = MAX(ofMap(alpha, 0, 100.0, 0, 5.0), 0.0);
            ofSetBackgroundColor(redBackground, greenBackground, 0);
        }
        
        // keep shapoe on screen when moving via mouse
        float shapeCenterX = xCenter - gapX * j;
        float shapeCenterY = yCenter - gapY * j + rWave;
        float shiftFactor = 0.1;
        if (normalizedMouseX > 0.5) {
            float shift = ofMap(normalizedMouseX, 0.5, 1.0, 0.0, shiftFactor) * ofGetWidth();
            shapeCenterX -= shift;
        } else {
            float shift = ofMap(normalizedMouseX, 0.5, 0.0, 0.0, shiftFactor) * ofGetWidth();
            shapeCenterX += shift;
        }
        if (normalizedMouseY > 0.5) {
            float shift = ofMap(normalizedMouseY, 0.5, 1.0, 0.0, shiftFactor) * ofGetHeight();
            shapeCenterY -= shift;
        } else {
            float shift = ofMap(normalizedMouseY, 0.5, 0.0, 0.0, shiftFactor) * ofGetHeight();
            shapeCenterY += shift;
        }
        
        // don't uncenter shape until use explicitly moves mouse
        if (!hasMouseMoved) {
            shapeCenterX = ofGetWidth() / 2;
            shapeCenterY = ofGetHeight() / 2;
        }
        
        // draw shape
        ofSetColor(red, green, blue, alpha);
        ofBeginShape();
        for (int i = 0; i < segments; i++) {
            float theta = (2 * PI) * ((float)i / (float)segments);
            float x = shapeCenterX +
            cos(theta + PI / 2.0) * r *
            (shift * cos(500 * theta + PI) + PI / 2.0);
            float y = shapeCenterY +
            sin(theta + PI / 2.0) * r *
            (sin(multY * theta + PI) + PI / 2.0);
            ofVertex(x, y);
        }
        ofEndShape(true);
    }
    
    
//    // see FFT visualizer
//    ofEnableAlphaBlending();
//        ofSetColor(255,255,255,100);
//        ofDrawRectangle(100,ofGetHeight()-300,5*128,200);
//    ofDisableAlphaBlending();
//    // draw the fft resutls:
//    ofSetColor(255,255,255,255);
//    float width = (float)(5*128) / nBandsToGet;
//    for (int i = 0;i < nBandsToGet; i++){
//        // (we use negative height here, because we want to flip them
//        // because the top corner is 0,0)
//        ofDrawRectangle(100+i*width,ofGetHeight()-100,width,-(fftSmoothed[i] * 200));
//    }
//    imgScreenshot.grabScreen(0, 0, ofGetWidth(), ofGetHeight());
//    if (ofRandom(0, 1.0) > 0.8) {
//        string filename = "img_" + ofToString(imgCount, 3, '0') + ".png";
//        imgScreenshot.save(filename);
//        imgCount++;
//    }
    
    if (bRecording) {
        recordFbo.end();
    }
}


//--------------------------------------------------------------
//void ofApp::audioRequested     (float * output, int bufferSize, int nChannels){
//
//    for (int i = 0; i < bufferSize; i++){
//
//        /* Stick your maximilian 'play()' code in here ! Declare your objects in testApp.h.
//
//         For information on how maximilian works, take a look at the example code at
//
//         http://www.maximilian.strangeloop.co.uk
//
//         under 'Tutorials'.
//
//         */
//
//
//
////        sample=beat.play(0.25, 0, beat.length);
//        wave=sine1.sinebuf(abs(mouseX));/* mouse controls sinewave pitch. we get abs value to stop it dropping
//                                         //                                         delow zero and killing the soundcard*/
//
//        mymix.stereo(sample + wave, outputs, 0.5);
//
//
//        output[i*nChannels    ] = outputs[0]; /* You may end up with lots of outputs. add them here */
//        output[i*nChannels + 1] = outputs[1];
//    }
//
//}
//
////--------------------------------------------------------------
//void ofApp::audioReceived     (float * input, int bufferSize, int nChannels){
//
//
//    /* You can just grab this input and stick it in a double, then use it above to create output*/
//
//    for (int i = 0; i < bufferSize; i++){
//
//        /* you can also grab the data out of the arrays*/
//
//
//    }
//
//}

//--------------------------------------------------------------
void ofApp::audioIn(float *input, int bufferSize, int nChannels){
    if(bRecording)
        vidRecorder.addAudioSamples(input, bufferSize, nChannels);
}

//--------------------------------------------------------------
void ofApp::recordingComplete(ofxVideoRecorderOutputFileCompleteEventArgs& args){
    cout << "The recoded video file is now complete." << endl;
}

//--------------------------------------------------------------
void ofApp::exit(){
    ofRemoveListener(vidRecorder.outputFileCompleteEvent, this, &ofApp::recordingComplete);
    vidRecorder.close();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    if (key == '1') {
        ofDisableBlendMode();
    } else if (key == '2') {
        ofEnableBlendMode(OF_BLENDMODE_ADD);
    }
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){
    if(key=='r'){
        bRecording = !bRecording;
        if(bRecording && !vidRecorder.isInitialized()) {
//            vidRecorder.setup(fileName+ "_" + ofGetTimestampString()+fileExt, ofGetWidth(), ofGetHeight(), 30, sampleRate, channels);
          vidRecorder.setup(fileName+ "_" + ofGetTimestampString()+fileExt, ofGetWidth(), ofGetHeight(), 30); // no audio
//            vidRecorder.setup(fileName+ofGetTimestampString()+fileExt, 0,0,0, sampleRate, channels); // no video
//          vidRecorder.setupCustomOutput(vidGrabber.getWidth(), vidGrabber.getHeight(), 30, sampleRate, channels, "-vcodec mpeg4 -b 1600k -acodec mp2 -ab 128k -f mpegts udp://localhost:1234"); // for custom ffmpeg output string (streaming, etc)

            // Start recording
            vidRecorder.start();
        }
        else if(!bRecording && vidRecorder.isInitialized()) {
            vidRecorder.setPaused(true);
        }
        else if(bRecording && vidRecorder.isInitialized()) {
            vidRecorder.setPaused(false);
        }
    }
    if(key=='c'){
        bRecording = false;
        vidRecorder.close();
    }
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y){
    if (!hasMouseMoved) hasMouseMoved = true;
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
