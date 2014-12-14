#include "gcoder.h"

using namespace mgl;
using namespace std;
using namespace libthing;

#include "log.h"
#include <math.h>

// function that adds an s to a noun if count is more than 1
std::string plural(const char*noun, int count, const char* ending = "s")
{
	string s(noun);
	if (count>1)
	{
		return s+ending;
	}
	return s;
}

//
// computes 2 positions (one before and one at the end of) the polygon and stores them in start and stop.
// These positions are aligned with the fisrt line and last line of the polygon.
// LeadIn is the distance between start and the first point of the polygon (along the first polygon line).
// LeadOut is the distance between the last point of the Polygon and stop (along the last polygon line).
void polygonLeadInAndLeadOut(const Polygon &polygon, const Extruder &extruder,
							 double leadIn, double leadOut,
							 Vector2 &start, Vector2 &end)
{
	size_t count =  polygon.size();

	const Vector2 &a = polygon[0];	// first element
	const Vector2 &b = polygon[1];

	const Vector2 &c = polygon[count-2];
	const Vector2 &d = polygon[count-1]; // last element

	if (extruder.isVolumetric()) {
		start = a;
		end = d;
		return;
	}

	Vector2 ab = b - a;
	ab.normalise();
	Vector2 cd = d-c;
	cd.normalise();

	start.x = a.x - ab.x * leadIn;
	start.y = a.y - ab.y * leadIn;
	end.x   = d.x + cd.x * leadOut;
	end.y   = d.y + cd.y * leadOut;

}


void Gantry::writeSwitchExtruder(ostream& ss, Extruder &extruder)
{
	ss << "( extruder " << extruder.id << " )" << endl;
	ss << "( GSWITCH T" << extruder.id << " )" << endl;
	ss << "( TODO: add offset management to Gantry )" << endl;
	ab = extruder.code;
	ss << endl;
}





void GCoder::writeMachineInitialization(std::ostream &ss) const
{
	ss <<  "G21 (set units to mm)" << endl;
	ss <<  "G90 (absolute positioning mode)" << endl;

	const GCoder &gcoder = *this;
	int toolHeadId = 0;
	if (gcoder.gcoderCfg.extruders.size() > 1)
	{
		for (std::vector<Extruder>::const_iterator i= gcoder.gcoderCfg.extruders.begin(); i!=gcoder.gcoderCfg.extruders.end(); i++)
		{
			const Extruder &t = *i;
			int coordinateSystemNb = toolHeadId +1;
			ss << "G10 P" << coordinateSystemNb << " X" <<  t.coordinateSystemOffsetX << " Y0 Z-0.3" << endl;
			toolHeadId ++;
		}
	}
	ss << endl;
}


void GCoder::writeExtrudersInitialization(std::ostream &ss) const
{

	string plural = "";
	if(gcoderCfg.extruders.size()>1) plural = "s";
	ss << "(setup extruder" << plural <<")" <<endl;
	int toolHeadId = 0;
	for (std::vector<Extruder>::const_iterator i= gcoderCfg.extruders.begin(); i!=gcoderCfg.extruders.end(); i++)
	{
		const Extruder &t = *i;
		ss << "M103 T" << toolHeadId << " (Make sure motor for extruder " << toolHeadId << " is stopped)" << endl;
		ss << "M104 S" << t.extrusionTemperature  << " T" << toolHeadId << " (set temperature of extruder " << toolHeadId <<  " to "  << t.extrusionTemperature << " degrees Celsius)" << endl;
		ss << endl;
		toolHeadId ++;
	}
	ss << endl;
}


void GCoder::writePlatformInitialization(std::ostream &ss) const
{

	double t = gcoderCfg.platform.temperature;
	ss << "M109 S" << t << " T0 (heat the build-platform to "  << t << " Celsius)" << endl;
	ss << endl;

}


