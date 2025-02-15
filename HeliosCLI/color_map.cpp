#include <map>
#include <string>

#include "ColorConstants.h"
#include "color_map.h"

// map of colors for the -C, --colorset parsing
std::map<std::string, uint32_t> color_map = {
  {"off", RGB_OFF},
  {"white", RGB_WHITE},
  {"red", RGB_RED},
  {"coralorange", RGB_CORAL_ORANGE},
  {"orange", RGB_ORANGE},
  {"yellow", RGB_YELLOW},
  {"limegreen", RGB_LIME_GREEN},
  {"green", RGB_GREEN},
  {"seafoam", RGB_SEAFOAM},
  {"turquoise", RGB_TURQUOISE},
  {"iceblue", RGB_ICE_BLUE},
  {"lightblue", RGB_LIGHT_BLUE},
  {"blue", RGB_BLUE},
  {"royalblue", RGB_ROYAL_BLUE},
  {"purple", RGB_PURPLE},
  {"pink", RGB_PINK},
  {"hotpink", RGB_HOT_PINK},
  {"magenta", RGB_MAGENTA},
  {"whitebrilow", RGB_WHITE_BRI_LOW},
  {"redbrilow", RGB_RED_BRI_LOW},
  {"coralorangebrilow", RGB_CORAL_ORANGE_BRI_LOW},
  {"orangebrilow", RGB_ORANGE_BRI_LOW},
  {"yellowbrilow", RGB_YELLOW_BRI_LOW},
  {"limegreenbrilow", RGB_LIME_GREEN_BRI_LOW},
  {"greenbrilow", RGB_GREEN_BRI_LOW},
  {"seafoambrilow", RGB_SEAFOAM_BRI_LOW},
  {"turquoisebrilow", RGB_TURQUOISE_BRI_LOW},
  {"icebluebrilow", RGB_ICE_BLUE_BRI_LOW},
  {"lightbluebrilow", RGB_LIGHT_BLUE_BRI_LOW},
  {"bluebrilow", RGB_BLUE_BRI_LOW},
  {"royalbluebrilow", RGB_ROYAL_BLUE_BRI_LOW},
  {"purplebrilow", RGB_PURPLE_BRI_LOW},
  {"pinkbrilow", RGB_PINK_BRI_LOW},
  {"hotpinkbrilow", RGB_HOT_PINK_BRI_LOW},
  {"magentabrilow", RGB_MAGENTA_BRI_LOW},
  {"whitebrilowest", RGB_WHITE_BRI_LOWEST}
};