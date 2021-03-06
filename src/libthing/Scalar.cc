#include "libthing/Scalar.h"

namespace libthing {

/** (t)olerance (equals)
 * @returns true if two Scalar values are approximately the same using tolerance
 */
bool tequals(Scalar const& a, Scalar const& b, Scalar const& tol)
{
	return SCALAR_ABS(a-b) < tol;
}

};