void GCoder::writeHomingSequence(std::ostream &ss)
{
	ss << endl;
	ss << "(go to home position)" << endl;

	if(gcoderCfg.gantry.xyMaxHoming)
		ss << "G162 X Y F" << gcoderCfg.gantry.rapidMoveFeedRateXY<< " (home XY axes maximum)" << endl;
	else
		ss << "G161 X Y F" << gcoderCfg.gantry.rapidMoveFeedRateXY<< " (home XY axes minimum)" << endl;

	if(gcoderCfg.gantry.zMaxHoming)
		ss << "G162 Z F" << gcoderCfg.gantry.rapidMoveFeedRateZ<< " (home Z axis maximum)" << endl;
	else
		ss << "G161 Z F" << gcoderCfg.gantry.rapidMoveFeedRateZ<< " (home Z axis minimum)" << endl;

	ss << "G92 Z5 (set Z to 5)" << endl;
	ss << "G1 Z0.0 (move Z down 0)" << endl;

	if(gcoderCfg.gantry.zMaxHoming)
		ss << "G162 Z F" << gcoderCfg. gantry.homingFeedRateZ<< " (home Z axis maximum)" << endl;
	else
		ss << "G161 Z F" << gcoderCfg.gantry.homingFeedRateZ<< " (home Z axis minimum)" << endl;

	ss << "M132 X Y Z A B (Recall stored home offsets for XYZAB axis)" << endl;

	if (gcoderCfg.extruders.size() > 1)
		ss << "G54 (first work coordinate system)" << endl;
	ss << endl;

	int extruderCount = gcoderCfg.extruders.size();
	if (extruderCount >0)
	{
		ss << "G92 A0" << endl;
		if (extruderCount > 1) {
			ss << "G92 B0" << endl;
		}

		gcoderCfg.gantry.a = 0;
		gcoderCfg.gantry.b = 0;
		gcoderCfg.gantry.extruding = false;

		gcoderCfg.gantry.g1(ss, gcoderCfg.platform.waitingPositionX,
							gcoderCfg.platform.waitingPositionY,
							gcoderCfg.platform.waitingPositionZ,
							gcoderCfg.gantry.rapidMoveFeedRateXY,
							"go to waiting position");
						

	}
	else
	{
		stringstream ss;
		ss << "There are no extruders configured. Has the config file been read?";
		GcoderException mixup(ss.str().c_str());
		throw mixup;
	}
}

void GCoder::writeWarmupSequence(std::ostream &ss)
{

	ss << endl;
	size_t extruderCount = gcoderCfg.extruders.size();
	for (size_t i=0; i< extruderCount; i++)
	{
		ss << "M6 T" << i << " (wait for tool " << i<<" to reach temperature)" << endl;
	}

	ss << "(note: the heated build platform temperature is tied to tool 0 for now)" << endl;
	ss << endl;
}

/**
 * Writes intial gcode data to start of the gcode file, including setup & startup info
 * @param gout - output stream for the gcode text
 * @param sourceName - source of this gcode (usually the origional stl file)
 */
void GCoder::writeStartDotGCode(std::ostream &gout, const char* sourceName)
{
	gout.precision(3);
	gout.setf(ios::fixed);

	writeGCodeConfig(gout, sourceName);

    const string &header_file = gcoderCfg.header;

	if (header_file.length() > 0) {
		ifstream header_in(header_file.c_str(), ifstream::in);

		if (header_in.fail()) 
			throw GcoderException((string("Unable to open header file [") +
								   header_file + "]").c_str());

		gout << "(header [" << header_file << "] begin)" << endl;

		while (header_in.good()) {
			char buf[1024];

			header_in.read(buf, sizeof(buf));
			gout.write(buf, header_in.gcount());
		}

		if (header_in.fail() && !header_in.eof())
			throw GcoderException((string("Error reading header file [") +
								   header_file + "]").c_str());

		gout << "(header [" << header_file << "] end)" << endl << endl;
	}
	else {
		writeMachineInitialization(gout);
		writeExtrudersInitialization(gout);
		writePlatformInitialization(gout);
		writeHomingSequence(gout);
		writeWarmupSequence(gout);
		writeAnchor(gout);
	}
}

