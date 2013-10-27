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

#include "mainmenu.hpp"
#include "contextmenuitem.hpp"
#include "core/event.hpp"
#include "environment.hpp"

namespace gui
{
#define DEFAULT_MENU_HEIGHT 15

//! constructor
MainMenu::MainMenu( Widget* parent, const Rect& rectangle, const int id )
	: ContextMenu( parent, rectangle, id, false, true)
{
  setCloseHandling( cmIgnore );
#ifdef _DEBUG
	setDebugName( "MainMenu" );
#endif

	//const ElementStyle& style = getStyle().getSubStyle( NES_ITEM );
	setTextAlignment( alignCenter, alignCenter );

	setNotClipped(false);

	recalculateSize_();
}

ContextMenuItem* MainMenu::addItem(const std::string& text, int commandId, bool enabled, bool hasSubMenu, bool checked, bool autoChecking)
{
    ContextMenuItem* ret = ContextMenu::addItem( text, commandId, enabled, hasSubMenu, checked, autoChecking );
    if( ret && ret->getSubMenu() )
    {
      //ret->getSubMenu()->setStyle( getStyle().getSubStyle( NES_SUBMENU ).getName() );
		  ret->setFlag( ContextMenuItem::drawSubmenuSprite, false );
      ret->setBackgroundMode( Label::bgNone ); 
    }
    //refItem.alignEnabled = true;
    //refItem.horizontal = EGUIA_CENTER;
    //refItem.vertical = EGUIA_CENTER;

    return ret;
}

//! called if an event happened.
bool MainMenu::onEvent(const NEvent& event)
{
	if (isEnabled())
	{
		switch(event.EventType)
		{
		case sEventGui:
			switch(event.GuiEvent.EventType)
			{
			case guiElementFocusLost:
				if (event.GuiEvent.Caller == this && !isMyChild(event.GuiEvent.Element))
				{
					closeAllSubMenus_();
					setHoverIndex_( -1 );
				}
				break;
			case guiElementFocused:
				if (event.GuiEvent.Caller == this )
				{
					bringToFront();
				}
				break;
			default:
				break;
			}
			break;
		case sEventMouse:
			switch(event.MouseEvent.Event)
			{
			case mouseLbtnPressed:
			{
				if (!getEnvironment()->hasFocus(this))
				{
					getEnvironment()->setFocus(this);
				}

    	  bringToFront();

				Point p(event.MouseEvent.getPosition() );
				bool shouldCloseSubMenu = hasOpenSubMenu_();
				if (!getAbsoluteClippingRect().isPointInside(p))
				{
					shouldCloseSubMenu = false;
				}
				isHighlighted_( event.MouseEvent.getPosition(), true);
				if ( shouldCloseSubMenu )
				{
          getEnvironment()->removeFocus(this);
				}

				return true;
			}

			case mouseLbtnRelease:
			{
        Point p(event.MouseEvent.getPosition() );
				if (!getAbsoluteClippingRect().isPointInside(p))
				{
					int t = sendClick_(p);
					if ((t==0 || t==1) && isFocused())
						removeFocus();
				}

			  return true;
			}

      case mouseMoved:
      {
				if (getEnvironment()->hasFocus(this) && getHoveredIndex() >= 0)
				{
				  int oldHighLighted = getHoveredIndex();
					isHighlighted_( event.MouseEvent.getPosition(), true);
					if ( getHoveredIndex() < 0 )
          {
            setHoverIndex_( oldHighLighted );   // keep last hightlight active when moving outside the area
          }
				}
				return true;
      }

      default:
			break;
			}
		break;
		
    default:
		break;
		}
	}

	return Widget::onEvent(event);
}

void MainMenu::recalculateSize_()
{
	Rect parentRect = getParent()->getClientRect(); // client rect of parent  

	//AbstractSkin* skin = getEnvironment()->getSkin();
  Font font = Font::create( FONT_2_WHITE );
  
  int height = std::max<int>( DEFAULT_MENU_HEIGHT, getHeight() );
	setGeometry( Rect( parentRect.UpperLeftCorner.getX(), parentRect.UpperLeftCorner.getY(),
                     parentRect.LowerRightCorner.getX(), parentRect.UpperLeftCorner.getY() + height ) );
	Rect rect;

  rect.UpperLeftCorner = parentRect.UpperLeftCorner;
  height = std::max<int>( font.getSize("A").getHeight(), height );
	//if (skin && height < skin->getSize ( EGDS_MENU_HEIGHT ))
	//	height = skin->getSize(EGDS_MENU_HEIGHT);
	int width = rect.UpperLeftCorner.getX();
	int i;

	for( i=0; i<(int)getItemCount(); ++i)
	{
    ContextMenuItem* refItem = getItem( i );
		if ( refItem->isSeparator() )
		{
			refItem->setDim( Size( 16, getHeight() ) );
		}
		else
		{
      Size itemSize = font.getSize( refItem->getText() ) + Size( 20, 0 );
      itemSize.setHeight( getHeight() );
			refItem->setDim( itemSize );
		}

		refItem->setOffset( width );
		width += refItem->getDim().getWidth();
	}

	// recalculate submenus
	for (i=0; i<(int)getItemCount(); ++i)
  {
    ContextMenuItem* refItem = getItem( i );

    Rect rectangle( refItem->getOffset(), 0, refItem->getOffset() + refItem->getDim().getWidth(), height );
    refItem->setGeometry( rectangle );

		if (refItem->getSubMenu())
		{
			// move submenu
      Size itemSize = refItem->getSubMenu()->getAbsoluteRect().getSize();

			refItem->getSubMenu()->setGeometry( Rect( refItem->getOffset(), height,
					                          						refItem->getOffset() + itemSize.getWidth()-5, height+itemSize.getHeight() ));
		}
  }
}

}//end namespace gui
