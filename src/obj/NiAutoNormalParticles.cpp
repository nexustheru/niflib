/* Copyright (c) 2006, NIF File Format Library and Tools
All rights reserved.  Please see niflib.h for licence. */

#include "../../include/obj/NiAutoNormalParticles.h"
using namespace Niflib;

//Definition of TYPE constant
const Type NiAutoNormalParticles::TYPE("NiAutoNormalParticles", &NI_AUTO_NORMAL_PARTICLES_PARENT::TYPE );

NiAutoNormalParticles::NiAutoNormalParticles() NI_AUTO_NORMAL_PARTICLES_CONSTRUCT {}

NiAutoNormalParticles::~NiAutoNormalParticles() {}

void NiAutoNormalParticles::Read( istream& in, list<unsigned int> & link_stack, const NifInfo & info ) {
	InternalRead( in, link_stack, info );
}

void NiAutoNormalParticles::Write( ostream& out, const map<NiObjectRef,unsigned int> & link_map, const NifInfo & info ) const {
	InternalWrite( out, link_map, info );
}

string NiAutoNormalParticles::asString( bool verbose ) const {
	return InternalAsString( verbose );
}

void NiAutoNormalParticles::FixLinks( const map<unsigned int,NiObjectRef> & objects, list<unsigned int> & link_stack, const NifInfo & info ) {
	InternalFixLinks( objects, link_stack, info );
}

list<NiObjectRef> NiAutoNormalParticles::GetRefs() const {
	return InternalGetRefs();
}

const Type & NiAutoNormalParticles::GetType() const {
	return TYPE;
};
