/* Copyright (c) 2006, NIF File Format Library and Tools
All rights reserved.  Please see niflib.h for licence. */

#ifndef _NIPARTICLEBOMB_H_
#define _NIPARTICLEBOMB_H_

#include "AParticleModifier.h"
namespace Niflib {

#include "../gen/obj_defines.h"

class NiParticleBomb;
typedef Ref<NiParticleBomb> NiParticleBombRef;

/*!
 * NiParticleBomb - A particle modifier.
 */

class NIFLIB_API NiParticleBomb : public NI_PARTICLE_BOMB_PARENT {
public:
	NiParticleBomb();
	~NiParticleBomb();
	//Run-Time Type Information
	static const Type & TypeConst() { return TYPE; }
private:
	static const Type TYPE;
public:
	virtual void Read( istream& in, list<uint> & link_stack, unsigned int version, unsigned int user_version );
	virtual void Write( ostream& out, map<NiObjectRef,uint> link_map, unsigned int version, unsigned int user_version ) const;
	virtual string asString( bool verbose = false ) const;
	virtual void FixLinks( const vector<NiObjectRef> & objects, list<uint> & link_stack, unsigned int version, unsigned int user_version );
	virtual list<NiObjectRef> GetRefs() const;
	virtual const Type & GetType() const;

protected:
	NI_PARTICLE_BOMB_MEMBERS
	STANDARD_INTERNAL_METHODS
};

}
#endif