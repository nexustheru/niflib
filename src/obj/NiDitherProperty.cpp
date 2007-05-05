/* Copyright (c) 2006, NIF File Format Library and Tools
All rights reserved.  Please see niflib.h for licence. */

#include "../../include/obj/NiDitherProperty.h"
using namespace Niflib;

//Definition of TYPE constant
const Type NiDitherProperty::TYPE("NiDitherProperty", &NI_DITHER_PROPERTY_PARENT::TYPE );

NiDitherProperty::NiDitherProperty() NI_DITHER_PROPERTY_CONSTRUCT {}

NiDitherProperty::~NiDitherProperty() {}

void NiDitherProperty::Read( istream& in, list<unsigned int> & link_stack, const NifInfo & info ) {
	InternalRead( in, link_stack, info );
}

void NiDitherProperty::Write( ostream& out, const map<NiObjectRef,unsigned int> & link_map, const NifInfo & info ) const {
	InternalWrite( out, link_map, info );
}

string NiDitherProperty::asString( bool verbose ) const {
	return InternalAsString( verbose );
}

void NiDitherProperty::FixLinks( const map<unsigned int,NiObjectRef> & objects, list<unsigned int> & link_stack, const NifInfo & info ) {
	InternalFixLinks( objects, link_stack, info );
}

list<NiObjectRef> NiDitherProperty::GetRefs() const {
	return InternalGetRefs();
}

const Type & NiDitherProperty::GetType() const {
	return TYPE;
};

unsigned short NiDitherProperty::GetFlags() const {
   return flags;
}

void NiDitherProperty::SetFlags( unsigned short n ) {
   flags = n;
}
