#ifndef ThresholdPtTrajectoryFilter_H
#define ThresholdPtTrajectoryFilter_H

#include "TrackingTools/TrajectoryFiltering/interface/TrajectoryFilter.h"
#include "TrackingTools/PatternTools/interface/Trajectory.h"
#include "TrackingTools/PatternTools/interface/TempTrajectory.h"

#include "TrackingTools/TrajectoryState/interface/FreeTrajectoryState.h"
#include "TrackingTools/TrajectoryParametrization/interface/CurvilinearTrajectoryError.h"
#include "TrackingTools/TrajectoryState/interface/TrajectoryStateAccessor.h"

/** A TrajectoryFilter that stops reconstruction if P_t goes 
 *  above some value at some confidence level.
 *  The CkfTrajectoryBuilder uses this class to
 *  implement the conditional p_T cut
 */

class ThresholdPtTrajectoryFilter : public TrajectoryFilter {
public:

  explicit ThresholdPtTrajectoryFilter( double ptThreshold, float nSigma = 5.F, int nH): thePtThreshold( ptThreshold), theNSigma(nSigma), theMinHits(nH) {}

  explicit ThresholdPtTrajectoryFilter( const edm::ParameterSet & pset) :
    thePtThreshold(pset.getParameter<double>("thresholdPt")),
    theNSigma(pset.getParameter<double>("nSigmaThresholdPt")),
    theMinHits(pset.getParameter<int>("minHitsThresholdPt"))
      {}

  virtual bool qualityFilter( const Trajectory& traj) const { return !test(traj.lastMeasurement(),traj.foundHits());}
  virtual bool qualityFilter( const TempTrajectory& traj) const { return !test(traj.lastMeasurement(),traj.foundHits());}
   
  virtual bool toBeContinued( Trajectory& traj) const { return test(traj.lastMeasurement(),traj.foundHits()); }
  virtual bool toBeContinued( TempTrajectory& traj) const { return test(traj.lastMeasurement(),traj.foundHits()); }
  
  virtual std::string name() const {return "ThresholdPtTrajectoryFilter";}

 protected:

  bool test( const TrajectoryMeasurement & tm, int foundHits) const 
  {
    static bool answerMemory=false;

    //first check min number of hits 
    if (foundHits < theMinHits ){answerMemory=true; return true;}

    // check for momentum below limit
    const FreeTrajectoryState& fts = *tm.updatedState().freeTrajectoryState();

    //avoid doing twice the check in TBC and QF
    static FreeTrajectoryState ftsMemory;
    if (ftsMemory.parameters().vector() == fts.parameters().vector()) {ftsMemory=fts; return answerMemory;}

    //if p_T is way too small: stop
    double pT = fts.momentum().perp();
    if (pT<0.010) {answerMemory=false; return false;}
    //if error is way too big: stop
    double invError = TrajectoryStateAccessor(fts).inversePtError();
    if (invError > 1.e10) {answerMemory=false;return false;}

    //calculate the actual pT cut: 
    if ((1/pT + theNSigma*invError ) > 1/thePtThreshold ) {answerMemory=false; return false;}
    //    first term is the minimal value of pT (pT-N*sigma(pT))
    //    secon term is the cut

    answerMemory=true; return true;
  }

  double thePtThreshold;
  double theNSigma;
  int theMinHits;

};

#endif
