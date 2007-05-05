/* Copyright (c) 2006, NIF File Format Library and Tools
All rights reserved.  Please see niflib.h for licence. */

#include "../../include/obj/NiBillboardNode.h"
using namespace Niflib;

//Definition of TYPE constant
const Type NiBillboardNode::TYPE("NiBillboardNode", &NI_BILLBOARD_NODE_PARENT::TYPE );

NiBillboardNode::NiBillboardNode() NI_BILLBOARD_NODE_CONSTRUCT {}

NiBillboardNode::~NiBillboardNode() {}

void NiBillboardNode::Read( istream& in, list<unsigned int> & link_stack, const NifInfo & info ) {
	InternalRead( in, link_stack, info );
}

void NiBillboardNode::Write( ostream& out, const map<NiObjectRef,unsigned int> & link_map, const NifInfo & info ) const {
	InternalWrite( out, link_map, info );
}

string NiBillboardNode::asString( bool verbose ) const {
	return InternalAsString( verbose );
}

void NiBillboardNode::FixLinks( const map<unsigned int,NiObjectRef> & objects, list<unsigned int> & link_stack, const NifInfo & info ) {
	InternalFixLinks( objects, link_stack, info );
}

list<NiObjectRef> NiBillboardNode::GetRefs() const {
	return InternalGetRefs();
}

const Type & NiBillboardNode::GetType() const {
	return TYPE;
};

BillboardMode NiBillboardNode::GetBillboardMode() const {
	return billboardMode;
}

void NiBillboardNode::SetBillboardMode( BillboardMode value ) {
	billboardMode = value;
}
