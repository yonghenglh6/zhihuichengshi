#include <iostream>
#include <fstream>
#include <string>
#include <vector>
using namespace std;

#include "cv.h"
using namespace cv;
using namespace std;
#include "tinyxml.h"
#include"highgui.h"

#include "Common.h"
#include "SDKDefines.h"
#include "SaveInfo.h"
#include "Util.h"

#include "TrackTargetItem.h"
#include "TrackTargetItems.h"
#include "SingleObjectTracking.h"
#include "TrackLabel.h"

SingleObjectTracking::SingleObjectTracking(void)
{
	targetIndex_ = 0;
	frameIndex_ = 0;
}

SingleObjectTracking::~SingleObjectTracking(void)
{
}

int SingleObjectTracking::start(const std::string path, const std::string xmlPath, VideoCaptureBase* videoCapture, TrackTarget& target)
{
	if (true == isInit())
	{
		cerr << "Have started!" << endl;
		return -1;
	}
	if (NULL == videoCapture)
	{
		cerr << "Invalidate parameter videoCapture. " << __FILE__ << " " << __LINE__ << endl; 
	}

	loadConfigFile(xmlPath);
	videoCapture_ = videoCapture;

	if (-1 == videoCapture_->open(path))
	{
		cerr << "Can not open video: " << path << endl;
		return -1;
	}

	videoPath_ = path;
	startFrameNum_ = atoi(trackTarget_.trackTargetItemSet_[targetIndex_].startFrameNum_.c_str());
	endFrameNum_ = atoi(trackTarget_.trackTargetItemSet_[targetIndex_].endFrameNum_.c_str());
	frameIndex_ = startFrameNum_ - 1;
	oldFrameIndex_ = startFrameNum_;
	target = trackTarget_;
	item_ = new TrackItem();
	items_ = new TrackItems();
	item_->frameNum_ = trackTarget_.trackTargetItemSet_[targetIndex_].startFrameNum_;
	
	items_->jobID_ = trackTarget_.trackTargetItemSet_[targetIndex_].jobID_;
	items_->startFrameNum_ = trackTarget_.trackTargetItemSet_[targetIndex_].startFrameNum_;
	items_->endFrameNum_ = trackTarget_.trackTargetItemSet_[targetIndex_].endFrameNum_;
	items_->trackItemSet_.push_back(item_);
	trackMessage_.trackItemsSet_.push_back(items_);
	isInit_ = true;

	return 0;
}