void GCoder::writeEndDotGCode(std::ostream &ss) const
{
    const string &footer_file = gcoderCfg.footer;


	if (footer_file.length() > 0) {
		ifstream footer_in(footer_file.c_str(), ifstream::in);

		if (footer_in.fail()) 
			throw GcoderException((string("Unable to open footer file [") +
								   footer_file + "]").c_str());

		ss << "(footer [" << footer_file << "] begin)" << endl;

		while (footer_in.good()) {
			char buf[1024];

			footer_in.read(buf, sizeof(buf));
			ss.write(buf, footer_in.gcount());
		}

		if (footer_in.fail() && !footer_in.eof())
			throw GcoderException((string("Error reading footer file [") +
								   footer_file + "]").c_str());

		ss << "(footer [" << footer_file << "] end)" << endl << endl;
	}		

	else {
		for (size_t i=0; i< gcoderCfg.extruders.size(); i++)
		{
			ss << "M104 S0 T" << i << " (set extruder temperature to 0)" << endl;
			ss << "M109 S0 T" << i << " (set heated-build-platform id tied an extrusion tool)" << endl;
		}

		if(gcoderCfg.gantry.zMaxHoming)
			ss << "G162 Z F500 (home Z axis maximum)" << endl;
		ss << "(That's all folks!)" << endl;
	}
}

void GCoder::writeAnchor(std::ostream &ss)
{
	double anchorFeedRate = 3000;
	double z = 0.6;


	ss << "(Create Anchor)" << endl;
	ss << "G1 Z0.6 F300    (Position Height)" << endl;
	ss << "M108 R4.0   (Set Extruder Speed)" << endl;
	ss << "M101        (Start Extruder)" << endl;
	ss << "G4 P1600" << endl;

	gcoderCfg.gantry.g1(  ss,
			gcoderCfg.platform.waitingPositionX,
			gcoderCfg.platform.waitingPositionY,
				z,
			gcoderCfg.gantry.rapidMoveFeedRateXY,
				NULL );

	double dx = gcoderCfg.platform.waitingPositionX - 3.0;
	double dy = gcoderCfg.platform.waitingPositionY - 0.0;

	gcoderCfg.gantry.g1(ss, dx, dy, z, 0.2 * anchorFeedRate , NULL);
	ss << "M103 (Stop extruder)" << endl;
	ss << endl;
}


void GCoder::writePolygon(	std::ostream & ss,
							double z,
							const Extruder &extruder,
							const Extrusion &extrusion,
							const Polygon & polygon)
{
    Vector2 start(0, 0), stop(0, 0);

    polygonLeadInAndLeadOut(polygon, extruder, extrusion.leadIn, extrusion.leadOut, start, stop);

    // rapid move into position
    gcoderCfg.gantry.g1(ss, extruder, extrusion,
						start.x, start.y, z,
						gcoderCfg.gantry.rapidMoveFeedRateXY,
						"move into position");

    // start extruding ahead of time while moving towards the first point
    gcoderCfg.gantry.squirt(ss, polygon[0], extruder, extrusion);

	Vector2 last = start;
   // for all other points in the polygon
    for(size_t i=1; i < polygon.size(); i++)
	{
    	// move towards the point
		const Vector2 &p = polygon[i];
		Scalar dist = LineSegment2(last, p).length();
		stringstream comment;
		comment << "d: " << dist;

		gcoderCfg.gantry.g1(ss, extruder, extrusion, p.x, p.y, z,
							extrusion.feedrate, comment.str().c_str());
	}
    //ss << "(STOP!)" << endl;
    gcoderCfg.gantry.snort(ss, stop, extruder, extrusion);
    //ss << "(!STOP)" << endl;
    ss << endl;

}


void GCoder::writePolygons(std::ostream& ss,
						   double z,
						   const Extruder &extruder,
						   const Extrusion &extrusion,
						   const Polygons &paths)
{
	unsigned int pathCount = paths.size();
	for (unsigned int i = 0 ; i < pathCount;  i ++)
	{

		const Polygon &polygon = paths[i];
		ss << "(  path " << i << "/" << pathCount << ", " << polygon.size() << " points, "  << " )" << endl;

		unsigned int pointCount = polygon.size();
		if(pointCount < 2)
		{
			stringstream ss;
			ss << "Can't generate gcode for polygon " << i <<" with " << pointCount << " points.";
			GcoderException mixup(ss.str().c_str());
			throw mixup;
		}

		writePolygon(ss, z, extruder, extrusion, polygon);
	}
}

