/*
 * GitRevisionString.cpp
 *
 *  Created on: Mar 27, 2019
 *      Author: Pete Schultz
 */

#include "GitRevisionString.hpp"
#include "pvGitRevision.hpp"
// pvGitRevision.hpp is generated at compile time, in the build directory's lib/include directory.
// It is included by the GitRevisionString.cpp file, not the GitRevisionString.hpp file, so that
// GitRevisionString.cpp is the only source file that needs to be recompiled as a consequence of
// pvGitRevision.hpp changing.

namespace PV {

char const *GitRevisionString() { return pvGitRevision; }

} // end namespace PV