int SingleObjectTracking::start2(const std::string path, VideoCaptureBase* videoCapture, TrackTarget target)
{
	if (true == isInit())
	{
		cerr << "Have started!" << endl;
		return -1;
	}
	if (NULL == videoCapture)
	{
		cerr << "Invalidate parameter videoCapture. " << __FILE__ << " " << __LINE__ << endl; 
	}

	videoCapture_ = videoCapture;
	if (-1 == videoCapture_->open(path))
	{
		cerr << "Can not open video: " << path << endl;
		return -1;
	}

	trackTarget_ = target;

	videoPath_ = path;
	startFrameNum_ = atoi(trackTarget_.trackTargetItemSet_[targetIndex_].startFrameNum_.c_str());
	endFrameNum_ = atoi(trackTarget_.trackTargetItemSet_[targetIndex_].endFrameNum_.c_str());
	frameIndex_ = startFrameNum_ - 1;
	oldFrameIndex_ = startFrameNum_;
	target = trackTarget_;
	item_ = new TrackItem();
	item_->frameNum_ = trackTarget_.trackTargetItemSet_[targetIndex_].startFrameNum_;
	items_ = new TrackItems();
	items_->jobID_ = trackTarget_.trackTargetItemSet_[targetIndex_].jobID_;
	items_->startFrameNum_ = trackTarget_.trackTargetItemSet_[targetIndex_].startFrameNum_;
	items_->endFrameNum_ = trackTarget_.trackTargetItemSet_[targetIndex_].endFrameNum_;
	items_->trackItemSet_.push_back(item_);
	trackMessage_.trackItemsSet_.push_back(items_);
	isInit_ = true;

	return 0;
}
void SingleObjectTracking::loadGroundTruthConvergePath(const string gdpath,const string trajectorypath){
	cout<<"GDpath is :"<<gdpath<<endl;	
	cout<<"Trajectorypath is :"<<trajectorypath<<endl;	

	TrackTarget gdtarget;

	TiXmlDocument *doc = new TiXmlDocument(gdpath.c_str());
	doc->LoadFile();
	TiXmlElement *message = doc->RootElement();
	TiXmlElement *info = message->FirstChildElement();
	TiXmlElement *items = info->NextSiblingElement();
	while (items)
	{
		TrackTargetItems trackTargetItems;
		TiXmlAttribute *pAttribute = items->FirstAttribute();
		trackTargetItems.jobID_ = pAttribute->Value();
		pAttribute = pAttribute->Next();
		trackTargetItems.startFrameNum_ = pAttribute->Value();
		pAttribute = pAttribute->Next();
		trackTargetItems.endFrameNum_ = pAttribute->Value();
		char tfilename[80];
		sprintf(tfilename,"%s\%s.txt",trajectorypath.c_str(),trackTargetItems.jobID_.c_str());
//		cout<<"Trajectory is opened:"<<tfilename<<endl;
		ofstream trajectory(tfilename);
		if(!trajectory.is_open()){
			cout<<"Trajectory can't be opened:"<<tfilename<<endl;
			continue;
		}
//		cout<<"Trajectory is opened:"<<tfilename<<endl;
		TiXmlElement *item = items->FirstChildElement();
		while (item)
		{
			TrackTargetItem trackTargetItem;
			TiXmlAttribute *pAttribute = item->FirstAttribute();

			std::string frameNum=pAttribute->Value();
//			cout<<frameNum<<endl;
			TiXmlElement *label=item->FirstChildElement();
			pAttribute=label->FirstAttribute();
			trackTargetItem.type_ = pAttribute->Value();
			pAttribute = pAttribute->Next();
			trackTargetItem.left_ = pAttribute->Value();
			pAttribute = pAttribute->Next();
			trackTargetItem.top_ = pAttribute->Value();
			pAttribute = pAttribute->Next();
			trackTargetItem.right_ = pAttribute->Value();
			pAttribute = pAttribute->Next();
			trackTargetItem.bottom_ = pAttribute->Value();
			pAttribute = pAttribute->Next();
			trackTargetItem.id_ = pAttribute->Value();
			trackTargetItems.itemSet_.push_back(trackTargetItem);
			
			int midx=(atoi(trackTargetItem.left_.c_str())+atoi(trackTargetItem.right_.c_str()))/2;
//			cout<<x<<endl;
			int midy=(atoi(trackTargetItem.bottom_.c_str())+atoi(trackTargetItem.top_.c_str()))/2;
			
			trajectory<<frameNum<<" "<<midx<<" "<<midy<<endl;

			item = item->NextSiblingElement();
		}

		gdtarget.trackTargetItemSet_.push_back(trackTargetItems);
		items = items->NextSiblingElement();
	}
	
	
}
void SingleObjectTracking::loadConfigFile(const string configFilePath)
{
	TiXmlDocument *doc = new TiXmlDocument(configFilePath.c_str());
	doc->LoadFile();
	TiXmlElement *message = doc->RootElement();
	TiXmlElement *info = message->FirstChildElement();
	TiXmlElement *videoName = info->NextSiblingElement();
	TiXmlElement *mediaFile = videoName->NextSiblingElement();
	TiXmlElement *trackTarget = mediaFile->NextSiblingElement();
	TiXmlElement *items = trackTarget->FirstChildElement();
	while (items)
	{
		TrackTargetItems trackTargetItems;
		TiXmlAttribute *pAttribute = items->FirstAttribute();
		trackTargetItems.jobID_ = pAttribute->Value();
		pAttribute = pAttribute->Next();
		trackTargetItems.startFrameNum_ = pAttribute->Value();
		pAttribute = pAttribute->Next();
		trackTargetItems.endFrameNum_ = pAttribute->Value();

		TiXmlElement *item = items->FirstChildElement();
		while (item)
		{
			TrackTargetItem trackTargetItem;
			TiXmlAttribute *pAttribute = item->FirstAttribute();
			trackTargetItem.left_ = pAttribute->Value();
			pAttribute = pAttribute->Next();
			trackTargetItem.top_ = pAttribute->Value();
			pAttribute = pAttribute->Next();
			trackTargetItem.right_ = pAttribute->Value();
			pAttribute = pAttribute->Next();
			trackTargetItem.bottom_ = pAttribute->Value();
			pAttribute = pAttribute->Next();
			trackTargetItem.id_ = pAttribute->Value();
			pAttribute = pAttribute->Next();
			trackTargetItem.type_ = pAttribute->Value();
			trackTargetItems.itemSet_.push_back(trackTargetItem);
			item = item->NextSiblingElement();
		}

		trackTarget_.trackTargetItemSet_.push_back(trackTargetItems);
		items = items->NextSiblingElement();
	}
}

