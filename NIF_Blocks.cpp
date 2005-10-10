#include "NIF_Blocks.h"
#include "nif_attrs.h"
#include "nif_math.h"
#include <cmath>
#include <sstream>

extern bool verbose;
extern int blocks_in_memory;

#define endl "\r\n"

extern string current_file;

/***********************************************************
 * ABlock methods
 **********************************************************/

ABlock::ABlock() : _ref_count(0), _block_num(-1) {
		//Temporary to test reference counting
		blocks_in_memory++;
	}

ABlock::~ABlock() {
	//Temporary to test reference counting
	//cout << "A block has commited suicide." << endl;
	blocks_in_memory--;

	// Delete all attributes
	for (unsigned int i = 0; i < _attr_vect.size(); ++i ) {
		delete _attr_vect[i].ptr();
	}
}

void ABlock::AddAttr( string type, string name ) {
	IAttr * attr;
	if ( type == "int" ) {
		attr = new IntAttr( name, this );
	} else if ( type == "short" ) {
		attr = new ShortAttr( name, this );
	} else if ( type == "byte" ) {
		attr = new ByteAttr( name, this );
	} else if ( type == "float" ) {
		attr = new FloatAttr( name, this );
	} else if ( type == "float3" ) {
		attr = new Float3Attr( name, this );
	} else if ( type == "string" ) {
		attr = new StringAttr( name, this );
	} else if ( type == "index" ) {
		attr = new IndexAttr( name, this );
	} else if ( type == "flags" ) {
		attr = new FlagsAttr( name, this );
	} else if ( type == "matrix" ) {
		attr = new MatrixAttr( name, this );
	} else if ( type == "properties" ) {
		attr = new GroupAttr( name, this );
	} else if ( type == "children" ) {
		attr = new GroupAttr( name, this );
	} else if ( type == "effects" ) {
		attr = new GroupAttr( name, this );
	} else if ( type == "bones" ) {
		attr = new BoneAttr( name, this );
	} else if ( type == "bbox" ) {
		attr = new BBoxAttr( name, this );
	} else if ( type == "cint" ) {
		attr = new CIntAttr( name, this );
	} else if ( type == "vertmode" ) {
		attr = new VertModeAttr( name, this );
	} else if ( type == "lightmode" ) {
		attr = new LightModeAttr( name, this );
	} else if ( type == "texture" ) {
		attr = new TextureAttr( name, this );
	} else if ( type == "bumpmap" ) {
		attr = new TextureAttr( name, this, true );
	} else if ( type == "applymode" ) {
		attr = new ApplyModeAttr( name, this );
	} else if ( type == "texsource" ) {
		attr = new TexSourceAttr( name, this );
	} else if ( type == "pixellayout" ) {
		attr = new PixelLayoutAttr( name, this );
	} else if ( type == "mipmapformat" ) {
		attr = new MipMapFormatAttr( name, this );
	} else if ( type == "alphaformat" ) {
		attr = new AlphaFormatAttr( name, this );
	} else if ( type == "parent" ) {
		attr = new ParentAttr( name, this );
	} else if ( type == "root" ) {
		attr = new RootAttr( name, this );
	} else {
		cout << type << endl;
		throw runtime_error("Unknown attribute type requested.");
	}

	_attr_map[name] = attr_ref(attr);
	_attr_vect.push_back(attr_ref(attr));
}

attr_ref ABlock::GetAttr(string attr_name) {
	map<string, attr_ref>::iterator it;
	it = _attr_map.find(attr_name);
	if (it == _attr_map.end()) {
		return attr_ref(NULL);
	} else {
		return attr_ref((*it).second);
	}
	//return _attr_map[attr_name]; 
}

attr_ref ABlock::GetAttr(int index) {
	return attr_ref(_attr_vect[index]);
}

int ABlock::AttrCount() {
	return int(_attr_vect.size());
}

blk_ref ABlock::GetParent() {
	if (_parents.size() > 0 )
		return blk_ref(_parents[0]);
	else
		return blk_ref(-1);
}

void ABlock::Read( ifstream& in ) {
	for (unsigned int i = 0; i < _attr_vect.size(); ++i ) {
		_attr_vect[i]->Read( in );
	}
}

void ABlock::Write( ofstream& out ) {
	//Write Block Type
	WriteString( this->GetBlockType() , out );

	//Write Attributes
	for (unsigned int i = 0; i < _attr_vect.size(); ++i ) {
		_attr_vect[i]->Write( out );
	}
}

string ABlock::asString() {
	stringstream out;
	out.setf(ios::fixed, ios::floatfield);
	out << setprecision(1);

	out.setf(ios::fixed, ios::floatfield);
	out << setprecision(1);

	out << "Parent:  " << GetParent() << endl;
	for (unsigned int i = 0; i < _attr_vect.size(); ++i ) {
		out << _attr_vect[i]->GetName() << ":  " << _attr_vect[i]->asString() << endl;
	}

	//if ( _parents.size() > 1 ) {
	//	cout << endl  << "Parents:";
	//	for (unsigned int i = 0; i < _parents.size(); ++i ) {
	//		cout << "   " << _parents[i]->GetBlockType();
	//	}
	//	cout << endl << "Node:  " << GetBlockType() << "\a";
	//	cin.get();
	//}

	return out.str();
}

void ABlock::AddRef() {
	++_ref_count;
	//cout << GetBlockType() << " Reference increased to: " << _ref_count << endl;
}

void ABlock::SubtractRef() {
	--_ref_count;
	//cout << GetBlockType() << " Reference decreased to: " << _ref_count << endl;

	if ( _ref_count < 1 ) {
		delete this;
	}
}

list<blk_ref> ABlock::GetLinks() {
	list<blk_ref> links;

	//Search through all attributes for any links and add them to the list
	vector<attr_ref>::iterator it;
	for ( it = _attr_vect.begin(); it != _attr_vect.end(); ++it ) {
		if ( (*it)->HasLinks() == true ) {
			links.merge( list<blk_ref>(*it) );
		}
	}

	//Remove NULL links
	links.remove( blk_ref(-1) );

	return links;
}

void ABlock::FixUpLinks( const vector<blk_ref> & blocks ) {
	//Search through all attributes for any links and fix their references based on the list
	vector<attr_ref>::iterator it;
	for ( it = _attr_vect.begin(); it != _attr_vect.end(); ++it ) {
		if ( (*it)->HasLinks() == true ) {
			//Get the links out of this attribute and fix each one
			list<blk_ref> links = *it;
			list<blk_ref>::iterator it2;
			for (it2 = links.begin(); it2 != links.end(); ++it2) {
				int index = it2->get_index();
				if (index < int(blocks.size()) && index >= 0 ) {
					*it2 = blocks[index];
				}
			}
			//Now clear the old links and send in the new ones
			(*it)->ClearLinks();
			(*it)->AddLinks(links);
		}
	}
}

//-- Internal Functions --//

void ABlock::AddParent( blk_ref parent) { 
	//Don't add null parents
	if (parent.get_block() != NULL)
		_parents.push_back( parent.get_block() );
}

void ABlock::RemoveParent( IBlock * match ) {
	//Remove just one copy of the parent if there is one, incase a temporary reference is floating around
	vector<IBlock*>::iterator it = find<vector<IBlock*>::iterator, IBlock*>( _parents.begin(), _parents.end(), match);
	if (it != _parents.end() ) {
		_parents.erase( it );
	}

	/*cout << blk_ref(this) << " Parents Remaining:" << endl << "   ";
	for ( it = _parents.begin(); it != _parents.end(); ++it ) {
		cout << blk_ref(*it) << "  ";
	}
	cout << endl;*/

		
	//for (it = _parents.begin(); it != _parents.end(); ) {
	//	if ( *it == match )
	//		_parents.erase( it );
	//	else
	//		++it;
	//}
}

/***********************************************************
 * ANode methods
 **********************************************************/

void * ANode::QueryInterface( int id ) {
	// Contains INode Interface
	if ( id == Node ) {
		return (void*)static_cast<INode*>(this);
	} else {
		return ABlock::QueryInterface( id );
	}
}