void GCoder::moveZ(ostream & ss, double z, unsigned int  , double zFeedrate)
{
    bool doX = false;
    bool doY = false;
    bool doZ = true;
	bool doE = false;
    bool doFeed = true;


    gcoderCfg.gantry.g1Motion(ss, 0, 0, z, 0, zFeedrate, "move Z", doX, doY, doZ, doE, doFeed);

}


void GCoder::calcInfillExtrusion(unsigned int extruderId, unsigned int sliceId, Extrusion &extrusion) const
{
	string profileName;
	if(sliceId == 0)
	{
		profileName = gcoderCfg.extruders[extruderId].firstLayerExtrusionProfile;
	}
	else
	{
		profileName = gcoderCfg.extruders[extruderId].infillsExtrusionProfile;
	}

	const std::map<std::string, Extrusion>::const_iterator &it = gcoderCfg.extrusionProfiles.find(profileName);
	extrusion = it->second;
	extrusion.feedrate *= gcoderCfg.gantry.scalingFactor;
	extrusion.flow *= gcoderCfg.gantry.scalingFactor;
}

void GCoder::calcInSetExtrusion (   unsigned int extruderId,
                                    unsigned int sliceId,
                                    unsigned int , // insetId,
                                    unsigned int , // insetCount,
                                    Extrusion &extrusion) const
{
	string profileName;
	if(sliceId == 0)
	{
		profileName = gcoderCfg.extruders[extruderId].firstLayerExtrusionProfile;
	}
	else
	{
		profileName = gcoderCfg.extruders[extruderId].insetsExtrusionProfile;
	}

	const std::map<std::string, Extrusion>::const_iterator &it = gcoderCfg.extrusionProfiles.find(profileName);
	extrusion = it->second;
	extrusion.feedrate *= gcoderCfg.gantry.scalingFactor;
	extrusion.flow *= gcoderCfg.gantry.scalingFactor;
}

void GCoder::writeGcodeFile(std::vector <SliceData>& slices,
                             const mgl::LayerMeasure& layerMeasure,
                                std::ostream &gout,
                                    const char* title,
                                        int iFirstSliceIdx,
                                            int iLastSliceIdx)
{
    writeStartDotGCode(gout, title);

    size_t sliceCount = slices.size();
    size_t firstSliceIdx = 0;
    size_t lastSliceIdx = sliceCount-1;

    if(iFirstSliceIdx > 0) firstSliceIdx = (size_t) iFirstSliceIdx;
    if(iLastSliceIdx  < 0) lastSliceIdx  = (size_t) iLastSliceIdx;

    initProgress("gcode", sliceCount);
    size_t codeSlice = 0;
    for(size_t sliceId=0; sliceId < sliceCount; sliceId++)
    {
        tick();
        if(sliceId < firstSliceIdx) continue;
        if(sliceId > lastSliceIdx) break;

        Scalar z = layerMeasure.sliceIndexToHeight(codeSlice);
        SliceData &slice = slices[sliceId];
        slice.updatePosition(z, sliceId);

        writeSlice(gout, slice);
        codeSlice ++;
    }

	writeEndDotGCode(gout);
}

