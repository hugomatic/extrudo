
#ifndef CONFIGURATION_H_
#define CONFIGURATION_H_

#define GRUE_VERSION  "v0.0.4.0"
#define GRUE_PROGRAM_NAME  "Miracle-Grue"

#include <sstream>
#include <vector>
#include <string>

#include <json/reader.h>
#include <json/writer.h>
#include <json/value.h>
#include <json/writer.h>

#include "mgl.h"



namespace mgl
{

class ConfigException : public Exception {public: ConfigException(const char *msg):Exception(msg) {	} };

// checks that the value exist before returning it
double doubleCheck(const Json::Value &value, const char *name);
unsigned int uintCheck(const Json::Value &value, const char *name);
std::string stringCheck(const Json::Value &value, const char *name);
bool boolCheck(const Json::Value &value, const char *name);
//
// This class contains settings for the 3D printer, and user preferences
//
class Configuration {

public: void readFromString(const char* jsonStr);


public:
	Json::Value root;

	Configuration();
         ~Configuration();

	void readFromFile(const char* filename);
	void readFromFile(const std::string &filename) { readFromFile(filename.c_str()); };

	/// index function, to read/write values as config["foo"]
	Json::Value& operator[] (const char* key)
	{

		return this->root[key];	}
	const Json::Value& operator[] (const char* key) const
	{
		return this->root[key];
	}

	std::string asJson(Json::StyledWriter writer = Json::StyledWriter()) const;


};

struct GCoderConfig;
struct SlicerConfig;

void loadGCoderConfigFromFile(const Configuration& conf, GCoderConfig &gcoder);
void loadSlicerConfigFromFile( const Configuration &config, SlicerConfig &slicer);

}
#endif /* CONFIGURATION_H_ */