void ANode::GetLocalTransform( float out_matrix[4][4] ) {
	//Get transform data from atributes
	float3 tran;
	matrix f;
	GetAttr("Rotation")->asMatrix( f );
	GetAttr("Translation")->asFloat3( tran );
	float scale = GetAttr("Scale")->asFloat();

	//Set up a matrix with rotate and translate information
	float rt[4][4];
	rt[0][0] = f[0][0];	rt[0][1] = f[0][1];	rt[0][2] = f[0][2];	rt[0][3] = 0.0f;
	rt[1][0] = f[1][0];	rt[1][1] = f[1][1];	rt[1][2] = f[1][2];	rt[1][3] = 0.0f;
	rt[2][0] = f[2][0];	rt[2][1] = f[2][1];	rt[2][2] = f[2][2];	rt[2][3] = 0.0f;
	rt[3][0] = tran[0];	rt[3][1] = tran[1];	rt[3][2] = tran[2];	rt[3][3] = 1.0f;

	//Set up another matrix with the scale information
	float s[4][4];
	s[0][0] = scale;	s[0][1] = 0.0f;		s[0][2] = 0.0f;		s[0][3] = 0.0f;
	s[1][0] = 0.0f;		s[1][1] = scale;	s[1][2] = 0.0f;		s[1][3] = 0.0f;
	s[2][0] = 0.0f;		s[2][1] = 0.0f;		s[2][2] = scale;	s[2][3] = 0.0f;
	s[3][0] = 0.0f;		s[3][1] = 0.0f;		s[3][2] = 0.0f;		s[3][3] = 1.0f;

	//Multiply the two for the resulting local transform
	MultMatrix44(rt, s, out_matrix);
}

void ANode::GetWorldTransform( float out_matrix[4][4] ) {
	//Get Parent Transform if there is one
	blk_ref par = GetParent();
	INode * node;
	if ( par.is_null() == false && ( node = (INode*)par->QueryInterface(Node) ) != NULL) {
		//Get Local Transform
		float local[4][4];
		GetLocalTransform( local );

		//Get Parent World Transform
		float par_world[4][4];
		node->GetWorldTransform( par_world );

		//Multipy local matrix parent world matrix for result
		MultMatrix44( par_world, local, out_matrix );
	}
	else {
		//No parent transform, simply return local transform
		GetLocalTransform( out_matrix );
	}
}

void ANode::GetBindPosition( float out_matrix[4][4] ) {
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			out_matrix[i][j] = bindPosition[i][j];
		}
	}
}

void ANode::GetLocalBindPos( float out_matrix[4][4] ) {
	//Get Parent Transform if there is one
	blk_ref par = GetParent();
	INode * node;
	if ( par.is_null() == false && ( node = (INode*)par->QueryInterface(Node) ) != NULL) {
		//There is a node parent
		//multiply its inverse with this block's bind position to get the local bind position

		float par_mat[4][4];
		node->GetBindPosition( par_mat );
		float par_inv[4][4];
		InverseMatrix44( par_mat, par_inv );
		
		MultMatrix44( bindPosition, par_inv, out_matrix );
	}
	else {
		//No parent transform, simply return local transform
		GetBindPosition( out_matrix );
	}
}

void ANode::SetBindPosition( float in_matrix[4][4] ) {
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			bindPosition[i][j] = in_matrix[i][j];
		}
	}
}

/***********************************************************
 * NiTriShapeData methods
 **********************************************************/

/**
 * NiTriShapeData::Read - Assumes block name has already been read from in
 */
void NiTriShapeData::Read( ifstream& in ){

	short vert_count = ReadUShort( in );
	int hasVertices = ReadUInt( in );
	if ( hasVertices != 0 ){
		vertices.resize( vert_count );
		for ( uint i = 0; i < vertices.size(); ++i ){
			vertices[i].x = ReadFloat( in );
			vertices[i].y = ReadFloat( in );
			vertices[i].z = ReadFloat( in );
		}
	}

	int hasNormals = ReadUInt( in );
	if ( hasNormals != 0 ){
		normals.resize( vert_count );
		for ( uint i = 0; i < normals.size(); ++i ){
			normals[i].x = ReadFloat( in );
			normals[i].y = ReadFloat( in );
			normals[i].z = ReadFloat( in );
		}
	}

	GetAttr("Center")->Read( in );
	GetAttr("Radius")->Read( in );

	int hasVertexColors = ReadUInt( in );
	if ( hasVertexColors != 0 ){
		colors.resize( vert_count );
		for ( uint i = 0; i < colors.size(); ++i ){
			colors[i].r = ReadFloat( in );
			colors[i].g = ReadFloat( in );
			colors[i].b = ReadFloat( in );
			colors[i].a = ReadFloat( in );
		}
	}

	short numTexSets = ReadUShort( in );
	int hasUVs = ReadUInt( in );
	if ( numTexSets > 0 && hasUVs != 0 ){
		uv_sets.resize( numTexSets );
		//UVs = new fVector2[numVertices * numTexSets];
		for ( uint i = 0; i < uv_sets.size(); ++i ){
			uv_sets[i].resize( vert_count );
			for ( uint j = 0; j < uv_sets[i].size(); ++j){
				uv_sets[i][j].u = ReadFloat( in );
				uv_sets[i][j].v = ReadFloat( in );
			}
		}
	}

	short numTriangles = ReadUShort( in );
	if (numTriangles > 0) {
		int numVertexIndices = ReadUInt( in );
		triangles.resize( numTriangles );
		for ( int i = 0; i < numTriangles; ++i ){
			triangles[i].v1 = ReadUShort( in );
			triangles[i].v2 = ReadUShort( in );
			triangles[i].v3 = ReadUShort( in );
		}
	}

	short matchGroupCount = ReadUShort( in );
	match_group_mode = ( matchGroupCount != 0 );  // Only record whether or not file prefers to have match data generated

	short sub_count;
	for ( int i = 0; i < matchGroupCount; ++i ){
		sub_count = ReadUShort( in );
		for (ushort j = 0; j < sub_count; ++j) {
			ReadUShort( in );  // Read data, but don't care what it is
		}
	}
}

string NiTriShapeData::asString() {
	stringstream out;
	out.setf(ios::fixed, ios::floatfield);
	out << setprecision(1);

	out << "Vertices:  " << uint(vertices.size());
	if (verbose) {
		out << "   ";
		for ( uint i = 0; i < vertices.size(); ++i) {
			if (i % 3 == 0)
				out << endl << "   ";
			else
				out << "  ";

			out << "(" << setw(5) << vertices[i].x << ", " << setw(5) << vertices[i].y << ", " << setw(5) << vertices[i].z << " )";
		}
	} else {
		out << endl << "<<Data Not Shown>>";
	}
	out << endl;

	out << "Normals:  " << uint(normals.size());
	if (verbose) {
		out << "   ";
		for ( uint i = 0; i < normals.size(); ++i) {
			if (i % 3 == 0)
				out << endl << "   ";
			else
				out << "  ";

			out << "(" << setw(5) << normals[i].x << ", " << setw(5) << normals[i].y << ", " << setw(5) << normals[i].z << " )";
		}
	} else {
		out << endl << "<<Data Not Shown>>";
	}
	out << endl;
	
	attr_ref attr = GetAttr("Center");
	out << attr->GetName() << ":  " << attr->asString() << endl;
	attr = GetAttr("Radius");
	out << attr->GetName() << ":  " << attr->asString() << endl;

	out << "Vertex Colors:  " << uint(colors.size());
	if (verbose) {
		out << "   ";
		for ( uint i = 0; i < colors.size(); ++i) {
			if (i % 3 == 0)
				out << endl << "   ";
			else
				out << "  ";
			
			out << "(" << setw(5) << colors[i].r << ", " << setw(5) << colors[i].g << ", " << setw(5) << colors[i].b << ", " << setw(5) << colors[i].a << " )";
		}
	} else {
		out << endl << "<<Data Not Shown>>";
	}
	out << endl;

	out << "Texture Coordinate Sets:  " << uint(uv_sets.size());
	if (verbose) {
		for ( uint i = 0; i < uv_sets.size(); ++i) {
			out << endl 
				<< "   UV Set " << i+1 << ":";

			for ( uint j = 0; j < uv_sets[i].size(); ++j) {
				if (j % 3 == 0)
					out << endl << "      ";
				else
					out << "  ";

				out << "(" << setw(5) << uv_sets[i][j].u << ", " << setw(5) << uv_sets[i][j].v << " )";
			}
		}
	} else {
		out << endl << "<<Data Not Shown>>";
	}
	out << endl;

	out << "Triangles:  " << uint(triangles.size());
	if (verbose) {
		for ( uint i = 0; i < triangles.size(); ++i) {
			if (i % 3 == 0)
				out << endl << "   ";
			else
				out << "  ";
			
			out << "(" << setw(5) << triangles[i].v1 << ", " << setw(5) << triangles[i].v2 << ", " << setw(5) << triangles[i].v3 << " )";
		}
	} else {
		out << endl << "<<Data Not Shown>>";
	}
	out << endl;

	out << "Match Detection:  ";
	if ( match_group_mode )
		out << "On" << endl;
	else
		out << "Off" << endl;	

	return out.str();
}

/**
 * NiTriShapeData::Write - Writes block name to out, in addition to data.
 */