void GCoder::writeSlice(ostream& ss, const SliceData& sliceData )
{
	double layerZ = sliceData.getZHeight();
	unsigned int sliceIndex = sliceData.getIndex();
	unsigned int extruderCount = sliceData.extruderSlices.size();

	ss << "(Slice " << sliceIndex << ", " << extruderCount << " " << plural("Extruder", extruderCount) << ")"<< endl;
	// to each extruder its speed
	double zFeedrate = gcoderCfg.gantry.scalingFactor * gcoderCfg.extruders[0].zFeedRate;
	// moving all up. This is the first move for every new layer

	for(unsigned int extruderId = 0; extruderId < extruderCount; extruderId++)
	{
	    double z = layerZ + gcoderCfg.extruders[extruderId].nozzleZ;
		Extruder &extruder = gcoderCfg.extruders[extruderId];

		try
		{
		    moveZ(ss, z, extruderId, zFeedrate);
		}
		catch(GcoderException &mixup)
		{
                    Log::often() << "ERROR writing Z move in slice " << sliceIndex  << " for extruder " << extruderId << " : " << mixup.error << endl;
		}

	  	ss << "(   Extruder " <<  extruderId << ")" << endl;
		const Polygons &loops = sliceData.extruderSlices[extruderId].boundary;
		const Polygons &infills = sliceData.extruderSlices[extruderId].infills;
		const vector<Polygons> &insets = sliceData.extruderSlices[extruderId].insetLoopsList;

                // Log::often() << endl <<  "Slice " << sliceData.sliceIndex << endl;

		try
		{
			if (extruderCount > 0)
			{
				gcoderCfg.gantry.writeSwitchExtruder(ss, extruder);
			}
			if(gcoderCfg.doInfills && gcoderCfg.doInfillsFirst)
			{
				Extrusion extrusion;
				calcInfillExtrusion(extruderId, sliceIndex, extrusion);
				ss << "(infills: "  << infills.size() << ")"<< endl;
				writePolygons(ss, z, extruder, extrusion, infills);
			}
		}
		catch(GcoderException &mixup)
		{
			Log::often() << "ERROR writing infills in slice " << sliceIndex  << " for extruder " << extruderId << " : " << mixup.error << endl;
			Log::error() << "ERROR writing infills in slice " << sliceIndex  << " for extruder " << extruderId << " : " << mixup.error << endl;
		}
		/// Write outlines? outlines == skirt
		try
		{

			if(gcoderCfg.doOutlines)
			{
				Extrusion extrusion;
				calcInfillExtrusion(extruderId, sliceIndex, extrusion);
								//Log::often()  << "   Write OUTLINE" << endl;
				ss << "(outlines: " << loops.size() << " )"<< endl;
				writePolygons(ss, z, extruder, extrusion, loops);
			}
		}
		catch(GcoderException &mixup)
		{
                    Log::often()  << "ERROR writing loops in slice " << sliceIndex  << " for extruder " << extruderId << " : " << mixup.error << endl;
                    Log::error() << "ERROR writing loops in slice " << sliceIndex  << " for extruder " << extruderId << " : " << mixup.error << endl;
                    cerr << mixup.error << endl;
		}

		try
		{
			if( gcoderCfg.doInsets )
			{
				// each iteration is for a shell
				unsigned int insetCount = insets.size();
				for(unsigned int i=0; i < insetCount; i++)
				{
					Extrusion extrusion;
					calcInSetExtrusion(extruderId, sliceIndex, i, insetCount, extrusion);
					const Polygons &inset = insets[i];
                                        // Log::often() << "   Write INSETS " << i << endl;
					ss << "(inset " << i << "/"<<  insetCount<< " )"<< endl;
					writePolygons(ss, z, extruder, extrusion, inset);

				}
			}
		}
		catch(GcoderException &mixup)
		{
                        Log::often() << "ERROR writing infills in slice " << sliceIndex  << " for extruder " << extruderId << " : " << mixup.error << endl;
			cerr << "ERROR writing infills in slice " << sliceIndex  << " for extruder " << extruderId << " : " << mixup.error << endl;
		}

		try
		{
			if(gcoderCfg.doInfills && ! gcoderCfg.doInfillsFirst)
			{
                                //Log::often() << "   Write INFILLS" << endl;
				ss << "(infills: "  << infills.size() << ")"<< endl;
				Extrusion extrusion;
				calcInfillExtrusion(extruderId, sliceIndex, extrusion);
				writePolygons(ss, z, extruder, extrusion, infills);
			}
		}
		catch(GcoderException &mixup)
		{
                        Log::often() << "ERROR writing infills in slice " << sliceIndex  << " for extruder " << extruderId << " : " << mixup.error << endl;
			cerr << "ERROR writing infills in slice " << sliceIndex  << " for extruder " << extruderId << " : " << mixup.error << endl;
		}


		extruderId ++;
	}
}

