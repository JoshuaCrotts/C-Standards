//=============================================================================================//
// FILENAME :       grid.c
//
// DESCRIPTION :
//        Defines the functions associated with grids, and is able to treat
//        each 'box' as a button and deals with events like clicking.
//
// NOTES :
//        Permission is hereby granted, free of charge, to any person obtaining a copy
//        of this software and associated documentation files (the "Software"), to deal
//        in the Software without restriction, including without limitation the rights
//        to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//        copies of the Software, and to permit persons to whom the Software is
//        furnished to do so, subject to the following conditions:
//
//        The above copyright notice and this permission notice shall be included in all
//        copies or substantial portions of the Software.
//
//        THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//        IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//        FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//        AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//        LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//        OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//        SOFTWARE.
//
// AUTHOR :   strah19        START DATE :    19 Jul 2020
//
//=============================================================================================//

#include "../include/grid.h"
#include "../include/animation.h"
#include "../include/draw.h"

static bool Stds_AssertGrid( struct grid_t *grid );

/**
 * Created grid with no texture, no collision, etc. This is useful for grids that have to change
 * colors overtime, or just don't have an accompanying texture, or other special feature.
 *
 * @param float x top-left x pos of button.
 * @param float y top-left y pos of button.
 * @param int32_t squareWidth width of each square.
 * @param int32_t squareHeight height of each square.
 * @param uint32_t cols number of columns the grid will have.
 * @param uint32_t rows number of rows the grid will have.
 * @param SDL_Color lineColor color of the lining of the grid.
 * @param SDL_Color fillColor color of the square's in the grid.
 *
 * @return grid_t pointer.
 */
struct grid_t *
Stds_CreateGrid( float x, float y, int32_t squareWidth, int32_t squareHeight, uint32_t cols,
                 uint32_t rows, SDL_Color lineColor, SDL_Color fillColor ) {
  struct grid_t *grid;
  grid = malloc( sizeof( struct grid_t ) );

  if ( grid == NULL ) {
    SDL_LogDebug( SDL_LOG_CATEGORY_APPLICATION,
                  "Error: could not allocate memory for grid_t, %s.\n", SDL_GetError() );
    exit( EXIT_FAILURE );
  } else {
    SDL_LogDebug( SDL_LOG_CATEGORY_APPLICATION, "Created grid_t.\n" );
  }

  memset( grid, 0, sizeof( struct grid_t ) );
  grid->x             = x;
  grid->y             = y;
  grid->sx            = x;
  grid->sy            = y;
  grid->sw            = squareWidth;
  grid->sh            = squareHeight;
  grid->cols          = cols;
  grid->rows          = rows;
  grid->lineColor     = lineColor;
  grid->fillColor     = fillColor;
  grid->textures      = NULL;
  grid->textureBuffer = 0;
  grid->spriteSheet   = NULL;
  grid->clip.x = grid->clip.y = grid->clip.w = grid->clip.h = 0;
  grid->spriteSheetCols                                     = 0;
  grid->spriteSheetRows                                     = 0;
  grid->animation       = Stds_VectorCreate( sizeof( struct animation_t * ) );
  grid->animationBuffer = -1;
  grid->isCameraOn = false;

  return grid;
}

/**
 * Draws lines for where the box's of the grid would be.
 *
 * @param grid_t* pointer to grid_t.
 *
 * @return void.
 */
void
Stds_DrawLineGrid( struct grid_t *grid ) {
  if ( Stds_AssertGrid( grid ) ) {
    grid->x = grid->sx;
    grid->y = grid->sy;

    for ( uint32_t r = 0; r < grid->rows; r++ ) {
      Stds_DrawLine( grid->x, grid->y, grid->x + ( float ) ( grid->sw * grid->cols ), grid->y,
                     &grid->lineColor );
      grid->y += ( float ) grid->sh;
    }

    grid->y = grid->sy;

    for ( uint32_t c = 0; c < grid->cols; c++ ) {
      Stds_DrawLine( grid->x, grid->y, grid->x, grid->y + ( float ) ( grid->sh * grid->rows ),
                     &grid->lineColor );
      grid->x += ( float ) grid->sw;
    }

    grid->x = grid->sx;

    Stds_DrawLine( grid->x, grid->y + ( float ) ( grid->sh * grid->rows ),
                   grid->x + ( float ) ( grid->sw * grid->cols ),
                   grid->y + ( float ) ( grid->sh * grid->rows ), &grid->lineColor );
    Stds_DrawLine( grid->x + ( float ) ( grid->sw * grid->cols ), grid->y,
                   grid->x + ( float ) ( grid->sw * grid->cols ),
                   grid->y + ( float ) ( grid->sh * grid->rows ), &grid->lineColor );
  }
}

