#ifndef ART_MTG_SIXEL_H
#define ART_MTG_SIXEL_H

#include "art.h"

// Returns 1 if the terminal supports Sixel, 0 otherwise.
int is_sixel_supported();

// Gets the Sixel art module.
ArtModule get_mtg_sixel_module();

#endif // ART_MTG_SIXEL_H