void NiTriShapeData::Write( ofstream& out ){

	WriteString( "NiTriShapeData", out );

	WriteUShort( short(vertices.size()), out );

	if ( vertices.size() > 0 )
		WriteUInt( 1, out );
	else
		WriteUInt( 0, out );

	for ( uint i = 0; i < vertices.size(); ++i ){
		WriteFloat( vertices[i].x, out );
		WriteFloat( vertices[i].y, out );
		WriteFloat( vertices[i].z, out );
	}

	if ( normals.size() > 0 )
		WriteUInt( 1, out );
	else
		WriteUInt( 0, out );

	for ( uint i = 0; i < normals.size(); ++i) {
		WriteFloat( normals[i].x, out );
		WriteFloat( normals[i].y, out );
		WriteFloat( normals[i].z, out );
	}

	GetAttr("Center")->Write( out );
	GetAttr("Radius")->Write( out );

	if ( colors.size() > 0 )
		WriteUInt( 1, out );
	else
		WriteUInt( 0, out );

	for ( uint i = 0; i < colors.size(); ++i ){
		WriteFloat( colors[i].r, out );
		WriteFloat( colors[i].g, out );
		WriteFloat( colors[i].b, out );
		WriteFloat( colors[i].a, out );
	}

	WriteUShort( ushort(uv_sets.size()), out );
	if ( uv_sets.size() > 0 && uv_sets[0].size() > 0 ) // hasUVs
		WriteUInt( 1, out );
	else
		WriteUInt( 0, out );

	for ( uint i = 0; i < uv_sets.size(); ++i) {
		for ( uint j = 0; j < uv_sets[i].size(); ++j) {
			WriteFloat( uv_sets[i][j].u, out );
			WriteFloat( uv_sets[i][j].v, out );
		}
	}

	WriteUShort( ushort(triangles.size()), out );

	if ( triangles.size() > 0 ) {
		//Write number of shorts:  triCount * 3
		WriteUInt( ushort(triangles.size()) * 3, out );

		for ( uint i = 0; i < triangles.size(); ++i ){
			WriteUShort( triangles[i].v1, out );
			WriteUShort( triangles[i].v2, out );
			WriteUShort( triangles[i].v3, out );
		}
	}

	

	if ( match_group_mode ) {
		WriteUShort( ushort(vertices.size()), out ); //Match Group Count = Vertex Count

		vector<ushort> matches;
		for ( uint i = 0; i < vertices.size(); ++i ){
			// Find all vertices that match this one.
			matches.clear();
			for (ushort j = 0; j < vertices.size(); ++j) {
				if ( i != j && vertices[i].x == vertices[j].x && vertices[i].y == vertices[j].y && vertices[i].z == vertices[j].z ) {
					matches.push_back(j);
				}
			}
			//Match Count
			WriteUShort( ushort(matches.size()) , out );

			//Output Vertex indicies
			for (ushort j = 0; j < matches.size(); ++j) {
				WriteUShort( matches[j], out );
			}
		}	
	} else {
		WriteUShort( 0, out ); //Match Group Count = 0
	}
}

void * NiTriShapeData::QueryInterface( int id ) {
	// Contains TriShapeData Interface
	if ( id == TriShapeData ) {
		return (void*)static_cast<ITriShapeData*>(this);
	} else {
		return ABlock::QueryInterface( id );
	}
}

void NiTriShapeData::SetVertexCount(int n) {
	vertices.resize(n);
	normals.resize(n);
	colors.resize(n);
	for (uint i = 0; i < uv_sets.size(); ++i) {
		uv_sets[i].resize(n);
	}
}

void NiTriShapeData::SetUVSetCount(int n) {
	uv_sets.resize(n);
}

void NiTriShapeData::SetTriangleCount(int n) {
	triangles.resize(n);
}

//--Setters--//
void NiTriShapeData::SetVertices( const vector<Vector3D> & in ) {
	if (in.size() != vertices.size())
		throw runtime_error("Input array size must equal Vertex Count.  Call SetVertexCount() to resize.");
	vertices = in;
}

void NiTriShapeData::SetNormals( const vector<Vector3D> & in ) {
	if (in.size() != vertices.size())
		throw runtime_error("Input array size must equal Vertex Count.  Call SetVertexCount() to resize.");
	normals = in;
}

void NiTriShapeData::SetColors( const vector<Color> & in ) {
	if (in.size() != vertices.size())
		throw runtime_error("Vector size must equal Vertex Count.  Call SetVertexCount() to resize.");
	colors = in;
}

void NiTriShapeData::SetUVSet( int index, const vector<UVCoord> & in ) {
	if (in.size() != vertices.size())
		throw runtime_error("Vector size must equal Vertex Count.  Call SetVertexCount() to resize.");
	uv_sets[index] = in;
}

void NiTriShapeData::SetTriangles( const vector<Triangle> & in ) {
	if (in.size() != vertices.size())
		throw runtime_error("Vector size must equal Vertex Count.  Call SetVertexCount() to resize.");
	triangles = in;
}

/***********************************************************
 * NiSkinData methods
 **********************************************************/

void NiSkinData::Read( ifstream& in ) {
	
	for (int c = 0; c < 3; ++c) {
		for (int r = 0; r < 3; ++r) {
			rotation[r][c] = ReadFloat( in );
		}
	}
	ReadFVector3( translation, in );
	scale = ReadFloat( in );
	int boneCount = ReadUInt( in );

	unknown.Read( in );

	bones.resize(boneCount);
	for( int i = 0; i < boneCount; i++ ) {
		for (int c = 0; c < 3; ++c) {
			for (int r = 0; r < 3; ++r) {
				bones[i].rotation[r][c] = ReadFloat( in );
			}
		}
		ReadFVector3( bones[i].translation, in );
		bones[i].scale = ReadFloat( in );

		ReadFVector4( bones[i].unknown4Floats, in );
		short numWeights = ReadUShort( in );
		bones[i].weights.clear();
		for ( int j = 0; j < numWeights; ++j ){
			short vertexNum = ReadUShort( in );
			float vertexWeight = ReadFloat( in );
			bones[i].weights[vertexNum] = vertexWeight;
		}
	}
}

void NiSkinData::Write( ofstream& out ) {
	WriteString( "NiSkinData", out );

	for (int c = 0; c < 3; ++c) {
		for (int r = 0; r < 3; ++r) {
			WriteFloat( rotation[r][c], out );
		}
	}
	WriteFVector3( translation, out );
	WriteFloat( scale, out );
	WriteUInt(short(bones.size()), out);
	unknown.Write( out );

	map<blk_ref, Bone>::iterator it;
	for( it = bone_map.begin(); it != bone_map.end(); ++it ) {
		for (int c = 0; c < 3; ++c) {
			for (int r = 0; r < 3; ++r) {
				WriteFloat( it->second.rotation[r][c], out );
			}
		}
		WriteFVector3( it->second.translation, out );
		WriteFloat( it->second.scale, out );

		WriteFVector4( it->second.unknown4Floats, out );
		WriteUShort( short(it->second.weights.size() ), out );
		
		map<int, float>::iterator it2;
		for ( it2 = it->second.weights.begin(); it2 != it->second.weights.end(); ++it2 ){
			WriteUShort( it2->first, out );
			WriteFloat( it2->second, out );
		}
	}
}

//void GetBuiltUpTransform(blk_ref block, Matrix & m/*, blk_ref stop*/) {
//	matrix temp;
//	float3 t2;
//
//	// Do multiplication
//	attr_ref rot_attr = block["Rotation"];
//	attr_ref trn_attr = block["Translation"];
//	attr_ref scl_attr = block["Scale"];
//
//	if ( rot_attr.is_null() == false && trn_attr.is_null() == false && scl_attr.is_null() == false ) {
//		rot_attr->asMatrix(temp);
//		trn_attr->asFloat3(t2);
//		float scale = scl_attr->asFloat();
//
//		Matrix tr = Matrix( temp[0][0], temp[0][1], temp[0][2], 0.0f,
//							temp[1][0], temp[1][1], temp[1][2], 0.0f,
//							temp[2][0], temp[2][1], temp[2][2], 0.0f,
//							t2[0], t2[1], t2[2], 1.0f);
//
//		Matrix s = Matrix( scale, 0.0f, 0.0f, 0.0f,
//							0.0f, scale, 0.0f, 0.0f,
//							0.0f, 0.0f, scale, 0.0f,
//							0.0f, 0.0f, 0.0f, 1.0f );
//
//		m = m * (tr * s);
//	}
//	else {
//		throw runtime_error("GetBuiltUpTranslations attempted to access a block without rotation, translation, and scale");
//	}
//
//	//Check if there are any parents
//	blk_ref par = block->GetParent();
//	if ( par.is_null() == false ) {
//		//There is a parent, re-call this function on it
//		GetBuiltUpTransform( par, m/*, stop*/ );
//	}
//}