/**
 * Fills the area of the grid with rectangles.
 *
 * @param grid_t* pointer to grid_t.
 *
 * @return void.
 */
void
Stds_FillWholeGrid( struct grid_t *grid ) {
  if ( Stds_AssertGrid( grid ) ) {
    grid->x = grid->sx;
    grid->y = grid->sy;

    SDL_FRect fillRect = {grid->x, grid->y, ( float ) grid->sw, ( float ) grid->sh};

    for ( uint32_t r = 0; r < grid->rows; r++ ) {
      for ( uint32_t c = 0; c < grid->cols; c++ ) {

        Stds_DrawRectF( &fillRect, &grid->fillColor, true, 0 );

        fillRect.x += ( float ) grid->sw;
      }
      fillRect.y += ( float ) grid->sh;
      fillRect.x = grid->sx;
    }
  }
}

/**
 * Cleans up the grid.
 *
 * @param grid_t* pointer to grid_t.
 *
 * @return void.
 */
void
Stds_FreeGrid( struct grid_t *grid ) {
  if ( grid->textures != NULL ) {
    for ( uint32_t textureIndex = 0; textureIndex < grid->textureBuffer; textureIndex++ ) {
      SDL_DestroyTexture( grid->textures[textureIndex] );
      SDL_LogDebug( SDL_LOG_CATEGORY_APPLICATION, "Freeing texture %d.\n", textureIndex );
    }
    free( grid->textures );
  }

  if ( grid->spriteSheet != NULL ) {
    SDL_DestroyTexture( grid->spriteSheet );
    SDL_LogDebug( SDL_LOG_CATEGORY_APPLICATION, "Freeing spritesheet.\n" );
  }

  Stds_VectorDestroy( grid->animation );

  memset( grid, 0, sizeof( struct grid_t ) ); /* Makes the grid be equal to NULL. */
  free( grid );
  SDL_LogDebug( SDL_LOG_CATEGORY_APPLICATION, "Freed grid_t.\n" );
}

/**
 * Checks if mouse is over any of the squares.
 *
 * @param grid_t* pointer to grid_t.
 *
 * @return grid_pair_t struct that holds data for what square is being hovered.
 */
struct grid_pair_t
Stds_OnGridHover( struct grid_t *grid ) {
  struct grid_pair_t p;

  if ( Stds_AssertGrid( grid ) ) {
    grid->x = grid->sx;
    grid->y = grid->sy;

    SDL_Rect hoverRect = {( int ) grid->x, ( int ) grid->y, ( int ) grid->sw, ( int ) grid->sh};

    /* Loops through each square. */
    for ( uint32_t r = 0; r < grid->rows; r++ ) {
      p.r = ( int32_t ) r;
      p.y = ( float ) hoverRect.y;
      for ( uint32_t c = 0; c < grid->cols; c++ ) {

        p.c = ( int32_t ) c;
        p.x = ( float ) hoverRect.x;

        if ( Stds_IsMouseOverRect( ( float ) app.mouse.x, ( float ) app.mouse.y, &hoverRect ) ) {
          return p;
        }
        hoverRect.x += ( int ) grid->sw;
      }

      hoverRect.y += ( int ) grid->sh;
      hoverRect.x = ( int ) grid->sx;
    }
  }

  /* -1 means that the mouse did not hover over any of the squares. */
  p.c = -1;
  p.r = -1;

  return p;
}

/**
 * Checks if the mouse clicked over any of the squares.
 *
 * @param grid_t* pointer to grid_t.
 * @param int32_t the code for which mous ebutton is clicked.
 *
 * @return grid_pair_t struct that holds data for what square is being clicked.
 */
