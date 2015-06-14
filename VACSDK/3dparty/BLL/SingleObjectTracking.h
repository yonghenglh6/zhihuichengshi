#pragma once

#include <string>
#include <vector>
#include "TrackItem.h"
#include "TrackItems.h"
#include "TrackMessage.h"
#include "TrackTarget.h"
#include "AnalysisSDKBase.h"
#include "VideoCaptureBase.h"

class __declspec(dllexport) SingleObjectTracking
{
public:
	SingleObjectTracking(void);
	~SingleObjectTracking(void);

	int start(const std::string videoPath, const std::string xmlPath, VideoCaptureBase* videoCapture, TrackTarget& target);
	int start2(const std::string path, VideoCaptureBase* videoCapture, TrackTarget target);
	int getOneFrame(FrameInfo& param);
	bool pushOneInfo(const SaveInfo& param);
	bool saveInfoToFile();
	bool saveInfoToFile(const std::string savePath);
	bool saveInfoToFileAcrossCameras(const std::string savaPath);
	void stop();

	void printTrackTarget();
	void loadGroundTruthConvergePath(const std::string gdpath,const std::string trajectorypath);
public:
	string xmlFilePath_;
	std::string videoPath_;
	int targetIndex_;
	TrackItem *item_;
	TrackItems *items_;
	TrackMessage trackMessage_;
private:
	bool isInit() const;
	void loadConfigFile(const std::string configFilePath);

private:
	VideoCaptureBase* videoCapture_;
	bool isInit_;
	
	int frameIndex_;
	int oldFrameIndex_;

	
	TrackTarget trackTarget_;
	int startFrameNum_;
	int endFrameNum_;
	
};