Scalar Extrusion::crossSectionArea(Scalar height) const {
	Scalar width = height * extrudedDimensionsRatio;

	//two semicircles joined by a rectangle
	Scalar radius = height / 2;
	return (M_TAU / 2) * radius * radius + height * (width - height);
	//LONG LIVE TAU!
}

Scalar Extruder::feedCrossSectionArea()  const {
	Scalar radius = feedDiameter / 2;
	//feedstock should be a cylinder
	return (M_TAU / 2) * radius * radius;
	//LONG LIVE TAU!
}

Scalar Gantry::segmentVolume(const Extruder &, // extruder,
							 const Extrusion &extrusion,
							 LineSegment2 &segment) const {
	Scalar cross_area = extrusion.crossSectionArea(layerH);
	Scalar length = segment.length();

	return cross_area * length;
}
	

Scalar Gantry::volumetricE(	const Extruder &extruder,
							const Extrusion &extrusion,
							Scalar x, Scalar y, Scalar // z
						   ) const
{

	//There isn't yet a LineSegment3, so for now I'm assuming that only 2d
	//segments get extruded
	LineSegment2 seg(Vector2(this->x, this->y), Vector2(x, y));
	Scalar seg_volume = segmentVolume(extruder, extrusion, seg);

	Scalar feed_cross_area = extruder.feedCrossSectionArea();

	Scalar feed_len = seg_volume / feed_cross_area;

	return feed_len + getCurrentE();
}

/*if extruder and extrusion are null we don't extrude*/
void Gantry::g1(std::ostream &ss,
				const Extruder *extruder, const Extrusion *extrusion,
				double x, double y, double z, double feed,
				const char *comment = NULL)
{

	bool doX = true;
	bool doY = true;
	bool doZ = true;
	bool doFeed = true;
	bool doE = false;
	Scalar e = getCurrentE();

	if(!libthing::tequals(this->x, x, SAMESAME_TOL))
	{
		doX = true;
	}
	if(!libthing::tequals(this->y, y, SAMESAME_TOL))
	{
		doY=true;
	}
	if(!libthing::tequals(this->z, z, SAMESAME_TOL))
	{
		doZ=true;
	}

	if(!libthing::tequals(this->feed, feed, SAMESAME_TOL))
	{
		doFeed=true;
	}

	if(extruding && extruder != NULL && extrusion != NULL
	   && extruder->isVolumetric()) {
		doE = true;
		e = volumetricE(*extruder, *extrusion, x, y, z);
	}		

	g1Motion(ss, x, y, z, e, feed, comment,
			 doX, doY, doZ, doE, doFeed);
}

void Gantry::squirt(std::ostream &ss, const Vector2 &lineStart,
					const Extruder &extruder, const Extrusion &extrusion)
{
	if (extruder.isVolumetric()) {
		g1Motion(ss, x, y, z,
				 getCurrentE() + extrusion.retractDistance
				 + extrusion.restartExtraDistance, extrusion.retractRate,
				 "squirt", false, false, false, true, true); //only E and F
	}
	else {
		ss << "M108 R" <<  extrusion.squirtFlow << " (squirt)" << endl;
		ss << "M101" << endl;
		g1(ss, extruder, extrusion,
		   lineStart.x, lineStart.y, z, extrusion.squirtFeedrate, NULL);
		ss << "M108 R" << extrusion.flow << " (good to go)" << endl;
	}

	extruding = true;
}

void Gantry::snort(std::ostream &ss, const Vector2 &lineEnd,
				   const Extruder &extruder, const Extrusion &extrusion)
{
	if (extruder.isVolumetric()) {
		g1Motion(ss, x, y, z,
				 getCurrentE() - extrusion.retractDistance,
				 extrusion.retractRate, "snort",
				 false, false, false, true, true); //only E and F
	}
	else {
		ss << "M108 R" << extrusion.snortFlow << "  (snort)" << endl;
		ss << "M102" << endl;
		g1(ss, extruder, extrusion, lineEnd.x, lineEnd.y, z,
		   extrusion.snortFeedrate, NULL);
		ss << "M103" << endl;
	}

	extruding = false;
}