struct grid_pair_t
Stds_OnGridClicked( struct grid_t *grid, int32_t mouseCode ) {
  struct grid_pair_t p;

  if ( Stds_AssertGrid( grid ) ) {
    grid->x = grid->sx;
    grid->y = grid->sy;

    SDL_Rect clickRect = {( int ) grid->x, ( int ) grid->y, ( int ) grid->sw, ( int ) grid->sh};

    /* Loops through each square */
    for ( uint32_t r = 0; r < grid->rows; r++ ) {
      p.r = ( int32_t ) r;
      p.y = ( float ) clickRect.y;
      for ( uint32_t c = 0; c < grid->cols; c++ ) {

        p.c = ( int32_t ) c;
        p.x = ( float ) clickRect.x;

        if ( Stds_IsMouseOverRect( ( float ) app.mouse.x, ( float ) app.mouse.y, &clickRect ) &&
             app.mouse.button[mouseCode] ) {
          app.mouse.button[mouseCode] = 0;
          return p;
        }

        clickRect.x += ( int ) grid->sw;
      }

      clickRect.y += ( int ) grid->sh;
      clickRect.x = ( int ) grid->sx;
    }
  }

  /* -1 means that the mouse did not hover over any of the squares */
  p.c = -1;
  p.r = -1;

  return p;
}

/**
 * Initializes enough space for the amount of textures wanted for the grid.
 *
 * @param grid_t* pointer to grid_t.
 * @param int32_t number of textures for the grid.
 *
 * @return void.
 */
void
Stds_InitializeGridTextures( struct grid_t *grid, int32_t textureBuffer ) {
  static bool onceCall = false;
  if ( Stds_AssertGrid( grid ) && !onceCall ) {
    grid->textures      = malloc( textureBuffer * sizeof( SDL_Texture * ) );
    grid->textureBuffer = textureBuffer;
    onceCall            = true;
  }
}

/**
 * Adds a single texture to the grid for use.
 *
 * @param grid_t* pointer to grid_t.
 * @param const char* filePath to texture.
 *
 * @return uint32_t ? What does this return?
 */
int32_t
Stds_AddGridTexture( struct grid_t *grid, const char *filePath ) {
  if ( Stds_AssertGrid( grid ) ) {
    int32_t currentTextureNum = -1;
    if ( currentTextureNum < grid->textureBuffer - 1 ) {
      currentTextureNum++;
      SDL_LogDebug( SDL_LOG_CATEGORY_APPLICATION, "Added texture %d to grid with path %s\n",
                    currentTextureNum, filePath );
      grid->textures[currentTextureNum] = Stds_LoadTexture( filePath );
      return currentTextureNum;
    } else {
      return -1;
    }
  }
  return -1;
}

/**
 * Will render the specified texture id onto the grid.
 *
 * @param grid_t* pointer to grid_t.
 * @param uint32_t which column texture will be put.
 * @param uint32_t which row texture will be put.
 * @param int32_t index in texture array.
 * @param SDL_RendererFlip will flip texture.
 * @param uint16_t the angle of the image.
 * 
 * @return void.
 */
void
Stds_PutGridTexture( struct grid_t *grid, uint32_t col, uint32_t row, int32_t index, SDL_RendererFlip flip, uint16_t angle ) {
  if ( Stds_AssertGrid( grid ) ) {
    if ( index < grid->textureBuffer && index > -1 ) {
      SDL_FRect texturePosition = {grid->x + ( float ) ( col * grid->sw ),
                                   grid->y + ( float ) ( row * grid->sh ), ( float ) grid->sw,
                                   ( float ) grid->sh};
      Stds_BlitTexture( grid->textures[index], NULL, texturePosition.x, texturePosition.y,
                        texturePosition.w, texturePosition.h, angle, flip, NULL, grid->isCameraOn );
    }
  }
}

/**
 * Adds a sprite sheet to the grid.
 *
 * @param grid_t* pointer to grid_t.
 * @param const char* filePath filePath to texture
 * @param uint32_t number of cols the sprite sheet will have.
 * @param uint32_t number of rows the sprite sheet will have.
 *
 * @return void.
 */
void
Stds_AddSpriteSheetToGrid( struct grid_t *grid, const char *filePath, uint32_t cols,
                           uint32_t rows ) {
  if ( Stds_AssertGrid( grid ) ) {
    bool once_call = false;
    if ( Stds_AssertGrid( grid ) && !once_call ) {
      SDL_LogDebug( SDL_LOG_CATEGORY_APPLICATION, "Added spriteSheet to grid with path %s\n",
                    filePath );
      grid->spriteSheet = Stds_LoadTexture( filePath );
      once_call         = true;
      SDL_QueryTexture( grid->spriteSheet, NULL, NULL, &grid->clip.w, &grid->clip.h );

      grid->clip.w /= cols;
      grid->clip.h /= rows;

      grid->spriteSheetCols = cols;
      grid->spriteSheetRows = rows;
    }
  }
}

