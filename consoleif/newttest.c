/*
 * newttest.c
 *
 *  Created on: 2013-09-21
 *      Author: Blair
 */

#include <newt.h>
#include <stdlib.h>
#include "opendash.h"

int main( int argc, char **argv )
{
    newtComponent form1, button;

    newtInit();
    newtCls();

    newtDrawRootText( 0, 0, "Newt testing program" );
    newtCenteredWindow( SCREEN_WIDTH - 2, SCREEN_HEIGHT - 2, "Hello, world!" );
    form1 = newtForm( NULL, NULL, 0 );
    button = newtButton( ( SCREEN_WIDTH - 16 ) / 2, ( SCREEN_HEIGHT - 4 ) / 2, "Hello, world!" );

    newtFormAddComponents( form1, button );
    newtRunForm( form1 );

    newtFormDestroy( form1 );
    newtFinished();
}
