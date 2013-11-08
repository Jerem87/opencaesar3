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

#include "film_widget.hpp"
#include "game/settings.hpp"
#include "pushbutton.hpp"
#include "core/event.hpp"
#include "core/stringhelper.hpp"
#include "core/foreach.hpp"
#include "texturedbutton.hpp"
#include "label.hpp"
#include "core/logger.hpp"

namespace gui
{

class FilmWidget::Impl
{
public:
  Label* lbTitle;
  TexturedButton* btnExit;
  Label* lbTime;
  Label* lbReceiver;
  Label* lbMessage;

public oc3_signals:
  Signal0<> onCloseSignal;
};

FilmWidget::FilmWidget(Widget* parent, io::FilePath film )
  : Widget( parent, -1, Rect( 0, 0, 1, 1 ) ), _d( new Impl )
{
  _d->lbMessage = 0;

  setupUI( GameSettings::rcpath( "/gui/filmwidget.gui" ) );

  setPosition( Point( parent->getWidth() - getWidth(), parent->getHeight() - getHeight() ) / 2 );

  _d->lbTitle = findChild<Label*>( "lbTitle", true );
  _d->btnExit = findChild<TexturedButton*>( "btnExit", true );
  _d->lbTime = findChild<Label*>( "lbTime", true );
  _d->lbReceiver = findChild<Label*>( "lbReceiver", true );
  _d->lbMessage = findChild<Label*>( "lbMessage", true );

  CONNECT( _d->btnExit, onClicked(), &_d->onCloseSignal, Signal0<>::emit );
  CONNECT( _d->btnExit, onClicked(), this, FilmWidget::deleteLater );
}

FilmWidget::~FilmWidget( void )
{
}

void FilmWidget::setText(const std::string& text)
{
  if( _d->lbMessage ) _d->lbMessage->setText( text );
}

void FilmWidget::setReceiver(std::string text)
{
  if( _d->lbReceiver ) _d->lbReceiver->setText( text );
}

void FilmWidget::setTitle(std::string text)
{
  if( _d->lbTitle ) _d->lbTitle->setText( text );
}

void FilmWidget::setTime(DateTime time)
{
  if( _d->lbTime ) _d->lbTime->setText( StringHelper::format( 0xff, "%s %d %s",
                                                              DateTime::getMonthName( time.getMonth() ),
                                                              time.getYear(),
                                                              time.getYear() < 0 ? "BC" : "AD" ) );
}

Signal0<>& FilmWidget::onClose()
{
  return _d->onCloseSignal;
}


}//end namespace gui