void Gantry::g1Motion(std::ostream &ss, double x, double y, double z, double e,
					  double feed, const char *g1Comment, bool doX,
					  bool doY, bool doZ, bool doE, bool doFeed)
{

	// not do something is not an option .. under certain conditions
	#ifdef STRONG_CHECKING
	if( !(doX || doY || doZ || doFeed )   )
	{
		stringstream ss;
		ss << "G1 without moving where x=" << x << ", y=" << y << ", z=" << z << ", feed=" << feed ;
		GcoderException mixup(ss.str().c_str());
		throw mixup;
	}
	#endif

	// our moto: don't be bad!
	bool bad = false;
	if(fabs(x) > MUCH_LARGER_THAN_THE_BUILD_PLATFORM_MM) bad = true;
	if(fabs(y) > MUCH_LARGER_THAN_THE_BUILD_PLATFORM_MM) bad = true;
	if(fabs(z) > MUCH_LARGER_THAN_THE_BUILD_PLATFORM_MM) bad = true;
	if(feed <= 0 || feed > 100000) bad = true;

	if(bad)
	{
		stringstream ss;
		ss << "Illegal G1 move where x=" << x << ", y=" << y << ", z=" << z << ", feed=" << feed ;
		GcoderException mixup(ss.str().c_str());
		throw mixup;
	}


	ss << "G1";
	if(doX) ss << " X" << x;
	if(doY) ss << " Y" << y;
	if(doZ) ss << " Z" << z;
	if(doFeed) ss << " F" << feed;
	if(doE) ss << " " << ab << e;
	if(g1Comment) ss << " (" << g1Comment << ")";
	ss << endl;

	// if(feed >= 5000) assert(0);

	// update state machine
	if (doX) this->x = x;
	if (doY) this->y = y;
	if (doZ) this->z = z;
	if (doFeed) this->feed = feed;
	if (doE) setCurrentE(e);

}

/**
 * Writes config header metadata into a gcode file
 * @param ss Stream to write config data to
 * @param sourceName - Name of source of this model. Usually the original .stl filename
 */
void GCoder::writeGCodeConfig(std::ostream &ss, const char* title="unknown source") const
{
	std::string indent = "* ";
	ss << endl;
	ss << "(Makerbot Industries)" << endl;
	ss << "(This file contains digital fabrication directives in gcode format)"<< endl;
	ss << "(For your 3D printer)" << endl;
	ss << "(http://wiki.makerbot.com/gcode)" <<  endl;

	MyComputer hal9000;

	ss << "(" << indent << "Generated by "<<  gcoderCfg.programName << " " << getMiracleGrueVersionStr() << ")"<< endl;
	ss << "(" << indent << hal9000.clock.now() <<  ")" << endl;
	ss << "(" << indent << "machine name: " << gcoderCfg.machineName << ")"<< endl;
	ss << "(" << indent << "firmware revision:" << gcoderCfg.firmware << ")" << endl;
	ss << "(" << indent << title << ")" << endl;

	std::string plurial = gcoderCfg.extruders.size()? "":"s";
	ss << "(" << indent << gcoderCfg.extruders.size() << " extruder" << plurial << ")" << endl;

	ss << "(" << indent << "Extrude infills: " << gcoderCfg.doInfills <<  ")" << endl;
	ss << "(" << indent;
	if(gcoderCfg.doInfillsFirst )
		ss << "first operation: Infill";
	else
		ss << "first operation: Insets";
	ss << ")" << endl;
	ss << "(" << indent << "Extrude insets: " << gcoderCfg.doInsets << ")" << endl;
	ss << "(" << indent << "Extrude outlines: " << gcoderCfg.doOutlines << ")" << endl;
	ss << endl;
}





