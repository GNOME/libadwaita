#include "hdy-nothing-private.h"

/**
 * PRIVATE:hdy-nothing
 * @short_description: A helper object for #HdyWindow and #HdyApplicationWindow
 * @title: HdyNothing
 * @See_also: #HdyApplicationWindow, #HdyWindow, #HdyWindowMixin
 * @stability: Private
 *
 * The HdyNothing widget does nothing. It's used as the titlebar for
 * #HdyWindow and #HdyApplicationWindow.
 *
 * Since: 1.0
 */

struct _HdyNothing
{
  GtkWidget parent_instance;
};

G_DEFINE_TYPE (HdyNothing, hdy_nothing, GTK_TYPE_WIDGET)

static void
hdy_nothing_class_init (HdyNothingClass *klass)
{
}

static void
hdy_nothing_init (HdyNothing *self)
{
}

/**
 * hdy_nothing_new:
 *
 * Creates a new #HdyNothing.
 *
 * Returns: (transfer full): a newly created #HdyNothing
 *
 * Since: 1.0
 */
GtkWidget *
hdy_nothing_new (void)
{
  return g_object_new (HDY_TYPE_NOTHING, NULL);
}