bool SingleObjectTracking::isInit() const
{
	return isInit_;
}

void SingleObjectTracking::printTrackTarget()
{
	for (vector<TrackTargetItems>::iterator it = trackTarget_.trackTargetItemSet_.begin(); 
		it != trackTarget_.trackTargetItemSet_.end(); ++it)
	{
		cout << it->jobID_ << " " << it->startFrameNum_ << " " << it->endFrameNum_ << endl;
		for (vector<TrackTargetItem>::iterator itItem = it->itemSet_.begin(); itItem != it->itemSet_.end(); ++itItem)
		{
			cout << "\t" << itItem->left_ << " " << itItem->top_ << " " << itItem->right_ << " " << itItem->bottom_ << " " 
				<< itItem->id_ << " " << itItem->type_ << endl;
		}
	}
}

int SingleObjectTracking::getOneFrame(FrameInfo& param)
{
	//oldFrameIndex_ = frameIndex_;
	++frameIndex_;
	if (frameIndex_ > endFrameNum_)
	{
		++targetIndex_;
		if (targetIndex_ == trackTarget_.trackTargetItemSet_.size())
		{
			return 0;
		}
		else
		{
			int oldEndFrameNum = endFrameNum_;
			startFrameNum_ = atoi(trackTarget_.trackTargetItemSet_[targetIndex_].startFrameNum_.c_str());
			param.frameSeq = startFrameNum_;
			cv::Mat oneFrame;
			for (int i = oldEndFrameNum + 1; i < startFrameNum_; ++i)
			{
				videoCapture_->getNextFrame(oneFrame);
			}
		}
	}
	else
	{
		param.frameSeq = frameIndex_;
	}
	//cv::Mat oneFrame;
	//;//videoCapture_->getNextFrame(oneFrame);
	//param.mediaFile = "";
	//param.img = oneFrame;
	//param.mediaFile = videoPath_;
	//const int BUFFER_SIZE = 256;
	//char buffer[BUFFER_SIZE];
	//memset(buffer, 0, BUFFER_SIZE);
	//sprintf_s(buffer, "%d", fr)
	return 0;
}

bool SingleObjectTracking::pushOneInfo(const SaveInfo& param)
{
	if (param.frameSeq != frameIndex_)
	{
		if (frameIndex_ > endFrameNum_)
		{
			item_ = new TrackItem();
			items_ = new TrackItems();
			std::cout<<targetIndex_<<" "<<trackTarget_.trackTargetItemSet_.size()<<endl;
			if (targetIndex_ < trackTarget_.trackTargetItemSet_.size())
			{
				items_->jobID_ = trackTarget_.trackTargetItemSet_[targetIndex_].jobID_;
				items_->startFrameNum_ = trackTarget_.trackTargetItemSet_[targetIndex_].startFrameNum_;
				items_->endFrameNum_ = trackTarget_.trackTargetItemSet_[targetIndex_].endFrameNum_;
				items_->trackItemSet_.push_back(item_);
				trackMessage_.trackItemsSet_.push_back(items_);
			}
			else
			{
				delete item_;
				delete items_;
				cerr << "Error." << " " << __FILE__ << " " << __LINE__ << endl;
				return false;
			}
			endFrameNum_ = atoi(trackTarget_.trackTargetItemSet_[targetIndex_].endFrameNum_.c_str());
		}
		frameIndex_ = param.frameSeq;
		oldFrameIndex_ = frameIndex_;
		const int BUFFER_SIZE = 256;
		char buffer[BUFFER_SIZE];
		memset(buffer, 0, BUFFER_SIZE);;
		sprintf_s(buffer, "%d", frameIndex_);
		item_->frameNum_ = string(buffer);
	}
	if (oldFrameIndex_ != frameIndex_)
	{
		const int BUFFER_SIZE = 256;
		char buffer[BUFFER_SIZE];
		memset(buffer, 0, BUFFER_SIZE);;
		sprintf_s(buffer, "%d", frameIndex_);
		item_ = new TrackItem();
		item_->frameNum_ = string(buffer);
		item_->frameNum_ = string(buffer);
		items_->trackItemSet_.push_back(item_);
		oldFrameIndex_ = frameIndex_;
	}
	TrackLabel *label = new TrackLabel();
	const int BUFFER_SIZE = 256;
	char buffer[BUFFER_SIZE];

	memset(buffer, 0, BUFFER_SIZE);
	sprintf_s(buffer, "%d", param.type);
	label->type_ = string(buffer);
	//
	memset(buffer, 0, BUFFER_SIZE);
	sprintf_s(buffer, "%d", param.left);
	label->left_ = string(buffer);
	//
	memset(buffer, 0, BUFFER_SIZE);
	sprintf_s(buffer, "%d", param.top);
	label->top_ = string(buffer);
	//
	memset(buffer, 0, BUFFER_SIZE);
	sprintf_s(buffer, "%d", param.right);
	label->right_ = string(buffer);
	//
	memset(buffer, 0, BUFFER_SIZE);
	sprintf_s(buffer, "%d", param.bottom);
	label->bottom_ = string(buffer);
	//
	memset(buffer, 0, BUFFER_SIZE);
	sprintf_s(buffer, "%d", param.id);
	label->id_ = string(buffer);
	item_->trackLabelSet_.push_back(label);
	//items_->trackItemSet_.push_back(item);
	return true;
}

