#include "adw-demo-adjustment-row.h"

#include <glib/gi18n.h>

struct _AdwDemoAdjustmentRow
{
  AdwBin parent_instance;

  gchar *title;
  guint digits;
  GtkAdjustment *adjustment;
};

G_DEFINE_TYPE (AdwDemoAdjustmentRow, adw_demo_adjustment_row, ADW_TYPE_BIN);

enum {
  PROP_0,
  PROP_TITLE,
  PROP_DIGITS,
  PROP_ADJUSTMENT,
  LAST_PROP
};

static GParamSpec *props[LAST_PROP];

static inline void
set_string (gchar       **dest,
            const gchar  *source)
{
  if (*dest)
    g_free (*dest);

  *dest = g_strdup (source);
}

static void
adw_demo_adjustment_row_dispose (GObject *object)
{
  AdwDemoAdjustmentRow *self = ADW_DEMO_ADJUSTMENT_ROW (object);

  g_clear_object (&self->adjustment);

  G_OBJECT_CLASS (adw_demo_adjustment_row_parent_class)->dispose (object);
}

static void
adw_demo_adjustment_row_finalize (GObject *object)
{
  AdwDemoAdjustmentRow *self = ADW_DEMO_ADJUSTMENT_ROW (object);

  g_clear_pointer (&self->title, g_free);

  G_OBJECT_CLASS (adw_demo_adjustment_row_parent_class)->finalize (object);
}

static void
adw_demo_adjustment_row_get_property (GObject    *object,
                                      guint       prop_id,
                                      GValue     *value,
                                      GParamSpec *pspec)
{
  AdwDemoAdjustmentRow *self = ADW_DEMO_ADJUSTMENT_ROW (object);

  switch (prop_id) {
  case PROP_TITLE:
    g_value_set_string (value, self->title);
    break;
  case PROP_DIGITS:
    g_value_set_uint (value, self->digits);
    break;
  case PROP_ADJUSTMENT:
    g_value_set_object (value, self->adjustment);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_demo_adjustment_row_set_property (GObject      *object,
                                      guint         prop_id,
                                      const GValue *value,
                                      GParamSpec   *pspec)
{
  AdwDemoAdjustmentRow *self = ADW_DEMO_ADJUSTMENT_ROW (object);

  switch (prop_id) {
  case PROP_TITLE:
    set_string (&self->title, g_value_get_string (value));
    break;
  case PROP_DIGITS:
    self->digits = g_value_get_uint (value);
    break;
  case PROP_ADJUSTMENT:
    g_set_object (&self->adjustment, g_value_get_object (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_demo_adjustment_row_class_init (AdwDemoAdjustmentRowClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->dispose = adw_demo_adjustment_row_dispose;
  object_class->finalize = adw_demo_adjustment_row_finalize;
  object_class->get_property = adw_demo_adjustment_row_get_property;
  object_class->set_property = adw_demo_adjustment_row_set_property;

  props[PROP_TITLE] =
    g_param_spec_string ("title",
                         _("Title"),
                         _("Title"),
                         NULL,
                         G_PARAM_READWRITE);

  props[PROP_DIGITS] =
    g_param_spec_uint ("digits",
                       _("Digits"),
                       _("Digits"),
                       0, G_MAXUINT, 0,
                       G_PARAM_READWRITE);

  props[PROP_ADJUSTMENT] =
    g_param_spec_object ("adjustment",
                         _("Adjustment"),
                         _("Adjustment"),
                         GTK_TYPE_ADJUSTMENT,
                         G_PARAM_READWRITE);

  g_object_class_install_properties (object_class, LAST_PROP, props);

  gtk_widget_class_set_template_from_resource (widget_class, "/org/gnome/Adwaita/Demo/pages/spring/adw-demo-adjustment-row.ui");
}

static void
adw_demo_adjustment_row_init (AdwDemoAdjustmentRow *self)
{
  self->digits = 0;

  gtk_widget_init_template (GTK_WIDGET (self));
}
