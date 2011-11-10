/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

 Header:       FGInitialCondition.h
 Author:       Tony Peden
 Date started: 7/1/99

 ------------- Copyright (C) 1999  Anthony K. Peden (apeden@earthlink.net) -------------

 This program is free software; you can redistribute it and/or modify it under
 the terms of the GNU Lesser General Public License as published by the Free Software
 Foundation; either version 2 of the License, or (at your option) any later
 version.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 details.

 You should have received a copy of the GNU Lesser General Public License along with
 this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 Place - Suite 330, Boston, MA  02111-1307, USA.

 Further information about the GNU Lesser General Public License can also be found on
 the world wide web at http://www.gnu.org.

 HISTORY
--------------------------------------------------------------------------------
7/1/99   TP   Created

FUNCTIONAL DESCRIPTION
--------------------------------------------------------------------------------

The purpose of this class is to take a set of initial conditions and provide
a kinematically consistent set of body axis velocity components, euler
angles, and altitude.  This class does not attempt to trim the model i.e.
the sim will most likely start in a very dynamic state (unless, of course,
you have chosen your IC's wisely) even after setting it up with this class.

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
SENTRY
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

#ifndef FGINITIALCONDITION_H
#define FGINITIALCONDITION_H

/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
INCLUDES
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

#include "input_output/FGXMLFileRead.h"
#include "math/FGLocation.h"

/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
DEFINITIONS
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

#define ID_INITIALCONDITION "$Id: FGInitialCondition.h,v 1.27 2011/05/20 00:47:03 bcoconni Exp $"

/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
FORWARD DECLARATIONS
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

namespace JSBSim {

class FGFDMExec;
class FGMatrix33;
class FGColumnVector3;

typedef enum { setvt, setvc, setve, setmach, setuvw, setned, setvg } speedset;

/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
CLASS DOCUMENTATION
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

/** Initializes the simulation run.
    Takes a set of initial conditions (IC) and provide a kinematically consistent set
    of body axis velocity components, euler angles, and altitude.  This class
    does not attempt to trim the model i.e. the sim will most likely start in a
    very dynamic state (unless, of course, you have chosen your IC's wisely, or
    started on the ground) even after setting it up with this class.

   <h3>Usage Notes</h3>

   With a valid object of FGFDMExec and an aircraft model loaded:

   @code
   FGInitialCondition* fgic = FDMExec->GetIC();

   // Reset the initial conditions and set VCAS and altitude
   fgic->InitializeIC();
   fgic->SetVcalibratedKtsIC(vcas);
   fgic->SetAltitudeAGLFtIC(altitude);

   // directly into Run
   FDMExec->GetPropagate()->SetInitialState(fgic);
   delete fgic;
   FDMExec->Run();

   //or to loop the sim w/o integrating
   FDMExec->RunIC();
   @endcode

   Alternatively, you can load initial conditions from an XML file:

   @code
   FGInitialCondition* fgic = FDMExec->GetIC();
   fgic->Load(IC_file);
   @endcode

   <h3>Speed</h3>

   Since vc, ve, vt, and mach all represent speed, the remaining
   three are recalculated each time one of them is set (using the
   current altitude).  The most recent speed set is remembered so
   that if and when altitude is reset, the last set speed is used
   to recalculate the remaining three. Setting any of the body
   components forces a recalculation of vt and vt then becomes the
   most recent speed set.

   <h3>Alpha,Gamma, and Theta</h3>

   This class assumes that it will be used to set up the sim for a
   steady, zero pitch rate condition. Since any two of those angles
   specifies the third gamma (flight path angle) is favored when setting
   alpha and theta and alpha is favored when setting gamma. i.e.

   - set alpha : recalculate theta using gamma as currently set
   - set theta : recalculate alpha using gamma as currently set
   - set gamma : recalculate theta using alpha as currently set

   The idea being that gamma is most interesting to pilots (since it
   is indicative of climb rate).

   Setting climb rate is, for the purpose of this discussion,
   considered equivalent to setting gamma.

   These are the items that can be set in an initialization file:

   - ubody (velocity, ft/sec)
   - vbody (velocity, ft/sec)
   - wbody (velocity, ft/sec)
   - vnorth (velocity, ft/sec)
   - veast (velocity, ft/sec)
   - vdown (velocity, ft/sec)
   - latitude (position, degrees)
   - longitude (position, degrees)
   - phi (orientation, degrees)
   - theta (orientation, degrees)
   - psi (orientation, degrees)
   - alpha (angle, degrees)
   - beta (angle, degrees)
   - gamma (angle, degrees)
   - roc (vertical velocity, ft/sec)
   - elevation (local terrain elevation, ft)
   - altitude (altitude AGL, ft)
   - altitudeAGL (altitude AGL, ft)
   - altitudeMSL (altitude MSL, ft)
   - winddir (wind from-angle, degrees)
   - vwind (magnitude wind speed, ft/sec)
   - hwind (headwind speed, knots)
   - xwind (crosswind speed, knots)
   - vc (calibrated airspeed, ft/sec)
   - mach (mach)
   - vground (ground speed, ft/sec)
   - running (-1 for all engines, 0 for no engines, 1 ... n for specific engines)

   <h3>Properties</h3>
   @property ic/vc-kts (read/write) Calibrated airspeed initial condition in knots
   @property ic/ve-kts (read/write) Knots equivalent airspeed initial condition
   @property ic/vg-kts (read/write) Ground speed initial condition in knots
   @property ic/vt-kts (read/write) True airspeed initial condition in knots
   @property ic/mach (read/write) Mach initial condition
   @property ic/roc-fpm (read/write) Rate of climb initial condition in feet/minute
   @property ic/gamma-deg (read/write) Flightpath angle initial condition in degrees
   @property ic/alpha-deg (read/write) Angle of attack initial condition in degrees
   @property ic/beta-deg (read/write) Angle of sideslip initial condition in degrees
   @property ic/theta-deg (read/write) Pitch angle initial condition in degrees
   @property ic/phi-deg (read/write) Roll angle initial condition in degrees
   @property ic/psi-true-deg (read/write) Heading angle initial condition in degrees
   @property ic/lat-gc-deg (read/write) Latitude initial condition in degrees
   @property ic/long-gc-deg (read/write) Longitude initial condition in degrees
   @property ic/h-sl-ft (read/write) Height above sea level initial condition in feet
   @property ic/h-agl-ft (read/write) Height above ground level initial condition in feet
   @property ic/sea-level-radius-ft (read/write) Radius of planet at sea level in feet
   @property ic/terrain-elevation-ft (read/write) Terrain elevation above sea level in feet
   @property ic/vg-fps (read/write) Ground speed initial condition in feet/second
   @property ic/vt-fps (read/write) True airspeed initial condition in feet/second
   @property ic/vw-bx-fps (read/write) Wind velocity initial condition in Body X frame in feet/second
   @property ic/vw-by-fps (read/write) Wind velocity initial condition in Body Y frame in feet/second
   @property ic/vw-bz-fps (read/write) Wind velocity initial condition in Body Z frame in feet/second
   @property ic/vw-north-fps (read/write) Wind northward velocity initial condition in feet/second
   @property ic/vw-east-fps (read/write) Wind eastward velocity initial condition in feet/second
   @property ic/vw-down-fps (read/write) Wind downward velocity initial condition in feet/second
   @property ic/vw-mag-fps (read/write) Wind velocity magnitude initial condition in feet/sec.
   @property ic/vw-dir-deg (read/write) Wind direction initial condition, in degrees from north
   @property ic/roc-fps (read/write) Rate of climb initial condition, in feet/second
   @property ic/u-fps (read/write) Body frame x-axis velocity initial condition in feet/second
   @property ic/v-fps (read/write) Body frame y-axis velocity initial condition in feet/second
   @property ic/w-fps (read/write) Body frame z-axis velocity initial condition in feet/second
   @property ic/vn-fps (read/write) Local frame x-axis (north) velocity initial condition in feet/second
   @property ic/ve-fps (read/write) Local frame y-axis (east) velocity initial condition in feet/second
   @property ic/vd-fps (read/write) Local frame z-axis (down) velocity initial condition in feet/second
   @property ic/gamma-rad (read/write) Flight path angle initial condition in radians
   @property ic/alpha-rad (read/write) Angle of attack initial condition in radians
   @property ic/theta-rad (read/write) Pitch angle initial condition in radians
   @property ic/beta-rad (read/write) Angle of sideslip initial condition in radians
   @property ic/phi-rad (read/write) Roll angle initial condition in radians
   @property ic/psi-true-rad (read/write) Heading angle initial condition in radians
   @property ic/lat-gc-rad (read/write) Geocentric latitude initial condition in radians
   @property ic/long-gc-rad (read/write) Longitude initial condition in radians
   @property ic/p-rad_sec (read/write) Roll rate initial condition in radians/second
   @property ic/q-rad_sec (read/write) Pitch rate initial condition in radians/second
   @property ic/r-rad_sec (read/write) Yaw rate initial condition in radians/second

   @author Tony Peden
   @version "$Id: FGInitialCondition.h,v 1.27 2011/05/20 00:47:03 bcoconni Exp $"
*/

/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
CLASS DECLARATION
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/

class FGInitialCondition : public FGJSBBase, public FGXMLFileRead
{
public:
  /// Constructor
  FGInitialCondition(FGFDMExec *fdmex);
  /// Destructor
  ~FGInitialCondition();

  /** Set calibrated airspeed initial condition in knots.
      @param vc Calibrated airspeed in knots  */
  void SetVcalibratedKtsIC(double vc);

  /** Set equivalent airspeed initial condition in knots.
      @param ve Equivalent airspeed in knots  */
  void SetVequivalentKtsIC(double ve);

  /** Set true airspeed initial condition in knots.
      @param vt True airspeed in knots  */
  void SetVtrueKtsIC(double vtrue) { SetVtrueFpsIC(vtrue*ktstofps); }

  /** Set ground speed initial condition in knots.
      @param vg Ground speed in knots  */
  void SetVgroundKtsIC(double vg) { SetVgroundFpsIC(vg*ktstofps); }

  /** Set mach initial condition.
      @param mach Mach number  */
  void SetMachIC(double mach);

  /** Sets angle of attack initial condition in degrees.
      @param a Alpha in degrees */
  void SetAlphaDegIC(double a) { SetAlphaRadIC(a*degtorad); }

  /** Sets angle of sideslip initial condition in degrees.
      @param B Beta in degrees */
  void SetBetaDegIC(double b) { SetBetaRadIC(b*degtorad);}

  /** Sets pitch angle initial condition in degrees.
      @param theta Theta (pitch) angle in degrees */
  void SetThetaDegIC(double theta) { SetThetaRadIC(theta*degtorad); }

  /** Resets the IC data structure to new values
      @param u, v, w, ... **/
  void ResetIC(double u0, double v0, double w0, double p0, double q0, double r0,
               double alpha0, double beta0, double phi0, double theta0, double psi0,
               double latitudeRad0, double longitudeRad0, double altitudeAGL0,
               double gamma0);

  /** Sets the roll angle initial condition in degrees.
      @param phi roll angle in degrees */
  void SetPhiDegIC(double phi)  { SetPhiRadIC(phi*degtorad);}

  /** Sets the heading angle initial condition in degrees.
      @param psi Heading angle in degrees */
  void SetPsiDegIC(double psi){ SetPsiRadIC(psi*degtorad); }

  /** Sets the climb rate initial condition in feet/minute.
      @param roc Rate of Climb in feet/minute  */
  void SetClimbRateFpmIC(double roc) { SetClimbRateFpsIC(roc/60.0); }

  /** Sets the flight path angle initial condition in degrees.
      @param gamma Flight path angle in degrees  */
  void SetFlightPathAngleDegIC(double gamma)
  { SetClimbRateFpsIC(vt*sin(gamma*degtorad)); }

  /** Sets the altitude above sea level initial condition in feet.
      @param altitudeASL Altitude above sea level in feet */
  void SetAltitudeASLFtIC(double altitudeASL);

  /** Sets the initial Altitude above ground level.
      @param agl Altitude above ground level in feet */
  void SetAltitudeAGLFtIC(double agl)
  { SetAltitudeASLFtIC(terrain_elevation + agl); }

  /** Sets the initial sea level radius from planet center
      @param sl_rad sea level radius in feet */
  void SetSeaLevelRadiusFtIC(double sl_rad) { sea_level_radius = sl_rad; }

  /** Sets the initial terrain elevation.
      @param elev Initial terrain elevation in feet */
  void SetTerrainElevationFtIC(double elev) { terrain_elevation = elev; }

  /** Sets the initial latitude.
      @param lat Initial latitude in degrees */
  void SetLatitudeDegIC(double lat) { position.SetLatitude(lat*degtorad); }

  /** Sets the initial longitude.
      @param lon Initial longitude in degrees */
  void SetLongitudeDegIC(double lon) { position.SetLongitude(lon*degtorad); }

  /** Gets the initial calibrated airspeed.
      @return Initial calibrated airspeed in knots */
  double GetVcalibratedKtsIC(void) const;

  /** Gets the initial equivalent airspeed.
      @return Initial equivalent airspeed in knots */
  double GetVequivalentKtsIC(void) const;

  /** Gets the initial ground speed.
      @return Initial ground speed in knots */
  double GetVgroundKtsIC(void) const { return GetVgroundFpsIC() * fpstokts; }

  /** Gets the initial true velocity.
      @return Initial true airspeed in knots. */
  double GetVtrueKtsIC(void) const { return vt*fpstokts; }

  /** Gets the initial mach.
      @return Initial mach number */
  double GetMachIC(void) const;

  /** Gets the initial climb rate.
      @return Initial climb rate in feet/minute */
  double GetClimbRateFpmIC(void) const
  { return GetClimbRateFpsIC()*60; }

  /** Gets the initial flight path angle.
      @return Initial flight path angle in degrees */
  double GetFlightPathAngleDegIC(void) const
  { return GetFlightPathAngleRadIC()*radtodeg; }

  /** Gets the initial angle of attack.
      @return Initial alpha in degrees */
  double GetAlphaDegIC(void) const { return alpha*radtodeg; }

  /** Gets the initial sideslip angle.
      @return Initial beta in degrees */
  double GetBetaDegIC(void) const  { return beta*radtodeg; }

  /** Gets the initial pitch angle.
      @return Initial pitch angle in degrees */
  double GetThetaDegIC(void) const { return theta*radtodeg; }

  /** Gets the initial roll angle.
      @return Initial phi in degrees */
  double GetPhiDegIC(void) const { return phi*radtodeg; }

  /** Gets the initial heading angle.
      @return Initial psi in degrees */
  double GetPsiDegIC(void) const { return psi*radtodeg; }

  /** Gets the initial latitude.
      @return Initial geocentric latitude in degrees */
  double GetLatitudeDegIC(void) const { return position.GetLatitudeDeg(); }

  /** Gets the initial longitude.
      @return Initial longitude in degrees */
  double GetLongitudeDegIC(void) const { return position.GetLongitudeDeg(); }

  /** Gets the initial altitude above sea level.
      @return Initial altitude in feet. */
  double GetAltitudeASLFtIC(void) const { return position.GetRadius() - sea_level_radius; }

  /** Gets the initial altitude above ground level.
      @return Initial altitude AGL in feet */
  double GetAltitudeAGLFtIC(void) const { return position.GetRadius() - sea_level_radius - terrain_elevation; }

  /** Gets the initial sea level radius.
      @return Initial sea level radius */
  double GetSeaLevelRadiusFtIC(void) const { return sea_level_radius; }

  /** Gets the initial terrain elevation.
      @return Initial terrain elevation in feet */
  double GetTerrainElevationFtIC(void) const { return terrain_elevation; }

  /** Sets the initial ground speed.
      @param vg Initial ground speed in feet/second */
  void SetVgroundFpsIC(double vg);

  /** Sets the initial true airspeed.
      @param vt Initial true airspeed in feet/second */
  void SetVtrueFpsIC(double vt);

  /** Sets the initial body axis X velocity.
      @param ubody Initial X velocity in feet/second */
  void SetUBodyFpsIC(double ubody) { SetBodyVelFpsIC(eU, ubody); }

  /** Sets the initial body axis Y velocity.
      @param vbody Initial Y velocity in feet/second */
  void SetVBodyFpsIC(double vbody) { SetBodyVelFpsIC(eV, vbody); }

  /** Sets the initial body axis Z velocity.
      @param wbody Initial Z velocity in feet/second */
  void SetWBodyFpsIC(double wbody) { SetBodyVelFpsIC(eW, wbody); }

  /** Sets the initial local axis north velocity.
      @param vn Initial north velocity in feet/second */
  void SetVNorthFpsIC(double vn) { SetNEDVelFpsIC(eU, vn); }

  /** Sets the initial local axis east velocity.
      @param ve Initial east velocity in feet/second */
  void SetVEastFpsIC(double ve) { SetNEDVelFpsIC(eV, ve); }

  /** Sets the initial local axis down velocity.
      @param vd Initial down velocity in feet/second */
  void SetVDownFpsIC(double vd) { SetNEDVelFpsIC(eW, vd); }

  /** Sets the initial roll rate.
      @param P Initial roll rate in radians/second */
  void SetPRadpsIC(double P)  { p = P; }

  /** Sets the initial pitch rate.
      @param Q Initial pitch rate in radians/second */
  void SetQRadpsIC(double Q) { q = Q; }

  /** Sets the initial yaw rate.
      @param R initial yaw rate in radians/second */
  void SetRRadpsIC(double R) { r = R; }

  /** Sets the initial wind velocity.
      @param wN Initial wind velocity in local north direction, feet/second
      @param wE Initial wind velocity in local east direction, feet/second
      @param wD Initial wind velocity in local down direction, feet/second   */
  void SetWindNEDFpsIC(double wN, double wE, double wD);

  /** Sets the initial total wind speed.
      @param mag Initial wind velocity magnitude in knots */
  void SetWindMagKtsIC(double mag);

  /** Sets the initial wind direction.
      @param dir Initial direction wind is coming from in degrees */
  void SetWindDirDegIC(double dir);

  /** Sets the initial headwind velocity.
      @param head Initial headwind speed in knots */
  void SetHeadWindKtsIC(double head);

  /** Sets the initial crosswind speed.
      @param cross Initial crosswind speed, positive from left to right */
  void SetCrossWindKtsIC(double cross);

  /** Sets the initial wind downward speed.
      @param wD Initial downward wind speed in knots*/
  void SetWindDownKtsIC(double wD);

  /** Sets the initial climb rate.
      @param roc Initial Rate of climb in feet/second */
  void SetClimbRateFpsIC(double roc);

  /** Gets the initial ground velocity.
      @return Initial ground velocity in feet/second */
  double GetVgroundFpsIC(void) const  { return vUVW_NED.Magnitude(eU, eV); }

  /** Gets the initial true velocity.
      @return Initial true velocity in feet/second */
  double GetVtrueFpsIC(void) const { return vt; }

  /** Gets the initial body axis X wind velocity.
      @return Initial body axis X wind velocity in feet/second */
  double GetWindUFpsIC(void) const { return GetBodyWindFpsIC(eU); }

  /** Gets the initial body axis Y wind velocity.
      @return Initial body axis Y wind velocity in feet/second */
  double GetWindVFpsIC(void) const { return GetBodyWindFpsIC(eV); }

  /** Gets the initial body axis Z wind velocity.
      @return Initial body axis Z wind velocity in feet/second */
  double GetWindWFpsIC(void) const { return GetBodyWindFpsIC(eW); }

  /** Gets the initial wind velocity in local frame.
      @return Initial wind velocity toward north in feet/second */
  double GetWindNFpsIC(void) const { return GetNEDWindFpsIC(eX); }

  /** Gets the initial wind velocity in local frame.
      @return Initial wind velocity eastwards in feet/second */
  double GetWindEFpsIC(void) const { return GetNEDWindFpsIC(eY); }

  /** Gets the initial wind velocity in local frame.
      @return Initial wind velocity downwards in feet/second */
  double GetWindDFpsIC(void) const { return GetNEDWindFpsIC(eZ); }

  /** Gets the initial total wind velocity in feet/sec.
      @return Initial wind velocity in feet/second */
  double GetWindFpsIC(void)  const;

  /** Gets the initial wind direction.
      @return Initial wind direction in feet/second */
  double GetWindDirDegIC(void) const;

  /** Gets the initial climb rate.
      @return Initial rate of climb in feet/second */
  double GetClimbRateFpsIC(void) const
  {
    FGColumnVector3 _vt_NED = Tb2l * Tw2b * FGColumnVector3(vt, 0., 0.);
    return _vt_NED(eW);
  }

  /** Gets the initial body axis X velocity.
      @return Initial body axis X velocity in feet/second. */
  double GetUBodyFpsIC(void) const { return GetBodyVelFpsIC(eU); }

  /** Gets the initial body axis Y velocity.
      @return Initial body axis Y velocity in feet/second. */
  double GetVBodyFpsIC(void) const { return GetBodyVelFpsIC(eV); }

  /** Gets the initial body axis Z velocity.
      @return Initial body axis Z velocity in feet/second. */
  double GetWBodyFpsIC(void) const { return GetBodyVelFpsIC(eW); }

  /** Gets the initial local frame X (North) velocity.
      @return Initial local frame X (North) axis velocity in feet/second. */
  double GetVNorthFpsIC(void) const { return vUVW_NED(eU); }

  /** Gets the initial local frame Y (East) velocity.
      @return Initial local frame Y (East) axis velocity in feet/second. */
  double GetVEastFpsIC(void) const { return vUVW_NED(eV); }

  /** Gets the initial local frame Z (Down) velocity.
      @return Initial local frame Z (Down) axis velocity in feet/second. */
  double GetVDownFpsIC(void) const { return vUVW_NED(eW); }

  /** Gets the initial body axis roll rate.
      @return Initial body axis roll rate in radians/second */
  double GetPRadpsIC() const { return p; }

  /** Gets the initial body axis pitch rate.
      @return Initial body axis pitch rate in radians/second */
  double GetQRadpsIC() const { return q; }

  /** Gets the initial body axis yaw rate.
      @return Initial body axis yaw rate in radians/second */
  double GetRRadpsIC() const { return r; }

  /** Sets the initial flight path angle.
      @param gamma Initial flight path angle in radians */
  void SetFlightPathAngleRadIC(double gamma)
  { SetClimbRateFpsIC(vt*sin(gamma)); }

  /** Sets the initial angle of attack.
      @param alpha Initial angle of attack in radians */
  void SetAlphaRadIC(double alpha);

  /** Sets the initial pitch angle.
      @param theta Initial pitch angle in radians */
  void SetThetaRadIC(double theta);

  /** Sets the initial sideslip angle.
      @param beta Initial angle of sideslip in radians. */
  void SetBetaRadIC(double beta);

  /** Sets the initial roll angle.
      @param phi Initial roll angle in radians */
  void SetPhiRadIC(double phi);

  /** Sets the initial heading angle.
      @param psi Initial heading angle in radians */
  void SetPsiRadIC(double psi);

  /** Sets the initial latitude.
      @param lat Initial latitude in radians */
  void SetLatitudeRadIC(double lat) { position.SetLatitude(lat); }

  /** Sets the initial longitude.
      @param lon Initial longitude in radians */
  void SetLongitudeRadIC(double lon) { position.SetLongitude(lon); }

  /** Sets the target normal load factor.
      @param nlf Normal load factor*/
  void SetTargetNlfIC(double nlf) { targetNlfIC=nlf; }

  /** Gets the initial flight path angle.
      If total velocity is zero, this function returns zero.
      @return Initial flight path angle in radians */
  double GetFlightPathAngleRadIC(void) const
  { return (vt == 0.0)?0.0:asin(GetClimbRateFpsIC() / vt); }

  /** Gets the initial angle of attack.
      @return Initial alpha in radians */
  double GetAlphaRadIC(void) const { return alpha; }

  /** Gets the initial angle of sideslip.
      @return Initial sideslip angle in radians */
  double GetBetaRadIC(void) const { return beta; }

  /** Gets the initial roll angle.
      @return Initial roll angle in radians */
  double GetPhiRadIC(void) const { return phi; }

  /** Gets the initial latitude.
      @return Initial latitude in radians */
  double GetLatitudeRadIC(void) const { return position.GetLatitude(); }

  /** Gets the initial longitude.
      @return Initial longitude in radians */
  double GetLongitudeRadIC(void) const { return position.GetLongitude(); }

  /** Gets the initial pitch angle.
      @return Initial pitch angle in radians */
  double GetThetaRadIC(void) const { return theta; }

  /** Gets the initial heading angle.
      @return Initial heading angle in radians */
  double GetPsiRadIC(void) const   { return psi; }

  /** Gets the initial speedset.
      @return Initial speedset */
  speedset GetSpeedSet(void) const { return lastSpeedSet; }

  /** Gets the target normal load factor set from IC.
      @return target normal load factor set from IC*/
  double GetTargetNlfIC(void) const { return targetNlfIC; }

  /** Loads the initial conditions.
      @param rstname The name of an initial conditions file
      @param useStoredPath true if the stored path to the IC file should be used
      @return true if successful */
  bool Load(string rstname, bool useStoredPath = true );

  /** Get init-file name
  */
  string GetInitFile(void) const {return init_file_name;}
  /** Set init-file name
  */
  void SetInitFile(string f) { init_file_name = f;}
  void WriteStateFile(int num);

private:
  FGColumnVector3 vUVW_NED;
  FGLocation position;
  double vt;
  double p,q,r;
  double sea_level_radius;
  double terrain_elevation;
  double targetNlfIC;

  FGMatrix33 Tw2b, Tb2w;
  FGMatrix33 Tl2b, Tb2l;
  double  alpha, beta, theta, phi, psi;

  speedset lastSpeedSet;

  FGFDMExec *fdmex;
  FGPropertyManager *PropertyManager;

  bool Load_v1(void);
  bool Load_v2(void);

  bool Constructing;

  void InitializeIC(void);
  void SetBodyVelFpsIC(int idx, double vel);
  void SetNEDVelFpsIC(int idx, double vel);
  double GetBodyWindFpsIC(int idx) const;
  double GetNEDWindFpsIC(int idx) const;
  double GetBodyVelFpsIC(int idx) const;
  double getMachFromVcas(double vcas);
  double calcVcas(double Mach) const;
  void calcAeroAngles(const FGColumnVector3& _vt_BODY);
  void calcThetaBeta(double alfa, const FGColumnVector3& _vt_NED);
  void bind(void);
  void Debug(int from);

  string init_file_name;
};
}
#endif

