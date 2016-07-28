/*
 * pv.cpp
 *
 */

#include <columns/buildandrun.hpp>
#include "DelayTestProbe.hpp"

int main(int argc, char * argv[]) {
   PV_Init pv_initObj(&argc, &argv, false/*do not allow unrecognized arguments*/);
   pv_initObj.registerKeyword("DelayTestProbe", Factory::create<DelayTestProbe>);
   int status = buildandrun(&pv_initObj);
   return status==PV_SUCCESS ? EXIT_SUCCESS : EXIT_FAILURE;
}