bool SingleObjectTracking::saveInfoToFile()
{
	if (false == isInit() || 0 == trackMessage_.trackItemsSet_.size())
	{
		return false;
	}

	TiXmlDocument doc; 
	TiXmlDeclaration * decl = NULL;
	TiXmlElement * messageElement = NULL;
	TiXmlElement * infoElement = NULL;

	//header
	decl = new TiXmlDeclaration("1.0", "gb2312", ""); 

	messageElement = new TiXmlElement( "Message" );  
	messageElement->SetAttribute("Version", "1.0");

	infoElement = new TiXmlElement( "Info" ); 
	const int BUFFER_SIZE = 256;
	char evaluateType[BUFFER_SIZE];
	memset(evaluateType, 0, BUFFER_SIZE);
	sprintf_s(evaluateType, "%d", int(EnumEvaluateType_SingleObjectTrack));
	infoElement->SetAttribute("evaluateType", evaluateType);

	doc.LinkEndChild(decl);
	doc.LinkEndChild(messageElement); 

	messageElement->LinkEndChild(infoElement);

	int startPos = videoPath_.rfind("\\");
	string videoName = videoPath_.substr(startPos+1, videoPath_.length());
	infoElement->SetAttribute("mediaFile", videoName.c_str());

	for (int i = 0; i < trackMessage_.trackItemsSet_.size(); ++i)
	{ 
		TiXmlElement *itemsElement = new TiXmlElement("Items");
		itemsElement->SetAttribute("jobID", trackMessage_.trackItemsSet_[i]->jobID_.c_str());
		itemsElement->SetAttribute("startFrameNum", trackMessage_.trackItemsSet_[i]->startFrameNum_.c_str());
		itemsElement->SetAttribute("endFrameNum", trackMessage_.trackItemsSet_[i]->endFrameNum_.c_str());

		for (int j = 0; j < trackMessage_.trackItemsSet_[i]->trackItemSet_.size(); ++j)
		{
			TiXmlElement *itemElement = new TiXmlElement("Item");
			itemElement->SetAttribute("frameNum", trackMessage_.trackItemsSet_[i]->trackItemSet_[j]->frameNum_.c_str());
			for (int k = 0; k < trackMessage_.trackItemsSet_[i]->trackItemSet_[j]->trackLabelSet_.size(); ++k)
			{
				TiXmlElement *labelElement = new TiXmlElement("Label");
				const TrackLabel* trackLabel = trackMessage_.trackItemsSet_[i]->trackItemSet_[j]->trackLabelSet_[k];
				labelElement->SetAttribute("type", trackLabel->type_.c_str());
				labelElement->SetAttribute("l", trackLabel->left_.c_str());
				labelElement->SetAttribute("t", trackLabel->top_.c_str());
				labelElement->SetAttribute("r", trackLabel->right_.c_str());
				labelElement->SetAttribute("b", trackLabel->bottom_.c_str());
				labelElement->SetAttribute("id", trackLabel->id_.c_str());
				itemElement->LinkEndChild(labelElement);
			}
			itemsElement->LinkEndChild(itemElement);
		}
		messageElement->LinkEndChild(itemsElement);
	}

	int endPos = videoPath_.rfind(".");
	string videoNameWithoutPostfix = videoPath_.substr(startPos+1, endPos-startPos-1);
	string xmlFileName = videoNameWithoutPostfix + ".xml";
	doc.SaveFile(xmlFileName.c_str()); 
	doc.Clear();
	xmlFilePath_ = xmlFileName;

	string encryptedFile = videoNameWithoutPostfix + ".txt";
	if (false == Util::encrypt(xmlFileName, encryptedFile, 10240000))
	{
		cerr << "Encryption error at SingleObjectTracking::saveInfoToFile() " 
			<< __FILE__ << " " << __LINE__ << endl; 
	}
	//remove(xmlFileName.c_str());

	return true;
}