/**
 * Select which sprite you want to use.
 *
 * @param grid_t* pointer to grid_t.
 * @param uint32_t which column the sprite is to be used.
 * @param uint32_t which row the sprite is to be used.
 *
 * @return void.
 */
void
Stds_SelectSpriteForGrid( struct grid_t *grid, uint32_t sheetCol, uint32_t sheetRow ) {
  if ( Stds_AssertGrid( grid ) && sheetCol < grid->spriteSheetCols &&
       sheetRow < grid->spriteSheetRows ) {
    grid->clip.x = sheetCol * grid->clip.w;
    grid->clip.y = sheetRow * grid->clip.h;
  }
}

/**
 * Will render the specified sprite selected from Stds_SelectSpriteForGrid.
 *
 * @param grid_t* pointer to grid_t.
 * @param uint32_t which column to render the specified sprite onto the grid.
 * @param uint32_t which row to render the specified sprite onto the grid.
 * @param SDL_RendererFlip will flip texture.
 * @param uint16_t the angle of the image.
 * 
 * @return void.
 */
void
Stds_DrawSelectedSpriteOnGrid( struct grid_t *grid, uint32_t gridCol, uint32_t gridRow, SDL_RendererFlip flip, uint16_t angle ) {
  if ( Stds_AssertGrid( grid ) && gridCol < grid->cols && gridRow < grid->rows ) {
    SDL_FRect position = {grid->x + ( float ) ( gridCol * grid->sw ),
                          grid->y + ( float ) ( gridRow * grid->sh ), ( float ) grid->sw,
                          ( float ) grid->sh};
    Stds_BlitTexture( grid->spriteSheet, &grid->clip, position.x, position.y, position.w, position.h, angle, flip, NULL, grid->isCameraOn );
  }
}

/**
 * Adds a animation_t to the vector of animations.
 * @param grid_t* pointer to grid_t.
 * @param animation_t* pointer to animation_t.
 * 
 * @return int32_t.
 */
int32_t
Stds_AddAnimationToGrid( struct grid_t *grid, struct animation_t *animate ) {
  if ( animate == NULL ) {
    Stds_Print( "Error, could not add animation to the grid, it is NULL." );
    return -1;
  } else if ( Stds_AssertGrid( grid ) ) {
    grid->animationBuffer++;
    Stds_VectorAppend( grid->animation, animate );
    return grid->animationBuffer;
  }
  return -1;
}

/**
 * Renders the animation according to the vectors index.
 * @param grid_t* pointer to grid_t.
 * @param uint32_t what column to render the animation.
 * @param uint32_t what row to render the animation.
 * @param int32_t index of the vector storing the animations.
 * @param SDL_RendererFlip will flip texture.
 * @param uint16_t the angle of the image.
 * 
 * @return void.
 */
void
Stds_RenderAnimationToGrid( struct grid_t *grid, uint32_t col, uint32_t row, int32_t index, SDL_RendererFlip flip, uint16_t angle ) {
  if ( Stds_AssertGrid( grid ) && grid->animation != NULL && col < grid->cols && row < grid->rows ) {
    struct animation_t *editAnim = Stds_VectorGet( grid->animation, index );
    editAnim->pos_x              = grid->x + ( float ) ( col * grid->sw );
    editAnim->pos_y              = grid->y + ( float ) ( row * grid->sh );
    editAnim->dest_width = grid->sw;
    editAnim->dest_height = grid->sh;
    editAnim->flip = flip;
    editAnim->angle = angle;
    editAnim->camera = grid->isCameraOn;
    Stds_AnimationDraw( editAnim );
    Stds_AnimationUpdate( editAnim );
  }
}

void 
Stds_AddCollisionToGrid( struct grid_t *grid, uint32_t col, uint32_t row ) {
  if ( Stds_AssertGrid( grid ) && col < grid->cols && row < grid->rows ) {
    
  }
  
}

/**
 * Ensures that the grid is not null.
 *
 * @param grid_t* pointer to grid_t.
 *
 * @return bool.
 */
static bool
Stds_AssertGrid( struct grid_t *grid ) {
  return !( grid == NULL );
}