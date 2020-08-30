/**
 * @file input.c
 * @author Joshua Crotts
 * @date June 18 2020
 * @version 1.0
 *
 * @section LICENSE
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * @section DESCRIPTION
 *
 * This file defines input handling events. As of now, it only supports mouse and keyboard,
 * but controller support may come at a later date.
 */
#include "../include/input.h"

static int16_t previous_frame_key = -1;

static inline void Stds_UpdateMouseState( void );
static inline void Stds_KeyPressed( const SDL_Event *event );
static inline void Stds_KeyReleased( const SDL_KeyboardEvent *event );
static inline void Stds_MousePressed( const SDL_MouseButtonEvent *event );
static inline void Stds_MouseReleased( const SDL_MouseButtonEvent *event );
static inline void Stds_MouseMoved( const SDL_MouseMotionEvent *event );
static void        Stds_UpdateTextFields( const SDL_Event *event );

/**
 * Starts the SDL event loop.
 *
 * @param void.
 *
 * @return void.
 */
void
Stds_ProcessInput( void ) {
  SDL_Event event;
  SDL_StartTextInput();

  while ( SDL_PollEvent( &event ) ) {
    Stds_UpdateMouseState();

    switch ( event.type ) {
    case SDL_QUIT:
      SDL_LogDebug( SDL_LOG_CATEGORY_APPLICATION, "Quit event." );
      Stds_Quit();
      exit( EXIT_SUCCESS );
      break;
    case SDL_KEYDOWN:
      Stds_KeyPressed( &event );
      break;
    case SDL_KEYUP:
      Stds_KeyReleased( &event.key );
      break;
    case SDL_MOUSEBUTTONDOWN:
      Stds_MousePressed( &event.button );
      break;
    case SDL_MOUSEBUTTONUP:
      Stds_MouseReleased( &event.button );
      break;
    case SDL_MOUSEWHEEL:
      g_app.mouse.wheel = event.wheel.y;
      break;
    case SDL_MOUSEMOTION:
      Stds_MouseMoved( &event.motion );
      break;
    case SDL_TEXTINPUT:
      Stds_UpdateTextFields( &event );
      break;
    default:
      break;
    }
  }
}

/**
 *
 *
 * @param
 *
 * @param void.
 */
static void
Stds_UpdateTextFields( const SDL_Event *event ) {
  for ( struct text_field_t *tf = &g_app.text_field_head; tf != NULL; tf = tf->next ) {
    if ( tf->toggle_text_input ) {
      SDL_StartTextInput();
      Stds_ReadTextField( tf, event );
    } else {
      SDL_StopTextInput();
    }
  }
}

/**
 *
 *
 * @param void.
 *
 * @return void.
 */
static inline void
Stds_UpdateMouseState( void ) {
  SDL_GetMouseState( &g_app.mouse.x, &g_app.mouse.y );
  g_app.mouse.is_moving = false;
}

/**
 *
 *
 * @param
 *
 * @return void.
 */
static inline void
Stds_KeyPressed( const SDL_Event *event ) {
  if ( event->key.repeat == 0 && event->key.keysym.scancode < MAX_KEYBOARD_KEYS ) {
    g_app.keyboard[event->key.keysym.scancode] = 1;
  }

  /* Special case for backspace. */
  if ( g_app.keyboard[SDL_SCANCODE_BACKSPACE] ) {
    Stds_UpdateTextFields( event );
  }
}

/**
 *
 *
 * @param
 *
 * @return void.
 */
static inline void
Stds_KeyReleased( const SDL_KeyboardEvent *event ) {
  if ( event->repeat == 0 && event->keysym.scancode < MAX_KEYBOARD_KEYS ) {
    g_app.keyboard[event->keysym.scancode] = 0;
  }
}

/**
 *
 *
 * @param
 *
 * @return void.
 */
static inline void
Stds_MousePressed( const SDL_MouseButtonEvent *event ) {
  g_app.mouse.button[event->button] = 1;
}

/**
 *
 */
static inline void
Stds_MouseReleased( const SDL_MouseButtonEvent *event ) {
  g_app.mouse.button[event->button] = 0;
}

/**
 * @TODO: Fix this, it does NOT work.
 */
static inline void
Stds_MouseMoved( const SDL_MouseMotionEvent *e ) {
  g_app.mouse.is_moving = true;
}