string NiSkinData::asString() {
	stringstream out;
	out.setf(ios::fixed, ios::floatfield);
	out << setprecision(1);

	//out //<< setprecision(6)
	//	<< "Rotate:" << endl
	//	<< "   |" << setw(6) << rotation[0][0] << "," << setw(6) << rotation[0][1] << "," << setw(6) << rotation[0][2] << " |" << endl
	//	<< "   |" << setw(6) << rotation[1][0] << "," << setw(6) << rotation[1][1] << "," << setw(6) << rotation[1][2] << " |" << endl
	//	<< "   |" << setw(6) << rotation[2][0] << "," << setw(6) << rotation[2][1] << "," << setw(6) << rotation[2][2] << " |" << endl
	//	<< "Translate:  " << translation << endl
	//	<< "Scale:  " << scale << endl
	//	<< "Bone Count:  " << uint(bone_map.size()) << endl
	//	//<< setprecision(3)
	//	<< "Unknown Index:  " << unknown << endl
	//	<< "Bones:" << endl;

	map<blk_ref, Bone>::iterator it;
	int num = 0;
	for( it = bone_map.begin(); it != bone_map.end(); ++it ) {
		//Friendlier name
		Bone & bone = it->second;

		num++;
		out << "Bone " << num << ":" << endl
			<< "   Block:  " << it->first << endl
			<< "   Bone Offset Transforms:" << endl
			<< "      Rotation:" << endl
			<< "         |" << setw(6) << bone.rotation[0][0] << "," << setw(6) << bone.rotation[0][1] << "," << setw(6) << bone.rotation[0][2] << " |" << endl
			<< "         |" << setw(6) << bone.rotation[1][0] << "," << setw(6) << bone.rotation[1][1] << "," << setw(6) << bone.rotation[1][2] << " |" << endl
			<< "         |" << setw(6) << bone.rotation[2][0] << "," << setw(6) << bone.rotation[2][1] << "," << setw(6) << bone.rotation[2][2] << " |" << endl
			<< "      Translate:  " << bone.translation << endl
			<< "      Scale:  " << bone.scale << endl;

		//float pi = 3.141592653589793f;

		//out << "      Euler Angles:" << endl
		//	<< "         X:  " << atan2( bone.rotation[1][2], bone.rotation[2][2] ) / pi * 180.0 << endl
		//	<< "         Y:  " << asin( -bone.rotation[0][2] ) / pi * 180.0 << endl
		//	<< "         Z:  " << atan2( bone.rotation[0][1], bone.rotation[0][0] ) / pi * 180.0 << endl;

		//if ( bone_blks.size() > 0 ) {
		//	blk_ref start_blk = GetParent()->GetParent()->GetParent();
		//	//cout << "TriShape Parent:  " << parent << endl;
		//	Matrix start_mat = IdentityMatrix();
		//	GetBuiltUpTransform( start_blk, start_mat );

		//	Matrix end_mat( bone.rotation[0][0], bone.rotation[0][1], bone.rotation[0][2], 0.0f,
		//					 bone.rotation[1][0], bone.rotation[1][1], bone.rotation[1][2], 0.0f,
		//					 bone.rotation[2][0], bone.rotation[2][1], bone.rotation[2][2], 0.0f,
		//					 bone.translation[0], bone.translation[1], bone.translation[2], 1.0f);
		//	
		//	//Get built up rotations to the root of the skeleton from this bone
		//	GetBuiltUpTransform( bone_blks[i], end_mat );

		//	out << "Start:" << endl;
		//	out << "      |" << setw(6) << start_mat(0,0) << "," << setw(6) << start_mat(0,1) << "," << setw(6) << start_mat(0,2) << "," << setw(6) << start_mat(0,3) << " |" << endl
		//		<< "      |" << setw(6) << start_mat(1,0) << "," << setw(6) << start_mat(1,1) << "," << setw(6) << start_mat(1,2) << "," << setw(6) << start_mat(1,3) << " |" << endl
		//		<< "      |" << setw(6) << start_mat(2,0) << "," << setw(6) << start_mat(2,1) << "," << setw(6) << start_mat(2,2) << "," << setw(6) << start_mat(2,3) << " |" << endl
		//		<< "      |" << setw(6) << start_mat(3,0) << "," << setw(6) << start_mat(3,1) << "," << setw(6) << start_mat(3,2) << "," << setw(6) << start_mat(3,3) << " |" << endl;

		//	out << "Euler Angles:" << endl
		//		<< "      X:  " << atan2( start_mat(1,2), start_mat(2,2) ) / pi * 180.0 << endl
		//		<< "      Y:  " << asin( -start_mat(0,2) ) / pi * 180.0 << endl
		//		<< "      Z:  " << atan2( start_mat(0,1), start_mat(0,0) ) / pi * 180.0 << endl;

		//	out << "End:" << endl;
		//	out << "      |" << setw(6) << end_mat(0,0) << "," << setw(6) << end_mat(0,1) << "," << setw(6) << end_mat(0,2) << "," << setw(6) << end_mat(0,3) << " |" << endl
		//		<< "      |" << setw(6) << end_mat(1,0) << "," << setw(6) << end_mat(1,1) << "," << setw(6) << end_mat(1,2) << "," << setw(6) << end_mat(1,3) << " |" << endl
		//		<< "      |" << setw(6) << end_mat(2,0) << "," << setw(6) << end_mat(2,1) << "," << setw(6) << end_mat(2,2) << "," << setw(6) << end_mat(2,3) << " |" << endl
		//		<< "      |" << setw(6) << end_mat(3,0) << "," << setw(6) << end_mat(3,1) << "," << setw(6) << end_mat(3,2) << "," << setw(6) << end_mat(3,3) << " |" << endl;

		//	out << "Euler Angles:" << endl
		//		<< "      X:  " << atan2( end_mat(1,2), end_mat(2,2) ) / pi * 180.0 << endl
		//		<< "      Y:  " << asin( -end_mat(0,2) ) / pi * 180.0 << endl
		//		<< "      Z:  " << atan2( end_mat(0,1), end_mat(0,0) ) / pi * 180.0 << endl;
		//	
		//	Matrix built_up = end_mat * start_mat.inverse();

		//	//blk_ref cur = bone_blks[i];
		//	//matrix temp;
		//	//float3 t2;
		//	//float scale;
		//	//attr_ref rot_attr;
		//	//attr_ref trn_attr;
		//	//attr_ref scl_attr;
		//	//while ( cur.is_null() == false && cur != parent ) {
		//	//	rot_attr = cur->GetAttr("Rotation");
		//	//	trn_attr = cur->GetAttr("Translation");
		//	//	scl_attr = cur->GetAttr("Scale");

		//	//	if ( rot_attr != NULL && trn_attr != NULL && scl_attr != NULL ) {
		//	//		rot_attr->asMatrix(temp);
		//	//		trn_attr->asFloat3(t2);
		//	//		scale = scl_attr->asFloat();

		//	//		Matrix tr = Matrix( temp[0][0], temp[0][1], temp[0][2], 0.0f,
		//	//					temp[1][0], temp[1][1], temp[1][2], 0.0f,
		//	//					temp[2][0], temp[2][1], temp[2][2], 0.0f,
		//	//					t2[0], t2[1], t2[2], 1.0f);

		//	//		Matrix s = Matrix( scale, 0.0f, 0.0f, 0.0f,
		//	//							0.0f, scale, 0.0f, 0.0f,
		//	//							0.0f, 0.0f, scale, 0.0f,
		//	//							0.0f, 0.0f, 0.0f, 1.0f );

		//	//		built_up = built_up * (tr * s);
		//	//	}

		//	//	cur = cur->GetParent();
		//	//}
		//	
		//	out << "      Difference from current position?:" << endl;
		//	out << "      |" << setw(6) << built_up(0,0) << "," << setw(6) << built_up(0,1) << "," << setw(6) << built_up(0,2) << "," << setw(6) << built_up(0,3) << " |" << endl
		//		<< "      |" << setw(6) << built_up(1,0) << "," << setw(6) << built_up(1,1) << "," << setw(6) << built_up(1,2) << "," << setw(6) << built_up(1,3) << " |" << endl
		//		<< "      |" << setw(6) << built_up(2,0) << "," << setw(6) << built_up(2,1) << "," << setw(6) << built_up(2,2) << "," << setw(6) << built_up(2,3) << " |" << endl
		//		<< "      |" << setw(6) << built_up(3,0) << "," << setw(6) << built_up(3,1) << "," << setw(6) << built_up(3,2) << "," << setw(6) << built_up(3,3) << " |" << endl;

		//	out << "Euler Angles:" << endl
		//		<< "      X:  " << atan2( built_up(1,2), built_up(2,2) ) / pi * 180.0 << endl
		//		<< "      Y:  " << asin( -built_up(0,2) ) / pi * 180.0 << endl
		//		<< "      Z:  " << atan2( built_up(0,1), built_up(0,0) ) / pi * 180.0 << endl;
		//}
		//
		float q[4] = {  bone.unknown4Floats[0],
					    bone.unknown4Floats[1],
					    bone.unknown4Floats[2],
					    bone.unknown4Floats[3] };

		//////Normalize Quaternion
		////float mag = sqrt(q[0] * q[0] + q[1] * q[1] + q[2] * q[2] + q[3] * q[3]);
		////q[0] /= mag;
		////q[1] /= mag;
		////q[2] /= mag;
		////q[3] /= mag;

		out << "Unknown 4 Floats:  " << setw(6) << q[0] << "," << setw(6) << q[1] << "," << setw(6) << q[2] << "," << setw(6) << q[3] << endl;
		//	<< "      As Matrix:";
		//
		//QuatToMatrix(q, out);
		//out << "      As Angles:";
		////QuatToEuler(quat, out);

		out << "   Weights:  " << uint(bone.weights.size()) << endl;

		if (verbose) {
			map<int, float>::iterator it2;
			for ( it2 = bone.weights.begin(); it2 != bone.weights.end(); ++it2 ){
				out << "   Vertex: " << it2->first << "\tWeight: " << it2->second << endl;
			}
		} else {
			out << "      <<Data Not Shown>>" << endl;
		}
	}
	
	//out << setprecision(1);

	return out.str();
}

