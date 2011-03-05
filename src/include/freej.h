/*  FreeJ
 *  (c) Copyright 2001-2010 Denis Roio <jaromil@dyne.org>
 *
 * This source code is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Public License as published
 * by the Free Software Foundation; either version 3 of the License,
 * or (at your option) any later version.
 *
 * This source code is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * Please refer to the GNU Public License for more details.
 *
 * You should have received a copy of the GNU Public License along with
 * this source code; if not, write to:
 * Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/** @mainpage FreeJ - vision mixer
 
    @section sec_intro Introduction

    FreeJ is  a commandline application  on GNU/Linux and  a graphical
    application on  Apple/OSX -  and besides that,  it provides  a C++
    library with  API for multimedia frameworks. FreeJ  code relies on
    portable  POSIX code  and  different native  functions present  on
    various operating systems, triggered at compile time (for instance
    uses FFMpeg on  GNU/Linux and Quicktime components on Apple/OSX).

    Using the libfreej library  all these internal implementations are
    operable  using the same  calls on  all different  functions: this
    documentation is intended to  give an introduction and overview on
    its API.

    @section sec_architecture Architecture

    The FreeJ engine is organized in a tree structure seen as:

    @verbatim
                                   
   ViewPort---(Blit)______________Layer_____Filter 
           \   vvvv                    \____Filter 
            \-(Blit)____Layer__Filter   \___Filter
             \ vvvv         \__Filter        ...
              (Blit)__Layer  \_Filter
                              \Filter
                                ...
    @endverbatim

    Every ViewPort  holds a list  of Layer objects, cycling  thru them
    during FreeJ's execution.

    Each Layer holds a list of instantiated Filter objects.

    A Filter is a plugin loaded and served by the Plugger.

    A Blit is the operation used to sum Layers into the ViewPort

    At last, there can be as many ViewPorts as you want (each with its
    own output  implementation at screen  or via special  devices) and
    their list is kept by the global Context.

    @section sec_makelayer Make a new Layer

    If you are implementing a new  Layer (or you simply want to use an
    existing  Layer even outside  the FreeJ  Context, still  using its
    library) you need  to take into account just  a few methods common
    to all Layer implementations:


    @verbatim

    bool open(char *file)  called to open MyLayer's source
                           returns false if is not accessible

    bool init(Context *scr)  if the open was successful, call this and
			     the Layer will enter the chain

    bool feed()   this function is executed by the Context when
                  it needs to grab more data in the Layer

    bool close()  you need to call this when you want to close
                  the Layer, in case you initialized it

   @endverbatim

   If you are  implementing a new Layer, you don't  need to care about
   threading or synchronizing the execution: just be sure to correctly
   free all the buffers you allocate ;^)

   @section sec_tobecontinued To Be Continued...
   
   This documentation needs to be completed but it's already useful as
   various coders found their way  through the headers for the missing
   bits. We  are actively documenting  the code! feel free  to contact
   our mailinglist if  you want to speed up the  process and point out
   parts you need to have documented ASAP.


 */
