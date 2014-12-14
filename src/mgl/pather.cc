#include "pather.h"

using namespace mgl;
using namespace std;

Pather::Pather(ProgressBar * progress)
		:Progressive(progress)
{
}


void Pather::generatePaths(const Tomograph &tomograph,
					const Regions &skeleton,
					std::vector<SliceData> &slices,
					int sfirstSliceIdx,  // =-1
					int slastSliceIdx )  //
{

	size_t sliceCount = tomograph.outlines.size();
	size_t firstSliceIdx = 0;
	size_t lastSliceIdx  = sliceCount-1;

	if(sfirstSliceIdx > 0 )
	{
		firstSliceIdx = (size_t)sfirstSliceIdx;
	}

	if(slastSliceIdx  > 0 )
	{
		lastSliceIdx = (size_t)slastSliceIdx;
	}

	slices.resize(sliceCount);
	bool direction = false;
	unsigned int currentSlice = 0;

	initProgress("Path generation", sliceCount);

	for(size_t i=0; i < sliceCount; i++)
	{
		tick();

		if(i <  firstSliceIdx) continue;
		if(i > lastSliceIdx) break;

		direction = !direction;
		SliceData& slice = slices[i];

		Scalar z = tomograph.layerMeasure.sliceIndexToHeight(currentSlice);
		currentSlice ++;

		slice.updatePosition(z, i);
		slice.extruderSlices.resize(1);

		ExtruderSlice &extruderSlice = slice.extruderSlices[0];

		const libthing::Insets &insetsSegments = skeleton.insets[i];
		const libthing::SegmentTable &outlineSegments = tomograph.outlines[i];

		outlines(outlineSegments, extruderSlice.boundary);

		PolygonsGroup &insetPolys = extruderSlice.insetLoopsList;
		this->insets(insetsSegments, insetPolys );

		const GridRanges &infillRanges = skeleton.infills[i];

		Polygons &infillsPolygons = extruderSlice.infills;
		this->infills(infillRanges, tomograph.grid, outlineSegments,
					  direction, infillsPolygons);
	}
}

void Pather::outlines(const libthing::SegmentTable& outlinesSegments, Polygons &boundary)
{
	createPolysFromloopSegments(outlinesSegments, boundary);
}

void Pather::insets(const libthing::Insets& insetsForSlice, PolygonsGroup &insetPolys)
{
	size_t nbOfShells = insetsForSlice.size();
	polygonsFromLoopSegmentTables(nbOfShells, insetsForSlice, insetPolys);
}

void Pather::infills(const GridRanges &infillRanges,
					 const Grid &grid,
					 const libthing::SegmentTable &outline,
					 bool direction,
					 Polygons &infills)
{
	grid.polygonsFromRanges(infillRanges, outline, direction, infills);
}
