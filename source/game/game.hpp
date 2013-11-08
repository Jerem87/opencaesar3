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


#ifndef __OPENCAESAR3_APPLICATION_H_INCLUDED__
#define __OPENCAESAR3_APPLICATION_H_INCLUDED__

#include "core/scopedptr.hpp"
#include "core/predefinitions.hpp"
#include "core/signals.hpp"

#include <string>

class GfxEngine;

class Game
{
public:
  Game();
  ~Game();

  void save(std::string filename) const;
  void load(std::string filename);

  void initialize();

  void exec();

  void reset();

  void initSound();
  void mountArchives();

  void setScreenWait();
  void setScreenMenu();
  void setScreenGame();

  PlayerPtr getPlayer() const;
  CityPtr getCity() const;
  EmpirePtr getEmpire() const;
  gui::GuiEnv* getGui() const;
  GfxEngine* getEngine() const;

  void setPaused( bool value );
  bool isPaused() const;

  void play();
  void pause();

  void changeTimeMultiplier(int percent);
  void setTimeMultiplier( int percent );
  int getTimeMultiplier() const;

public oc3_slots:
  void resolveEvent( events::GameEventPtr event );

public oc3_signals:
  Signal1<std::string>& onSaveAccepted();

private:
  class Impl;
  ScopedPtr< Impl > _d;
};


#endif //__OPENCAESAR3_APPLICATION_H_INCLUDED__