void * NiSkinData::QueryInterface( int id ) {
	// Contains ISkinData Interface
	if ( id == SkinData ) {
		return (void*)static_cast<ISkinData*>(this);
	} else if ( id == SkinDataInternal ) {
		return (void*)static_cast<ISkinDataInternal*>(this);
	} else {
		return ABlock::QueryInterface( id );
	}
}

void NiSkinData::SetBones( vector<blk_ref> bone_blocks ) {
	//Move bones from temproary vector to map, sorted by blk_ref
	for (uint i = 0; i < bones.size(); ++i) {
		bone_map.insert( pair<blk_ref, Bone>(bone_blocks[i], bones[i]) );
	}
	bones.clear();	
}

void NiSkinData::StraightenSkeleton() {
	////Get bone blocks from parent SkinInstance
	//vector<blk_ref> bone_blks;
	//blk_ref par = GetParent();
	//blk_link l;
	//attr_ref bone_attr = par->GetAttr("Bones");

	//if ( bone_attr.is_null() == false ) {
	//	for (int i = 0; i < par->LinkCount(); ++i) {
	//		l = par->GetLink(i);
	//		if ( l.attr == bone_attr ) {
	//			bone_blks.push_back(l.block);
	//		}
	//	}
	//}

	//Loop through all bones
	map<blk_ref, Bone>::iterator it;
	for ( it = bone_map.begin(); it != bone_map.end(); ++it ) {
		//Friendlier name for current bone
		Bone & bone = it->second;

		//Get current offset matrix for this bone
		float parent_offset[4][4] = {
			bone.rotation[0][0], bone.rotation[0][1], bone.rotation[0][2], 0.0f,
			bone.rotation[1][0], bone.rotation[1][1], bone.rotation[1][2], 0.0f,
			bone.rotation[2][0], bone.rotation[2][1], bone.rotation[2][2], 0.0f,
			bone.translation[0], bone.translation[1], bone.translation[2], 1.0f
		}; 
		//Loop through all bones again, checking for any that have this bone as a parent
		map<blk_ref, Bone>::iterator it2;
		for ( it2 = bone_map.begin(); it2 != bone_map.end(); ++it2 ) {
			if ( it2->first->GetParent() == it->first ) {
				//Block 2 has block 1 as a parent

				Bone & bone2 = it2->second;

				//Get child offset matrix
				float child_offset[4][4] = {
					bone2.rotation[0][0], bone2.rotation[0][1], bone2.rotation[0][2], 0.0f,
					bone2.rotation[1][0], bone2.rotation[1][1], bone2.rotation[1][2], 0.0f,
					bone2.rotation[2][0], bone2.rotation[2][1], bone2.rotation[2][2], 0.0f,
					bone2.translation[0], bone2.translation[1], bone2.translation[2], 1.0f
				};

				//Do calculation to get correct bone postion in relation to parent
				float inverse_co[4][4];
				InverseMatrix44(child_offset, inverse_co);
				float child_pos[4][4];
				MultMatrix44( inverse_co, parent_offset, child_pos);
				//Matrix child_pos = child_offset.inverse() * parent_offset;

				//Store result in block's Bind Position Matrix
				INode * node = (INode*)it2->first->QueryInterface(Node);
				if (node != NULL) {
					node->SetBindPosition(child_pos);
				}

    //            //Store result in child block
				//matrix rotate = { 
				//	child_pos[0][0], child_pos[0][1], child_pos[0][2],
				//	child_pos[1][0], child_pos[1][1], child_pos[1][2],
				//	child_pos[2][0], child_pos[2][1], child_pos[2][2]
				//};
				//it2->first->GetAttr("Rotation")->Set( rotate );
				//it2->first->GetAttr("Translation")->Set( child_pos[3][0], child_pos[3][1], child_pos[3][2] );
			}
		}
	}
}

void NiSkinData::RepositionTriShape() {
	//Get block we're going to move
	blk_ref tri_shape = GetParent()->GetParent();

	//There must be at least one bone to do anything
	if ( bone_map.size() > 0 ) {
		//--Start Position--//
		//Matrix start_mat = IdentityMatrix();
		//GetBuiltUpTransform( tri_shape->GetParent(), start_mat );

		//--End Position--//

		//Get first bone
		blk_ref bone_blk = bone_map.begin()->first;
		Bone & bone = bone_map.begin()->second;


		//blk_ref skin_inst = GetParent();
		//blk_link l;
		//attr_ref bone_attr = skin_inst->GetAttr("Bones");
		//
		//blk_ref bone_blk;
		//if ( bone_attr.is_null() == false ) {
		//	for (int i = 0; i < skin_inst->LinkCount(); ++i) {
		//		l = skin_inst->GetLink(i);
		//		if ( l.attr == bone_attr ) {
		//			//Found the first bone, record it and break
		//			bone_blk = l.block;
		//			break;
		//		}
		//	}
		//}
		//else {
		//	//Couldn't get the bone attr
		//	cout << "Couldn't get the bone attr" << endl;
		//	return;
		//}

		float offset_mat[4][4] = {
			bone.rotation[0][0], bone.rotation[0][1], bone.rotation[0][2], 0.0f,
			bone.rotation[1][0], bone.rotation[1][1], bone.rotation[1][2], 0.0f,
			bone.rotation[2][0], bone.rotation[2][1], bone.rotation[2][2], 0.0f,
			bone.translation[0], bone.translation[1], bone.translation[2], 1.0f
		};
			
		//Get built up rotations to the root of the skeleton from this bone
		INode * bone_node = (INode*)bone_blk->QueryInterface(Node);
		if (bone_node == NULL)
			throw runtime_error("Failed to get Node interface.");

		float bone_mat[4][4];
		bone_node->GetBindPosition( bone_mat );

		float result_mat[4][4];
		MultMatrix44( offset_mat, bone_mat, result_mat );

		//GetBuiltUpTransform( bone_blk, end_mat );

		//--Set TriShape Bind Position to Result--//
		INode * shape_node = (INode*)tri_shape->QueryInterface(Node);
		if (shape_node == NULL)
			throw runtime_error("Failed to get Node interface.");

		shape_node->SetBindPosition( result_mat );

		

		//Matrix res_mat = end_mat;// * start_mat.inverse();

		//tri_shape->GetAttr("Translation")->Set( res_mat(3,0), res_mat(3,1), res_mat(3,2) );

		//matrix rotation = { res_mat(0,0), res_mat(0,1), res_mat(0,2),
		//					res_mat(1,0), res_mat(1,1), res_mat(1,2),
		//					res_mat(2,0), res_mat(2,1), res_mat(2,2) };
		//tri_shape->GetAttr("Rotation")->Set( rotation );
	}
	else {
		cout << "Reposition Failed for " << tri_shape << endl;
	}
}

vector<blk_ref> NiSkinData::GetBones() {
	//Put all the valid bones from the map into a vector to return
	vector<blk_ref> bone_blks( bone_map.size() );

	map<blk_ref, Bone>::iterator it;
	int count = 0;
	for (it = bone_map.begin(); it != bone_map.end(); ++it ) {
		bone_blks[count] = it->first;
		count++;
	}

	return bone_blks;
}

map<int, float> NiSkinData::GetWeights( blk_ref bone ) {
	return bone_map[bone].weights;
}

