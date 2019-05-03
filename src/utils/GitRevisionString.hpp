/*
 * GitRevisionString.hpp
 *
 *  Created on: Mar 27, 2019
 *      Author: Pete Schultz
 */

#ifndef GITREVISIONSTRING_HPP_
#define GITREVISIONSTRING_HPP_

namespace PV {

// Returns a char array indicating the commit in the git repository used at compile time,
// and whether any uncommitted modifications to that commit existed at compile time.
char const *GitRevisionString();

} // end namespace PV

#endif // GITREVISIONSTRING_HPP_
