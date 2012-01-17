// CappConfig.cpp : Defines xml config
//
/*
* author: Maciej Lichoñ
*/
#include "stdafx.h"
using namespace std;
CAppConfig::CAppConfig()
{
}
CAppConfig::CAppConfig(string conf)
{
//	this->LoadDefaults();
	if(!Load())
	{
		SaveDefaults();
	}
	Load();

}
CAppConfig * CAppConfig::LoadDefaults()
{
_conffile="conf.cfg";
this->_InputFrameFiles.push_front("test");
    this->_recordIL="none";
	this->_recordIR="none";
	this->_recordOnInit = false;
	this->_conffile = "conf.cfg";
	this->_camL = 0;
	this->_camR = 1;
	return this;
}
CAppConfig::~CAppConfig()
{
	
}
bool CAppConfig::Load()
{
	try{
		CAppConfig conf ;
		std::ifstream ifs(_conffile.c_str());
		if(ifs.good())
		{
			//create serializer objecrt
			boost::archive::xml_iarchive ia(ifs);
			ia >> BOOST_SERIALIZATION_NVP(conf);
			return true;
		}
		else
		{
			return false;
		}
	}
    catch (const ios::failure& error) {
        cerr << "I/O exception: " << error.what() << endl;
        return false;
    }
    catch (const exception& error) {
        cerr << "standard exception: " << error.what() << endl;
        return false;
    }
    catch (...) {
        cerr << "unknown exception" << endl;
        return false;
    }
	//return false;
}
bool CAppConfig::SaveDefaults()
{
	CAppConfig  p = *LoadDefaults();
	Save();
	return false;
}

template<class archive>
void CAppConfig::serialize(archive& ar, const unsigned int version)
{
using  boost::serialization::make_nvp;
		ar & make_nvp("RecordOnStart", this->_recordOnInit);
		ar & make_nvp("RecordLeftFile", this->_recordIL);
		ar & make_nvp("RecordRightFile", this->_recordIR);
		ar & make_nvp("RecordRightFile", this->_recordIR);	
		ar & make_nvp("InputFrameFiles", this->_InputFrameFiles);
		ar & make_nvp("CameraLeftNum", this->_camL);
		ar & make_nvp("CameraRightNum", this->_camR);
}
void CAppConfig::Save()
{
	std::ofstream ofs(CAppConfig::_conffile.c_str());
	boost::archive::xml_oarchive oa(ofs);
    oa <<  boost::serialization::make_nvp("MasterWorkConfig", this);
}