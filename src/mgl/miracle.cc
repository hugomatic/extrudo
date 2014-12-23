

#include "JsonConverter.h"
#include <json/writer.h>

// #include "abstractable.h"
#include "miracle.h"

using namespace std;
using namespace mgl;
using namespace Json;
using namespace libthing;


//// @param slices list of output slice (output )
void mgl::miracleGrue(const GCoderConfig &gcoderCfg,
                      const SlicerConfig &slicerCfg,
                      const char *modelFile,
                      const char *, // scadFileStr,
                      const char *gcodeFile,
                      int firstSliceIdx,
                      int lastSliceIdx,
                      Tomograph &tomograph,
                      Regions &regions,
                      std::vector< SliceData >  &slices,
                      ProgressBar *progress)
{
	
	Meshy mesh(slicerCfg.firstLayerZ, slicerCfg.layerH);
	Slicer slicer(slicerCfg, progress);
	Regioner regioner(slicerCfg, progress);
	Pather pather(progress);
	std::ofstream gout(gcodeFile);
        GCoder gcoder(gcoderCfg, progress);


	mesh.readStlFile(modelFile);

	slicer.tomographyze(mesh, tomograph);

	regioner.generateSkeleton(tomograph, regions);

	pather.generatePaths(tomograph, regions, slices);


        gcoder.writeGcodeFile(slices, tomograph.layerMeasure, gout, modelFile, firstSliceIdx, lastSliceIdx);

	gout.close();

}