void NiSkinData::AddBone( blk_ref bone, map<int, float> in ) {
	bone_map[bone].weights = in;
}
/***********************************************************
 * NiGeomMorpherController methods
 **********************************************************/

string NiGeomMorpherController::asString() {
	stringstream out;
	out.setf(ios::fixed, ios::floatfield);
	out << setprecision(1);

	out << ABlock::asString();

	out << "Flag Analysis:" << endl
		<< "   Animation Type:  ";

	int flags = GetAttr("Flags")->asInt();
	
	if (flags & 1)
		out << "APP_INIT" << endl;
	else
		out << "APP_TIME" << endl;

	out << "   Cycle Type:  ";
	if (flags & 2)
		out << "REVERSE" << endl;
	else if (flags & 4)
		out << "CLAMP" << endl;
	else
		out << "LOOP" << endl;

	out << "   Animation Enabled:  ";
	if (flags & 8)
		out << "Yes" <<endl;
	else
		out << "No" <<endl;

	return out.str();
}

/***********************************************************
 * NiKeyframeData methods
 **********************************************************/

void NiKeyframeData::Read( ifstream& in ) {

	scaleType = rotationType = translationType = 0;

	//--Rotation--//
	numRotations = ReadUInt( in );

	if (numRotations > 0) {
		rotationType = ReadUInt( in );
	
		rotKeys = new Key<fVector4>[numRotations];
		for (unsigned int i = 0; i < numRotations; i++) {
			rotKeys[i].time = ReadFloat( in );

			if (rotationType != 4) {
				ReadFVector4(rotKeys[i].data, in );
			}

			if (rotationType == 3) {
				ReadFVector3( rotKeys[i].tbc, in ) ;
			} else if (rotationType == 4) {
				throw runtime_error("NiKeyframeData rotation type 4 currently unsupported");
				//cout << "Rotation Type 4 Unsupported - Data will not be read" << endl;

				////cout << endl;
				//for (int j = 0; j < 3; j++) {
				//	//cout << "--Rotation Group " << j + 1 << "--" << endl;
				//	int subCount = ReadUInt( in );
				//	//cout << "Sub Count:  " << subCount << endl;
				//	int subType = ReadUInt( in );
				//	//cout << "Sub Type:  " << subType << endl;

				//	for (int k = 0; k < subCount; k++) {
				//		float subTime = ReadFloat( in );
				//		//cout << "KeyTime:  " << subTime << "  ";
				//		float subUnk1 = ReadFloat( in );
				//		//cout << "Data:  " << subUnk1;
				//		if (subType == 2) {
				//			float subUnk2 = ReadFloat( in );
				//			float subUnk3 = ReadFloat( in );
				//			//cout << ", " << subUnk2 << ", " << subUnk3;
				//		}
				//		//cout << endl;
				//	}
				//}
			}
		}
	}

	//--Translation--//
	numTranslations = ReadUInt( in );

	if (numTranslations > 0) {
		translationType = ReadUInt( in );

		transKeys = new Key<fVector3>[numTranslations];
		for (unsigned int i = 0; i < numTranslations; i++) {
			transKeys[i].time = ReadFloat( in );
			
			ReadFVector3( transKeys[i].data, in );

			if (translationType == 2) {
				ReadFVector3( transKeys[i].forward_tangent, in );
				ReadFVector3( transKeys[i].backward_tangent, in );
			}else if (translationType == 3) {
				ReadFVector3( transKeys[i].tbc, in );
			}
		}
	}
                        
	//--Scale--//
	numScalings = ReadUInt( in );

	if (numScalings > 0) {
		scaleType = ReadUInt( in );

		scaleKeys = new Key<float>[numScalings];
		for (unsigned int i = 0; i < numScalings; i++) {
			scaleKeys[i].time = ReadFloat( in );

			scaleKeys[i].data = ReadFloat( in );

			if (scaleType == 2) {
				scaleKeys[i].forward_tangent = ReadFloat( in );
				scaleKeys[i].backward_tangent = ReadFloat( in );
			} else if (scaleType == 3) {
				ReadFVector3( scaleKeys[i].tbc, in );
			}
		}
	}
}

string NiKeyframeData::asString() {
	stringstream out;
	out.setf(ios::fixed, ios::floatfield);
	out << setprecision(1);

	//out << setprecision(2);

	//--Rotation--//
	out << "Rotations:  " << numRotations << endl;

	if (numRotations > 0) {
		out << "Rotation Type:  " << rotationType << endl;

		if (verbose) {
			for (unsigned int i = 0; i < numRotations; i++) {
				out << "Key Time:  " << rotKeys[i].time << "  ";

				if (rotationType != 4) {
					out << "Rotation:  Q[" << rotKeys[i].data
						<< "   As Matrix:";
					QuatToMatrix(rotKeys[i].data, out );
					out << "   As Angles:";
					QuatToEuler(rotKeys[i].data, out );

				}
				
				if (rotationType == 3) {
					out << ",  TBC" << rotKeys[i].tbc;
				} else if (rotationType == 4) {
					out << "Rotation Type 4 Unsupported - Data was not read" << endl;
				}
				out << endl;
			}
		} else {
			out << "<<Data Not Shown>>" << endl;
		}	
	}

	//--Translation--//
	out << "Translations:  " << numTranslations << endl;

	if (numTranslations > 0) {
		out << "Translation Type:  " << translationType << endl;

		if (verbose) {
			for (unsigned int i = 0; i < numTranslations; i++) {
				out << "Key Time:  " << transKeys[i].time << "  ";
				
				out << "Data:  V" << transKeys[i].data;

				if (translationType == 2) {
					out << ", F" << transKeys[i].forward_tangent << ", B" << transKeys[i].backward_tangent;
				}else if (translationType == 3) {
					out << ", TBC" << transKeys[i].tbc;
				}
				out << endl;
			}
		} else {
			out << "<<Data Not Shown>>" << endl;
		}
		
	}
                        
	//--Scale--//
	out << "Scalings:  " << numScalings << endl;

	if (verbose) {
		if (numScalings > 0) {
			out << "Scale Type:  " << scaleType << endl;

			for (unsigned int i = 0; i < numScalings; i++) {
				out << "Key Time:  " << scaleKeys[i].time  << "  ";

				out << "Data:  S(" << scaleKeys[i].data << ")";

				if (scaleType == 2) {
					out << ", F(" << scaleKeys[i].forward_tangent << "), B(" << scaleKeys[i].backward_tangent << ")";
				} else if (scaleType == 3) {
					out << ", TBC" << scaleKeys[i].tbc;
				}
				out << endl;
			}
		}
	} else {
		out << "<<Data Not Shown>>" << endl;
	}

	//out << setprecision(1);

	return out.str();
}

/***********************************************************
 * NiColorData methods
 **********************************************************/

void NiColorData::Read( ifstream& in ) {
	colorCount = ReadUInt( in );
	keyType = ReadUInt( in );

	if (keyType != 1) {
		cout << "NiColorData is thought to only support keyType of 1, but this NIF has a keyType of " << keyType << ".  Please report it to NIFLA.  Aborting" << endl;
		throw runtime_error("Abort");
	}

	Key<fVector4> tmp;
	for (uint i = 0; i < colorCount; i++) {
		tmp.time = ReadFloat( in );
		ReadFVector4( tmp.data, in );
		keys.push_back(tmp);
	}
}

string NiColorData::asString() {
	stringstream out;
	out.setf(ios::fixed, ios::floatfield);
	out << setprecision(1);

	out << "Color Count:  " << colorCount << endl
		<< "Key Type:  " << keyType << endl;

	if (verbose) {
		vector<Key<fVector4> >::iterator it;
		for ( it = keys.begin(); it != keys.end(); ++it ) {
			out << "Key Time:  " << (*it).time << "  Color:  " << (*it).data << endl;
		}
	} else {
		out << "<<Data Not Shown>>" << endl;
	}

	return out.str();
}

/***********************************************************
 * NiFloatData methods
 **********************************************************/

void NiFloatData::Read( ifstream& in ) {	
	colorCount = ReadUInt( in );
	keyType = ReadUInt( in );

	if (keyType != 2) {
		cout << "NiFloatata is thought to only support keyType of 2, but this NIF has a keyType of " << keyType << ".  Please report it to NIFLA.  Aborting" << endl;
		throw runtime_error("Abort");
	}

	Key<float> tmp;
	for (uint i = 0; i < colorCount; i++) {
		tmp.time = ReadFloat( in );
		tmp.data = ReadFloat( in );
		tmp.forward_tangent = ReadFloat( in );
		tmp.backward_tangent = ReadFloat( in );
		keys.push_back(tmp);
	}
}

