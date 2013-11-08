// This file is part of openCaesar3.
//
// openCaesar3 is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// openCaesar3 is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with openCaesar3.  If not, see <http://www.gnu.org/licenses/>.
//
// Copyright 2012-2013 Gregoire Athanase, gathanase@gmail.com


#ifndef _OPENCAESAR_BUILDING_H_INCLUDE_
#define _OPENCAESAR_BUILDING_H_INCLUDE_

#include <string>
#include <map>
#include <list>
#include <set>

#include "game/construction.hpp"
#include "gfx/tileoverlay.hpp"
#include "game/enums.hpp"
#include "game/good.hpp"
#include "core/scopedptr.hpp"
#include "gfx/animation.hpp"
#include "core/referencecounted.hpp"
#include "core/predefinitions.hpp"
#include "game/service.hpp"
#include "walker/constants.hpp"

class Building : public Construction
{
public:
  Building(const Type type, const Size& size=Size(1) );
  virtual void initTerrain(Tile& terrain);

  virtual void timeStep(const unsigned long time);
  virtual void storeGoods(GoodStock &stock, const int amount = -1);
  // evaluate the given service
  virtual float evaluateService(ServiceWalkerPtr walker);
  // handle service reservation
  void reserveService(const Service::Type service);
  void cancelService(const Service::Type service);
  virtual void applyService( ServiceWalkerPtr walker);
  // evaluate the need for the given trainee
  virtual float evaluateTrainee( constants::walker::Type traineeType);  // returns >0 if trainee is needed
  void reserveTrainee( constants::walker::Type traineeType); // trainee will come
  void cancelTrainee( constants::walker::Type traineeType);  // trainee will not come
  void applyTrainee( constants::walker::Type traineeType); // trainee arrives

protected:
  float _damageIncrement;
  float _fireIncrement;
  typedef std::map< constants::walker::Type, int> TraineeMap;
  std::set<Service::Type> _reservedServices;  // a serviceWalker is on the way
  TraineeMap _traineeMap;  // current level of trainees working in the building (0..200)
  std::set< constants::walker::Type > _reservedTrainees;  // a trainee is on the way
};

class SmallStatue : public Building
{
public:
  SmallStatue();
  bool isNeedRoadAccess() const;
};

class MediumStatue : public Building
{
public:
  MediumStatue();
  bool isNeedRoadAccess() const;
};

class BigStatue : public Building
{
public:
  BigStatue();
  bool isNeedRoadAccess() const;
};

class Shipyard : public Building
{
public:
  Shipyard();
};

class TriumphalArch : public Building
{
public:
  TriumphalArch();
};

class Dock : public Building
{
public:
  Dock();
  void timeStep(const unsigned long time);
};

#endif
