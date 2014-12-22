#include "connexity.h"

using namespace mgl;
using namespace std;
using namespace libthing;


index_t mgl::findOrCreateVertexIndex(std::vector<Vertex>& vertices ,
								const Vector3 &coords,
								Scalar tolerence)
{

	for(std::vector<Vertex>::iterator it = vertices.begin(); it != vertices.end(); it++)
	{
		//const Vector3 &p = (*it).point;
		Vector3 &p = (*it).point;
		Scalar dx = coords.x - p.x;
		Scalar dy = coords.y - p.y;
		Scalar dz = coords.z - p.z;

		Scalar dd =  dx * dx + dy * dy + dz * dz;
		if( dd < tolerence )
		{
			//EZLOGGERVLSTREAM(axter::log_often) << "Found VERTEX" << std::endl;
			index_t vertexIndex = std::distance(vertices.begin(), it);
			return vertexIndex;
		}
	}

	index_t vertexIndex;
	// EZLOGGERVLSTREAM(axter::log_often) << "NEW VERTEX " << coords << std::endl;
	Vertex vertex;
	vertex.point = coords;
	vertices.push_back(vertex);
	vertexIndex = vertices.size() -1;
	return vertexIndex;
}



Connexity::Connexity(Scalar tolerence)
	:tolerence(tolerence)
{

}


const std::vector<Edge>& Connexity::readEdges() const
{
	return edges;
}

const std::vector<Face>& Connexity::readFaces() const
{
	return faces;
}

const std::vector<Vertex>& Connexity::readVertices() const
{
	return vertices;
}


index_t Connexity::addTriangle(const Triangle3 &t)
{
	index_t faceId = faces.size();

	index_t v0 = findOrCreateVertex(t[0]);
	index_t v1 = findOrCreateVertex(t[1]);
	index_t v2 = findOrCreateVertex(t[2]);

	Face face;
	face.edgeIndices[0] = findOrCreateEdge(v0, v1, faceId);
	face.edgeIndices[1] = findOrCreateEdge(v1, v2, faceId);
	face.edgeIndices[2] = findOrCreateEdge(v2, v0, faceId);

	face.vertexIndices[0] = v0;
	face.vertexIndices[1] = v1;
	face.vertexIndices[2] = v2;

	// Update list of neighboring faces
	faces.push_back(face);
	vertices[v0].faces.push_back(faceId);
	vertices[v1].faces.push_back(faceId);
	vertices[v2].faces.push_back(faceId);




	return faces.size() -1;
}


// given a face index, this method returns the cached
void Connexity::lookupIncidentFacesToFace(index_t faceId, int& face0, int& face1, int& face2) const
{
	const Face& face = faces[faceId];

	const Edge &e0 = edges[face.edgeIndices[0] ];
	const Edge &e1 = edges[face.edgeIndices[1] ];
	const Edge &e2 = edges[face.edgeIndices[2] ];

	face0 = e0.lookUpNeighbor(faceId);
	face1 = e1.lookUpNeighbor(faceId);
	face2 = e2.lookUpNeighbor(faceId);

}

void Connexity::fillEdgeList(Scalar z, std::list<index_t> & crossingEdges) const
{
	assert(crossingEdges.size() == 0);
	for (index_t i=0; i < edges.size(); i++)
	{
		const Edge &e= edges[i];
		index_t v0 = e.vertexIndices[0];
		index_t v1 = e.vertexIndices[1];

		const Vector3 &p0 = vertices[v0].point;
		const Vector3 &p1 = vertices[v1].point;

		Scalar min = p0.z;
		Scalar max = p1.z;
		if(min > max)
		{
			min = p1.z;
			max = p0.z;
		}
		// The z less or equal to max while z strictly larger than min
		// Prevents, in the author's opinion, the possibility of having
		// 2 edges for the same point. It is also better for the case of
		// a very flat 3d object with a height that is precisely equal to
		// the first layer height
		if ( (max-min > 0) && (z > min)  && (z <= max) )
		{
			crossingEdges.push_back(i);
		}
	}
}


void Connexity::dump(std::ostream& out) const
{
	out << "Slicy" << std::endl;
	out << "  vertices: coords and face list" << vertices.size() << std::endl;
	out << "  edges: " << edges.size() << std::endl;
	out << "  faces: " << faces.size() << std::endl;


	int x =0;
	for(std::vector<Vertex>::const_iterator i = vertices.begin(); i != vertices.end(); i++ )
	{
		x ++;
	}


	x =0;
	for(std::vector<Edge>::const_iterator i = edges.begin(); i != edges.end(); i++)
	{
		x ++;
	}
}



// finds 2 neighboring edges
std::pair<index_t, index_t> Connexity::edgeToEdges(index_t edgeIndex) const
{
	std::pair<index_t, index_t> ret;
	const Edge &startEdge = edges[edgeIndex];
	index_t faceIndex = startEdge.face0;
	const Face &face = faces[faceIndex];

	unsigned int it = 0;
	ret.first = face.edgeIndices[it];
	it++;
	if(ret.first == edgeIndex)
	{
		ret.first = face.edgeIndices[it];
		it++;
	}
	ret.second = face.edgeIndices[it];
	if(ret.second == edgeIndex)
	{
		it++;
		ret.second = face.edgeIndices[it];
	}
	return ret;
}