string NiFloatData::asString() {
	stringstream out;
	out.setf(ios::fixed, ios::floatfield);
	out << setprecision(1);

	out << "Vector Count:  " << colorCount << endl
		<< "Key Type:  " << keyType << endl;

	if (verbose) {
		vector<Key<float> >::iterator it;
		for ( it = keys.begin(); it != keys.end(); ++it ) {
			out << "Key Time:  " << it->time << "  Data: " << it->data << "  Forward: " << it->forward_tangent << "  Back: " << it->backward_tangent << endl;
		}
	} else {
		out << "<<Data Not Shown>>" << endl;
	}
	
	return out.str();
}

/***********************************************************
 * NiStringExtraData methods
 **********************************************************/

void NiStringExtraData::Read( ifstream& in ) {

	next_index.Read( in );// = ReadUInt( in );
	bytesRemaining = ReadUInt( in );
	strData = ReadString( in );
}

string NiStringExtraData::asString() {
	stringstream out;
	out.setf(ios::fixed, ios::floatfield);
	out << setprecision(1);

	out << "Next Extra Data:  " << next_index << endl
		<< "Bytes Remaining:  " << bytesRemaining << endl
		<< "String:  " << strData << endl;
	
	return out.str();
}

/***********************************************************
 * NiMorphData methods
 **********************************************************/

void NiMorphData::Read( ifstream& in ) {
	morphCount = ReadUInt( in );
	vertCount = ReadUInt( in );
	unknownByte = ReadByte( in );

	morphs.resize(morphCount);
	for ( uint i = 0; i < morphs.size() ; ++i ) {
		morphs[i].timeCount = ReadUInt( in );
		morphs[i].keyType = ReadUInt( in );

		if (morphs[i].timeCount > 0 && morphs[i].keyType != 2) {
			cout << "NiMorphData is thought to only support keyType of 2, but this NIF has a keyType of " << morphs[i].keyType << ".  Please report it to NIFLA.  Aborting" << endl;
			throw runtime_error("Abort");
		}

		Key<float> t_key;
		for (uint j = 0; j < morphs[i].timeCount; ++j ) {
			t_key.time = ReadFloat( in );
			t_key.data = ReadFloat( in );
			t_key.forward_tangent = ReadFloat( in );
			t_key.backward_tangent = ReadFloat( in );

			morphs[i].keys.push_back(t_key);
		}
		
		morphs[i].morph = new fVector3[vertCount];
		for (uint k = 0; k < vertCount ; ++k ) {
			ReadFVector3( morphs[i].morph[k], in );
		}
	}
}

string NiMorphData::asString() {
	stringstream out;
	out.setf(ios::fixed, ios::floatfield);
	out << setprecision(1);

	out << "Morph Count:  " << morphCount << endl
		<< "Vert Count:  " << vertCount << endl
		<< "Unknown Byte:  " << int(unknownByte) << endl;

	for ( uint i = 0; i < morphs.size() ; ++i ) {
		out << "---Morph " << i + 1 << "---" << endl;

		out << "Time Count:  " << morphs[i].timeCount << endl
			<< "Key Type:  " << morphs[i].keyType << endl;
		
		if (verbose) {
			for (uint j = 0; j < morphs[i].keys.size(); ++j ) {
				out << "Key Time:  " << morphs[i].keys[j].time << " Influence?: " << morphs[i].keys[j].data << " F: " << morphs[i].keys[j].forward_tangent << " B: " << morphs[i].keys[j].backward_tangent << endl;
			}
			
			for (uint k = 0; k < vertCount ; ++k ) {
				out << "Morph " << k + 1 << ":  " << morphs[i].morph[k] << endl;
			}
		} else {
			out << "<<Data Not Shown>>" << endl;
		}
	}

	out.setf(ios::fixed, ios::floatfield);
	out << setprecision(1);
	return out.str();
}

/***********************************************************
 * NiPixelData methods
 **********************************************************/

void NiPixelData::Read( ifstream& in ) {
	unknownInt = ReadUInt( in );
	rMask = ReadUInt( in );
	gMask = ReadUInt( in );
	bMask = ReadUInt( in );
	aMask = ReadUInt( in );
	bpp = ReadUInt( in );
		
	for (int i = 0; i < 8; ++i) {
		unknown8Bytes[i] = ReadByte( in );
	}

	unknownIndex = ReadUInt( in );
	mipCount = ReadUInt( in );
	bytesPerPixel = ReadUInt( in );

	MipMap temp;
	for ( uint i = 0; i < mipCount; ++i ) {
		temp.width = ReadUInt( in );
		temp.height = ReadUInt( in );
		temp.offset = ReadUInt( in );
		mipmaps.push_back(temp);
	}

	dataSize = ReadUInt( in );
	data = new byte[dataSize];
	in.read( (char *)data, dataSize);
}

string NiPixelData::asString() {
	stringstream out;
	out.setf(ios::fixed, ios::floatfield);
	out << setprecision(1);

	out << "Unknown Int:  " << unknownInt << endl
		<< "Red Mask:   " << Hex(rMask) << endl
		<< "Blue Mask:  " << Hex(bMask) << endl
		<< "Green Mask: " << Hex(gMask) << endl
		<< "Alpha Mask: " << Hex(aMask) << endl
		<< "Bits Per Pixel:  " << bpp << endl
		<< "Unknown 8 Bytes:" << endl;

	for (int i = 0; i < 8; ++i) {
		out << Hex(unknown8Bytes[i]) << "  ";
	}
	out << endl;

	out << "Unkown Index:  " << Index(unknownIndex) << endl
		<< "Mipmap Count:  " << mipCount << endl
		<< "Bytes Per Pixel:  " << bytesPerPixel << endl;

	for ( uint i = 0; i < mipmaps.size(); ++i ) {
		out << "Mipmap " << i + 1 << ":" << endl
			<< "   Width:  "  << mipmaps[i].width << endl
			<< "   Height:  " << mipmaps[i].height << endl
			<< "   Offset into Image Data Block:  " << mipmaps[i].offset << endl;
	}

	out << "Mipmap Image Data:  "  << dataSize << " Bytes (Not Shown)" << endl;
	
	return out.str();
}

/***********************************************************
 * NiPosData methods
 **********************************************************/

void NiPosData::Read( ifstream& in ) {
	posCount = ReadUInt( in );
	keyType = ReadUInt( in );

	keys.resize(posCount);
	for (uint i = 0; i < keys.size(); i++) {
		keys[i].time = ReadFloat( in );
		ReadFVector3( keys[i].data, in );

		if (keyType == 2) {
			ReadFVector3( keys[i].forward_tangent, in );
			ReadFVector3( keys[i].backward_tangent, in );
		}

		if (keyType != 1 && keyType != 2) {
			cout << "NiPosData is thought to only support keyTypes of 1 and 2, but this NIF has a keyType of " << keyType << ".  Please report it to NIFLA.  Aborting" << endl;
			throw runtime_error("Abort");
		}
	}
}

string NiPosData::asString() {
	stringstream out;
	out.setf(ios::fixed, ios::floatfield);
	out << setprecision(1);

	out << "Pos Count:  " << posCount << endl
		<< "Key Type:  " << keyType << endl;

	if (verbose) {
		for (uint i = 0; i < keys.size(); i++) {
			out << "Key Time:  " << keys[i].time << "   Position:  " << keys[i].data;

			if (keyType == 2) {
				out << " F: " << keys[i].forward_tangent << "  B: " << keys[i].backward_tangent;
			}
			out << endl;
		}
	} else {
		out << "<<Data Not Shown>>" << endl;
	}
	
	return out.str();
}

/***********************************************************
 * NiRotatingParticlesData methods
 **********************************************************/

