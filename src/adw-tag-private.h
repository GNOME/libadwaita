#include "adw-tag.h"

G_BEGIN_DECLS

typedef enum
{
  ADW_TAG_ICON_NONE,
  ADW_TAG_ICON_GICON,
  ADW_TAG_ICON_PAINTABLE
} AdwTagIconType;

AdwTagIconType  adw_tag_get_icon_type (AdwTag *self);

G_END_DECLS
