/* Copyright (c) 2006, NIF File Format Library and Tools
All rights reserved.  Please see niflib.h for licence. */

#include "NiPathController.h"
#include "NiPosData.h"
#include "NiFloatData.h"

//Definition of TYPE constant
const Type NiPathController::TYPE("NiPathController", &NI_PATH_CONTROLLER_PARENT::TYPE );

NiPathController::NiPathController() NI_PATH_CONTROLLER_CONSTRUCT {}

NiPathController::~NiPathController() {}

void NiPathController::Read( istream& in, list<uint> link_stack, unsigned int version ) {
	NI_PATH_CONTROLLER_READ
}

void NiPathController::Write( ostream& out, map<NiObjectRef,uint> link_map, unsigned int version ) const {
	NI_PATH_CONTROLLER_WRITE
}

string NiPathController::asString( bool verbose ) const {
	NI_PATH_CONTROLLER_STRING
}

void NiPathController::FixLinks( const vector<NiObjectRef> & objects, list<uint> link_stack, unsigned int version ) {
	NI_PATH_CONTROLLER_FIXLINKS
}