void NiRotatingParticlesData::Read( ifstream& in ) {

 //   short count1

 //   int b1
 //   if(b1 != 0)
 //       foreach count1
 //           float[3] unknown

 //   int or float = 0
 //   float[4] unknown

 //   int b2
 //   if(b2 != 0)
 //       foreach count1
 //           float[4] unknown

 //   int = 0
 //   short unknown
 //   short count2             - always equal to count1 ?
 //   float unknown
 //   short unknown

 //   int b3
 //   if(b3 != 0)
 //       foreach count2 (or count1)
 //           float unknown

 //   int b4
 //   if(b4 != 0)
 //       foreach count2 (or count1)
 //           float[4] unknown

	ushort count1 = ReadUShort( in );
	uint b1 = ReadUInt( in );

	cout << "Count 1:  " << count1 << endl
		 << "Bool 1:  " << b1 << endl;

	if (b1) {
		fVector3 vect;
		for (uint i = 0; i < count1; ++i) {
			ReadFVector3( vect, in );
			cout << "Unknown 3 Floats:  " << vect << endl;
		}
	}

	uint unknownInt = ReadUInt( in );

	fVector4 unknown4Floats;
	ReadFVector4( unknown4Floats, in );
	uint b2 = ReadUInt( in );

	cout << "Unknown Int:  " << unknownInt << endl
		 << "Unknown 4 Floats:  " << unknown4Floats << endl
		 << "Bool 2:  " << b2 << endl;

	if (b2) {
		fVector4 unknown4Floats2;
		for (uint i = 0; i < count1; ++i) {
			ReadFVector4( unknown4Floats2, in );
			cout << "Unkown 4 Floats:  " << unknown4Floats2 << endl;
		}
	}

	uint unknownInt2 = ReadUInt( in );
	ushort unknownShort = ReadUShort( in );
	ushort count2 = ReadUShort( in );
	float unknownFloat = ReadFloat( in );
	ushort unknownShort2 = ReadUShort( in );
	uint b3 = ReadUInt( in );

	cout << "Unknown Int 2:  " << unknownInt2 << endl
		 << "Unknown Short:  " << unknownShort << endl
		 << "Count 2:  " << count2 << endl
		 << "Unknown Float:  " << unknownFloat << endl
		 << "Unknown Short 2:  " << unknownShort2 << endl
		 << "Bool 3:  " << b3 << endl;

	if (b3) {
		float temp;
		for (uint i = 0; i < count1; ++i) {
			temp = ReadFloat( in );
			cout << "Unkown Float:  " << temp << endl;
		}
	}

	uint b4 = ReadUInt( in );

	cout << "Bool 4:  " << b4 << endl;

	if (b4) {
		fVector4 unknown4Floats3;
		for (uint i = 0; i < count1; ++i) {
			ReadFVector4( unknown4Floats3, in );
			cout << "Unkown 4 Floats:  " << unknown4Floats3 << endl;
		}
	}

	//int len = BlockSearch(in);

	////Create byte array and read in unknown block
	//byte * data = new byte [len];
	//in.read((char*)data, len);

	//Display Data in Hex form
	//cout << hex << setfill('0');
	//for (int j = 0; j < len; j++) {
	//	cout << uppercase << setw(2) << uint(data[j]);
	//	if (j % 16 == 15 || j == len - 1)
	//		cout << endl;
	//	else if (j % 16 == 7)
	//		cout << "   ";
	//	else if (j % 8 == 3)
	//		cout << "  ";
	//	else
	//		cout << " ";
	//}
	//cout << dec << setfill(' ');

	/*delete [] data;*/

	//if (count1 != count2) {
	//	cout << "\a";
	//	cin.get();
	//}
}

/***********************************************************
 * NiTextKeyExtraData methods
 **********************************************************/

void NiTextKeyExtraData::Read( ifstream& in ) {
	next_index.Read( in );
	unknownInt = ReadUInt( in );
	keyCount = ReadUInt( in );

	keys.resize(keyCount);
	for (uint i = 0; i < keys.size(); ++i ) {
		keys[i].time = ReadFloat( in );
		keys[i].data = ReadString( in );
	}
}

string NiTextKeyExtraData::asString() {
	stringstream out;
	out.setf(ios::fixed, ios::floatfield);
	out << setprecision(1);

	out << "Next Extra Data:  " << next_index << endl
		<< "Unknown Int (Key Type?):  " << unknownInt << endl
		<< "Key Count:  " << keyCount << endl;

	if (verbose) {
		for (uint i = 0; i < keys.size(); ++i ) {
			out << "Key Time:  " << keys[i].time << endl
				<< "Key Text:  " << keys[i].data << endl;
		}
	} else {
		out << "<<Data Not Shown>>" << endl;
	}
	
	return out.str();
}

/***********************************************************
 * NiUVData methods
 **********************************************************/

void NiUVData::Read( ifstream& in ) {	
	for (uint i = 0; i < 4; ++i) {
		groups[i].count = ReadUInt( in );

		if ( groups[i].count > 0 ) {
			groups[i].keyType = ReadUInt( in );

			groups[i].keys.resize(groups[i].count);
			for (uint j = 0; j < groups[i].keys.size(); ++j) {
				groups[i].keys[j].time = ReadFloat( in );
				groups[i].keys[j].data = ReadFloat( in );

				if ( groups[i].keyType == 2) {
					groups[i].keys[j].forward_tangent = ReadFloat( in );
					groups[i].keys[j].backward_tangent = ReadFloat( in );
				}
			}
		}
	}
}

string NiUVData::asString() {
	stringstream out;
	out.setf(ios::fixed, ios::floatfield);
	out << setprecision(1);

	for (uint i = 0; i < 4; ++i) {
		out << "UV Group " << i + 1 << ":" << endl
			<< "   Count:  " << groups[i].count << endl;

		if ( groups[i].count > 0 ) {
			out << "   Key Type:  " << groups[i].keyType << endl;

			if (verbose) {
				for (uint j = 0; j < groups[i].keys.size(); ++j) {
					out << "   Key Time:  " << groups[i].keys[j].time << " Data:  " << groups[i].keys[j].data;

					if ( groups[i].keyType == 2) {
						out << "  F: " << groups[i].keys[j].forward_tangent << "  B: " << groups[i].keys[j].backward_tangent;
					}
					out << endl;
				}
			} else {
				out << "<<Data Not Shown>>" << endl;
			}
		}
	}

	return out.str();
}

/***********************************************************
 * NiVertWeightsExtraData methods
 **********************************************************/
 
void NiVertWeightsExtraData ::Read( ifstream& in ) {
	next_index.Read( in );
	bytes = ReadUInt( in );
	verts = ReadUShort( in );

	weights.resize( verts );
	for (uint i = 0; i < weights.size(); ++i) {
		weights[i] = ReadFloat( in );
	}
}

string NiVertWeightsExtraData::asString() {
	stringstream out;
	out.setf(ios::fixed, ios::floatfield);
	out << setprecision(1);

	out << "Next Extra Data:  " << next_index << endl
		<< "Bytes:  " << bytes << endl
		<< "Verts:  " << verts << endl;

	if (verbose) {
		for (uint i = 0; i < weights.size(); ++i) {
			out << "Weight " << i + 1 << ":  " << weights[i] << endl;
		}		
	} else {
		out << "<<Data Not Shown>>" << endl;
	}

	return out.str();
}

/***********************************************************
 * NiVisData methods
 **********************************************************/

void NiVisData ::Read( ifstream& in ) {
	visCount = ReadUInt( in );

	keys.resize( visCount );
	for (uint i = 0; i < keys.size(); ++i) {
		keys[i].time = ReadFloat( in );
		keys[i].data = ReadByte( in ); // Is Visible? True/False
	}
}

string NiVisData::asString() {
	stringstream out;
	out.setf(ios::fixed, ios::floatfield);
	out << setprecision(1);

	out << "Visibility Count:  "  << visCount << endl;

	if (verbose) {
		for (uint i = 0; i < keys.size(); ++i) {
			out << "Key Time:  " << keys[i].time << "  Visible:  " << int(keys[i].data) << endl;
		}		
	} else {
		out << "<<Data Not Shown>>" << endl;
	}

	return out.str();
}

/***********************************************************
 * UnknownMixIn methods
 **********************************************************/

void UnknownMixIn::Read( ifstream &in ) {
	len = BlockSearch(in);

	//Create byte array and read in unknown block
	data = new byte [len];
	in.read((char*)data, len);
}

string UnknownMixIn::asString() {
	stringstream out;
	out.setf(ios::fixed, ios::floatfield);
	out << setprecision(1);

	out << "Unknown Data (" << len << " bytes):" << endl;
	
	if (verbose) {
		//Display Data in Hex form
		out << hex << setfill('0');
		for (int j = 0; j < len; j++) {
			out << uppercase << setw(2) << uint(data[j]);
			if (j % 16 == 15 || j == len - 1)
				out << endl;
			else if (j % 16 == 7)
				out << "   ";
			else if (j % 8 == 3)
				out << "  ";
			else
				out << " ";
		}
		out << dec << setfill(' ');
	} else {
		out << "<<Data Not Shown>>" << endl;
	}

	return out.str();
}

void UnknownMixIn::Write( ofstream& out ) {
	out.write( (const char*)data, len );
}

//int len = BlockSearch(in);

////Create byte array and read in unknown block
//byte * data = new byte [len];
//in.read((char*)data, len);

////Display Data in Hex form
//cout << hex << setfill('0');
//for (int j = 0; j < len; j++) {
//	cout << uppercase << setw(2) << uint(data[j]);
//	if (j % 16 == 15 || j == len - 1)
//		cout << endl;
//	else if (j % 16 == 7)
//		cout << "   ";
//	else if (j % 8 == 3)
//		cout << "  ";
//	else
//		cout << " ";
//}
//cout << dec << setfill(' ');

//delete [] data;