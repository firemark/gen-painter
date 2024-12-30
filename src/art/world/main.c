#include "art/world.h"
#include "art/object/tree.h"

bool world_init(void) {
  return tree_init();
}

void world_reset(void) {
  tree_reset();
}