void Connexity::getAllNeighbors(index_t startFaceIndex, std::set<index_t>& allNeighbors) const
{
	const Face &face = faces[startFaceIndex];
	const std::vector<index_t>& neighbors0 = vertices[ face.vertexIndices[0]].faces;
	const std::vector<index_t>& neighbors1 = vertices[ face.vertexIndices[1]].faces;
	const std::vector<index_t>& neighbors2 = vertices[ face.vertexIndices[2]].faces;


	for(size_t i=0; i< neighbors0.size(); i++)
	{
		index_t faceId = neighbors0[i];
		if (faceId >=0  && faceId != startFaceIndex)
		{
			allNeighbors.insert(faceId);
		}
	}
	for(size_t i=0; i< neighbors1.size(); i++)
	{
		index_t faceId = neighbors1[i];
		if (faceId >=0  && faceId != startFaceIndex)
		{
			allNeighbors.insert(faceId);
		}
	}
	for(size_t i=0; i< neighbors2.size(); i++)
	{
		index_t faceId = neighbors2[i];
		if (faceId >=0  && faceId != startFaceIndex)
		{
			allNeighbors.insert(faceId);
		}
	}

}

index_t Connexity::cutNextFace(const std::list<index_t> &facesLeft,
						Scalar z,
						index_t startFaceIndex,
						LineSegment2& cut) const
{
	std::set<index_t> allNeighbors;
	getAllNeighbors(startFaceIndex, allNeighbors);

	for(std::set<index_t>::iterator i= allNeighbors.begin(); i != allNeighbors.end(); i++)
	{
		index_t faceIndex = *i;
		// use it only if its in the list of triangles left
		if(find(facesLeft.begin(), facesLeft.end(), faceIndex) != facesLeft.end())
		{
			const Face& face = faces[faceIndex];
			if( cutFace(z, face, cut))
			{
				return faceIndex;
			}
		}

	}

	return -1;
}


bool Connexity::cutFace(Scalar z, const Face &face, LineSegment2& cut) const
{


	const Vertex& v0 = vertices[face.vertexIndices[0]];
	const Vertex& v1 = vertices[face.vertexIndices[1]];
	const Vertex& v2 = vertices[face.vertexIndices[2]];

	Vector3 a(v0.point.x, v0.point.y, v0.point.z);
	Vector3 b(v1.point.x, v1.point.y, v1.point.z);
	Vector3 c(v2.point.x, v2.point.y, v2.point.z);
	Triangle3 triangle(a,b,c);

	bool success = triangle.cut( z, a, b);

	cut.a.x = a.x;
	cut.a.y = a.y;
	cut.b.x = b.x;
	cut.b.y = b.y;

	return success;
}


void Connexity::splitLoop(Scalar z, std::list<index_t> &facesLeft, std::list<LineSegment2> &loop) const
{
	assert(loop.size() == 0);
	assert(facesLeft.size() > 0);

	bool firstCutFound = false;
	LineSegment2 cut;

	index_t faceIndex;
	while (!firstCutFound)
	{
		if(facesLeft.size() ==0) return;

		faceIndex = *facesLeft.begin();
		facesLeft.remove(faceIndex);
		const Face &face = faces[faceIndex];
		firstCutFound = cutFace(z, face, cut);

	}

	// a cut is found
	loop.push_back(cut);

	bool loopEnd = false;
	while(!loopEnd)
	{
		faceIndex = *facesLeft.begin();
		facesLeft.remove(faceIndex);
		faceIndex = cutNextFace(facesLeft, z, faceIndex, cut);
		if(faceIndex >= 0)
		{
			facesLeft.remove(faceIndex);
			loop.push_back(cut);
		}
		else
		{
			loopEnd = true;
		}
		if(facesLeft.size() == 0)
		{
			loopEnd = true;
		}
	}
}


index_t Connexity::findOrCreateNewEdge(const Vector3 &coords0, const Vector3 &coords1, size_t face)
{
	index_t v0 = findOrCreateVertex(coords0);
	index_t v1 = findOrCreateVertex(coords1);
	findOrCreateEdge(v0, v1, face);
}

index_t Connexity::findOrCreateEdge(index_t v0, index_t v1, size_t face)
{

	Edge e(v0, v1, face);
	index_t edgeIndex;

	std::vector<Edge>::iterator it = find(edges.begin(), edges.end(), e);
	if(it == edges.end())
	{
		edges.push_back(e);
		edgeIndex = edges.size() -1;
	}
	else
	{
		it->connectFace(face);
		edgeIndex = std::distance(edges.begin(), it);
	}
	return edgeIndex;
}

index_t Connexity::findOrCreateVertex(const Vector3 &coords)
{
	return findOrCreateVertexIndex(vertices, coords, tolerence);
}


std::ostream& mgl::operator<<(std::ostream& os, const Vertex& v)
{
	os << " " << v.point << "\t[ ";
	for (size_t i=0; i< v.faces.size(); i++)
	{
		if (i>0)  os << ", ";
		os << v.faces[i];
	}
	os << "]";
	return os;
}

std::ostream& mgl::operator << (std::ostream &os, const Connexity &s)
{
	s.dump(os);
	return os;
}

