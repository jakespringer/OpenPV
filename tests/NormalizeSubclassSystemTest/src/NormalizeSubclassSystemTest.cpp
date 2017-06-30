//============================================================================
// Name        : NormalizeSystemTest.cpp
// Author      :
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include "NormalizeL3.hpp"
#include <columns/buildandrun.hpp>

int customexit(HyPerCol *hc, int argc, char *argv[]);

int main(int argc, char *argv[]) {
   PV_Init pv_initObj(&argc, &argv, false /*do not allow unrecognized arguments*/);
   pv_initObj.registerKeyword("normalizeL3", Factory::create<NormalizeL3>);
   int status = buildandrun(&pv_initObj, NULL, customexit);
   return status;
}

int customexit(HyPerCol *hc, int argc, char *argv[]) {
   float tol = 1e-5;

   BaseConnection *baseConn;
   baseConn                   = hc->getConnFromName("NormalizeL3Connection");
   HyPerConn *normalizeL3Conn = dynamic_cast<HyPerConn *>(baseConn);
   FatalIf(normalizeL3Conn == nullptr, "Connection \"NormalizeL3Connection\" does not exist.\n");
   NormalizeBase *normalizeL3Normalizer = normalizeL3Conn->getNormalizer();
   FatalIf(normalizeL3Normalizer == nullptr, "NormalizeL3Connection has no normalizer.\n");
   float normalizeL3Strength    = normalizeL3Normalizer->getStrength();
   float correctValue           = powf(normalizeL3Strength, 3.0f);
   HyPerLayer *normalizeL3Check = hc->getLayerFromName("NormalizeL3Check");
   FatalIf(normalizeL3Check == nullptr, "Layer \"NormalizeL3Check\" does not exist.\n");
   float normalizeL3Value = normalizeL3Check->getLayerData()[0];
   FatalIf(
         fabsf(normalizeL3Value - correctValue) >= tol,
         "Result %f differs from %f by more than allowed tolerance.\n",
         (double)normalizeL3Value,
         (double)correctValue);

   return PV_SUCCESS;
}