bool SingleObjectTracking::saveInfoToFile(const std::string savePath)
{
	if ( 0 == trackMessage_.trackItemsSet_.size())
	{
		return false;
	}

	TiXmlDocument doc; 
	TiXmlDeclaration * decl = NULL;
	TiXmlElement * messageElement = NULL;
	TiXmlElement * infoElement = NULL;

	//header
	decl = new TiXmlDeclaration("1.0", "gb2312", ""); 

	messageElement = new TiXmlElement( "Message" );  
	messageElement->SetAttribute("Version", "1.0");

	infoElement = new TiXmlElement( "Info" ); 
	const int BUFFER_SIZE = 256;
	char evaluateType[BUFFER_SIZE];
	memset(evaluateType, 0, BUFFER_SIZE);
	sprintf_s(evaluateType, "%d", int(EnumEvaluateType_SingleObjectTrack));
	infoElement->SetAttribute("evaluateType", evaluateType);

	doc.LinkEndChild(decl);
	doc.LinkEndChild(messageElement); 
	
	messageElement->LinkEndChild(infoElement);
	  
	//int startPos = videoPath_.rfind("\\");
	//string videoName = videoPath_.substr(startPos+1, videoPath_.length());
	infoElement->SetAttribute("mediaFile", videoPath_.c_str());
	//std::cout<<trackMessage_.trackItemsSet_.size()<<endl;
	
	for (int i = 0; i < trackMessage_.trackItemsSet_.size(); ++i)
	{ 

		
		TiXmlElement *itemsElement = new TiXmlElement("Items");
		itemsElement->SetAttribute("jobID", trackMessage_.trackItemsSet_[i]->jobID_.c_str());
		itemsElement->SetAttribute("startFrameNum", trackMessage_.trackItemsSet_[i]->startFrameNum_.c_str());
		itemsElement->SetAttribute("endFrameNum", trackMessage_.trackItemsSet_[i]->endFrameNum_.c_str());

		for (int j = 0; j < trackMessage_.trackItemsSet_[i]->trackItemSet_.size(); ++j)
		{
			//std::cout<<j<<endl;
			TiXmlElement *itemElement = new TiXmlElement("Item");
			itemElement->SetAttribute("frameNum", trackMessage_.trackItemsSet_[i]->trackItemSet_[j]->frameNum_.c_str());
			for (int k = 0; k < trackMessage_.trackItemsSet_[i]->trackItemSet_[j]->trackLabelSet_.size(); ++k)
			{
				//system("pause");
				TiXmlElement *labelElement = new TiXmlElement("Label");
				const TrackLabel* trackLabel = trackMessage_.trackItemsSet_[i]->trackItemSet_[j]->trackLabelSet_[k];
				labelElement->SetAttribute("type", trackLabel->type_.c_str());
				labelElement->SetAttribute("l", trackLabel->left_.c_str());
				labelElement->SetAttribute("t", trackLabel->top_.c_str());
				labelElement->SetAttribute("r", trackLabel->right_.c_str());
				labelElement->SetAttribute("b", trackLabel->bottom_.c_str());
				labelElement->SetAttribute("id", trackLabel->id_.c_str());
				itemElement->LinkEndChild(labelElement);
			}
			itemsElement->LinkEndChild(itemElement);
		}
		messageElement->LinkEndChild(itemsElement);
	}
	

	//int endPos = videoPath_.rfind(".");
	//string videoNameWithoutPostfix = videoPath_.substr(startPos+1, endPos-startPos-1);
	//string xmlFileName = videoNameWithoutPostfix + ".xml";
	//doc.SaveFile(xmlFileName.c_str()); 
	doc.SaveFile(savePath.c_str());
	
	doc.Clear();
	std::cout<<"finished"<<endl;
	//xmlFilePath_ = xmlFileName;

	/*string encryptedFile = videoNameWithoutPostfix + ".txt";
	if (false == Util::encrypt(savePath, encryptedFile, 10240000))
	{
		cerr << "Encryption error at SingleObjectTracking::saveInfoToFile() " 
			<< __FILE__ << " " << __LINE__ << endl; 
	}
	remove(xmlFileName.c_str());*/

	return true;
}



