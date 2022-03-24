#pragma once

#include "ofMain.h"
#include "ofxVideoRecorder.h"

class ofApp : public ofBaseApp{
	public:
		void setup();
		void update();
		void draw();
        void exit();
		
		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y);
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
        void audioIn(float * input, int bufferSize, int nChannels);
    
    
    
        ofSoundPlayer         beat;
        static constexpr size_t nBandsToGet = 128;
        std::array<float, nBandsToGet> fftSmoothed{{0}};
    
        static const int rollingAvgSize = 75;
        float rollingAvgArr[rollingAvgSize] = { 0.0 };
        float rollingAvgArr2[rollingAvgSize] = { 0.0 };
    
        bool hasMouseMoved = false;
    
        ofVideoGrabber      vidGrabber;
        ofxVideoRecorder    vidRecorder;
        ofSoundStream       soundStream;
        bool bRecording;
        int sampleRate;
        int channels;
        string fileName;
        string fileExt;

        void recordingComplete(ofxVideoRecorderOutputFileCompleteEventArgs& args);
        void updateVid(ofImage &img);

        ofFbo recordFbo;
        ofPixels recordPixels;


        ofImage imgScreenshot;
        int imgCount = 0;
    
};
