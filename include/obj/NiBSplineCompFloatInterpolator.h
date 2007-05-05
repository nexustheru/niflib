/* Copyright (c) 2006, NIF File Format Library and Tools
All rights reserved.  Please see niflib.h for licence. */

#ifndef _NIBSPLINECOMPFLOATINTERPOLATOR_H_
#define _NIBSPLINECOMPFLOATINTERPOLATOR_H_

#include "NiBSplineInterpolator.h"
namespace Niflib {


//#include "../gen/obj_defines.h"

class NiBSplineCompFloatInterpolator;
typedef Ref<NiBSplineCompFloatInterpolator> NiBSplineCompFloatInterpolatorRef;

/*!
 * NiBSplineCompFloatInterpolator - Unknown.
 */

class NiBSplineCompFloatInterpolator : public NI_B_SPLINE_COMP_FLOAT_INTERPOLATOR_PARENT {
public:
	NIFLIB_API NiBSplineCompFloatInterpolator();
	NIFLIB_API ~NiBSplineCompFloatInterpolator();
	//Run-Time Type Information
	NIFLIB_API static const Type TYPE;
	NIFLIB_API virtual const Type & GetType() const;
	NIFLIB_HIDDEN virtual void Read( istream& in, list<unsigned int> & link_stack, const NifInfo & info );
	NIFLIB_HIDDEN virtual void Write( ostream& out, const map<NiObjectRef,unsigned int> & link_map, const NifInfo & info ) const;
	NIFLIB_API virtual string asString( bool verbose = false ) const;
	NIFLIB_HIDDEN virtual void FixLinks( const map<unsigned int,NiObjectRef> & objects, list<unsigned int> & link_stack, const NifInfo & info );
	NIFLIB_HIDDEN virtual list<NiObjectRef> GetRefs() const;

protected:
	NI_B_SPLINE_COMP_FLOAT_INTERPOLATOR_MEMBERS
private:
	void InternalRead( istream& in, list<unsigned int> & link_stack, const NifInfo & info );
	void InternalWrite( ostream& out, const map<NiObjectRef,unsigned int> & link_map, const NifInfo & info ) const;
	string InternalAsString( bool verbose ) const;
	void InternalFixLinks( const map<unsigned int,NiObjectRef> & objects, list<unsigned int> & link_stack, const NifInfo & info );
	list<NiObjectRef> InternalGetRefs() const;
};

}
#endif