void SingleObjectTracking::stop()
{
	videoPath_ = "";
	//saveInfoSet_.clear();
	isInit_ = false;
}

bool SingleObjectTracking::saveInfoToFileAcrossCameras(const std::string savePath)
{
	if ( 0 == trackMessage_.trackItemsSet_.size())
	{
		return false;
	}

	TiXmlDocument doc; 
	TiXmlDeclaration * decl = NULL;
	TiXmlElement * messageElement = NULL;
	TiXmlElement * infoElement = NULL;

	//header
	decl = new TiXmlDeclaration("1.0", "gb2312", ""); 

	messageElement = new TiXmlElement( "Message" );  
	messageElement->SetAttribute("Version", "1.0");

	infoElement = new TiXmlElement( "Info" ); 
	const int BUFFER_SIZE = 256;
	char evaluateType[BUFFER_SIZE];
	memset(evaluateType, 0, BUFFER_SIZE);
	sprintf_s(evaluateType, "%d", int(EnumEvaluateType_MultiPedestrian));
	infoElement->SetAttribute("evaluateType", evaluateType);

	doc.LinkEndChild(decl);
	doc.LinkEndChild(messageElement); 
	
	messageElement->LinkEndChild(infoElement);
	  
	//int startPos = videoPath_.rfind("\\");
	//string videoName = videoPath_.substr(startPos+1, videoPath_.length());
	infoElement->SetAttribute("mediaFile", videoPath_.c_str());
	//std::cout<<trackMessage_.trackItemsSet_.size()<<endl;
	
	for (int i = 0; i < trackMessage_.trackItemsSet_.size(); ++i)
	{ 

		
		TiXmlElement *itemsElement = new TiXmlElement("Items");
		itemsElement->SetAttribute("VideoName", trackMessage_.trackItemsSet_[i]->videoName_.c_str());
		itemsElement->SetAttribute("startFrameNum", trackMessage_.trackItemsSet_[i]->startFrameNum_.c_str());
		itemsElement->SetAttribute("endFrameNum", trackMessage_.trackItemsSet_[i]->endFrameNum_.c_str());

		for (int j = 0; j < trackMessage_.trackItemsSet_[i]->trackItemSet_.size(); ++j)
		{
			//std::cout<<j<<endl;
			TiXmlElement *itemElement = new TiXmlElement("Item");
			itemElement->SetAttribute("frameNum", trackMessage_.trackItemsSet_[i]->trackItemSet_[j]->frameNum_.c_str());
			for (int k = 0; k < trackMessage_.trackItemsSet_[i]->trackItemSet_[j]->trackLabelSet_.size(); ++k)
			{
				//system("pause");
				TiXmlElement *labelElement = new TiXmlElement("Label");
				const TrackLabel* trackLabel = trackMessage_.trackItemsSet_[i]->trackItemSet_[j]->trackLabelSet_[k];
				labelElement->SetAttribute("type", trackLabel->type_.c_str());
				labelElement->SetAttribute("l", trackLabel->left_.c_str());
				labelElement->SetAttribute("t", trackLabel->top_.c_str());
				labelElement->SetAttribute("r", trackLabel->right_.c_str());
				labelElement->SetAttribute("b", trackLabel->bottom_.c_str());
				labelElement->SetAttribute("id", trackLabel->id_.c_str());
				itemElement->LinkEndChild(labelElement);
			}
			itemsElement->LinkEndChild(itemElement);
		}
		messageElement->LinkEndChild(itemsElement);
	}
	

	//int endPos = videoPath_.rfind(".");
	//string videoNameWithoutPostfix = videoPath_.substr(startPos+1, endPos-startPos-1);
	//string xmlFileName = videoNameWithoutPostfix + ".xml";
	//doc.SaveFile(xmlFileName.c_str()); 
	doc.SaveFile(savePath.c_str());
	
	doc.Clear();
	//xmlFilePath_ = xmlFileName;

	/*string encryptedFile = videoNameWithoutPostfix + ".txt";
	if (false == Util::encrypt(savePath, encryptedFile, 10240000))
	{
		cerr << "Encryption error at SingleObjectTracking::saveInfoToFile() " 
			<< __FILE__ << " " << __LINE__ << endl; 
	}
	remove(xmlFileName.c_str());*/

	return true;
}
