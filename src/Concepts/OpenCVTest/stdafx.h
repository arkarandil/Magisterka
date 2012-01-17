// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once
#include "targetver.h"
#include <stdio.h>
#include <tchar.h>
#include <cvaux.h>
#include <ml.h>
#include <highgui.h>
#include <string>
#include <fstream>
#include <iostream>
//Configuration Serialization
#include <boost/serialization/list.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/basic_xml_oarchive.hpp>
#include <boost/archive/basic_xml_iarchive.hpp>
#include <boost/archive/impl/xml_iarchive_impl.ipp>
#include <boost/archive/impl/xml_oarchive_impl.ipp>
#include <boost/archive/impl/basic_xml_oarchive.ipp>
#include <boost/archive/impl/basic_xml_iarchive.ipp>
#include <boost/archive/basic_text_oprimitive.hpp>
#include <boost/archive/basic_text_iprimitive.hpp>
#include <boost/archive/impl/basic_text_oprimitive.ipp>
#include <boost/archive/impl/basic_text_iprimitive.ipp>
#include <boost/archive/impl/basic_xml_grammar.hpp>
//END Configuration Serialization
#include "CAppConfig.h"

#include "CStereoVideo.h"





// TODO: reference additional headers your program requires here
