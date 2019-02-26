/*
 * Copyright (C) 2017 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#include "config.h"
#include <glib/gi18n-lib.h>

#include "hdy-dialer-cycle-button.h"

/**
 * SECTION:hdy-dialer-cycle-button
 * @short_description: A button on a #HdyDialer keypad cycling through available symbols
 * @Title: HdyDialerCycleButton
 *
 * The #HdyDialerCycleButton widget is a single button on a #HdyDialer
 * representing symbols such as digits, letters, #, +
 * or ☃.  When the button is pressed multiple times in a row, the
 * symbols are cycled through. That is a call to #get_curent_symbol
 * returns another symbol each time the button is pressed. If no
 * further button presses are received cycling mode ends after a
 * timeout. This is configurable via the
 * #HdyDialerCycleButton:cycle-timeout property.
 */

typedef struct
{
  int num;             /* number of button presses in the cycle */
  guint32 source_id;   /* timeout handler id */
  gint timeout;        /* timeout between button presses */
} HdyDialerCycleButtonPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (HdyDialerCycleButton, hdy_dialer_cycle_button, HDY_TYPE_DIALER_BUTTON)

enum {
  PROP_0,
  PROP_CYCLE_TIMEOUT,
  PROP_LAST_PROP,
};
static GParamSpec *props[PROP_LAST_PROP];

enum {
  SIGNAL_CYCLE_START,
  SIGNAL_CYCLE_END,
  SIGNAL_LAST_SIGNAL,
};
static guint signals [SIGNAL_LAST_SIGNAL];

static void
end_cycle (HdyDialerCycleButton *self)
{
  HdyDialerCycleButtonPrivate *priv =
    hdy_dialer_cycle_button_get_instance_private(HDY_DIALER_CYCLE_BUTTON (self));

  priv->num = 0;
  priv->source_id = 0;
  g_signal_emit (self, signals[SIGNAL_CYCLE_END], 0);
}

static gboolean
expire_cb (HdyDialerCycleButton *self)
{
  g_return_val_if_fail (HDY_IS_DIALER_CYCLE_BUTTON (self), FALSE);

  end_cycle (self);
  return FALSE;
}

static gboolean
button_clicked_cb (HdyDialerCycleButton *self,
                   GdkEventButton       *event)
{
  HdyDialerCycleButtonPrivate *priv = hdy_dialer_cycle_button_get_instance_private(HDY_DIALER_CYCLE_BUTTON (self));

  g_return_val_if_fail (HDY_IS_DIALER_CYCLE_BUTTON (self), FALSE);

  /* Only cycle if we have more than one symbol */
  if (strlen (hdy_dialer_button_get_symbols (HDY_DIALER_BUTTON (self))) < 2)
    return FALSE;

  if (hdy_dialer_cycle_button_is_cycling (self)) {
    g_source_remove (priv->source_id);
    priv->num++;
  } else {
    g_signal_emit (self, signals[SIGNAL_CYCLE_START], 0);
  }

  priv->source_id = g_timeout_add (priv->timeout,
                                   (GSourceFunc) expire_cb,
                                   self);
  return FALSE;
}

static void
hdy_dialer_cycle_button_set_property (GObject      *object,
                                      guint         property_id,
                                      const GValue *value,
                                      GParamSpec   *pspec)
{
  HdyDialerCycleButton *self = HDY_DIALER_CYCLE_BUTTON (object);
  HdyDialerCycleButtonPrivate *priv = hdy_dialer_cycle_button_get_instance_private (self);

  switch (property_id) {

  case PROP_CYCLE_TIMEOUT:
    priv->timeout = g_value_get_int (value);
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    break;
  }
}

static void
hdy_dialer_cycle_button_get_property (GObject    *object,
                                      guint       property_id,
                                      GValue     *value,
                                      GParamSpec *pspec)
{
  HdyDialerCycleButton *self = HDY_DIALER_CYCLE_BUTTON (object);
  HdyDialerCycleButtonPrivate *priv = hdy_dialer_cycle_button_get_instance_private (self);

  switch (property_id) {
  case PROP_CYCLE_TIMEOUT:
    g_value_set_int (value, priv->timeout);
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    break;
  }
}

static void
hdy_dialer_cycle_button_dispose (GObject *object)
{
  HdyDialerCycleButton *self = HDY_DIALER_CYCLE_BUTTON (object);
  HdyDialerCycleButtonPrivate *priv = hdy_dialer_cycle_button_get_instance_private (self);

  if (priv->source_id) {
    g_source_remove (priv->source_id);
    priv->source_id = 0;
  }

  G_OBJECT_CLASS (hdy_dialer_cycle_button_parent_class)->dispose (object);
}

static void
hdy_dialer_cycle_button_class_init (HdyDialerCycleButtonClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->dispose = hdy_dialer_cycle_button_dispose;

  object_class->set_property = hdy_dialer_cycle_button_set_property;
  object_class->get_property = hdy_dialer_cycle_button_get_property;

  props[PROP_CYCLE_TIMEOUT] =
    g_param_spec_int ("cycle-timeout",
                      _("Cycle timeout"),
                      _("The timeout (in seconds) between button presses after"
                        "which a cycle ends"),
                      0, G_MAXINT, 1000,
                      G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (object_class, PROP_LAST_PROP, props);

  /**
   * HdyDialerCycleButton::cycle-start:
   * @self: The #HdyDialer instance.
   *
   * This signal is emitted when the button starts cycling (that is on
   * the first button press).
   */
  signals[SIGNAL_CYCLE_START] =
    g_signal_new ("cycle-start",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (HdyDialerCycleButtonClass, cycle_start),
                  NULL, NULL, NULL,
                  G_TYPE_NONE,
                  0);

  /**
   * HdyDialerCycleButton::cycle-end:
   * @self: The #HdyDialer instance.
   *
   * This signal is emitted when the cycle ends. This can either be
   * because of timeout or because #hdy_dialer_cycle_stop_cycle got
   * called.
   */
  signals[SIGNAL_CYCLE_END] =
    g_signal_new ("cycle-end",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (HdyDialerCycleButtonClass, cycle_end),
                  NULL, NULL, NULL,
                  G_TYPE_NONE,
                  0);
}

/**
 * hdy_dialer_cycle_button_new:
 * @symbols: the symbols displayed on the #HdyDialerCycleButton
 *
 * Create a new #HdyDialerCycleButton which displays @symbols. The
 * symbols can by cycled through by pressing the button multiple
 * times.
 *
 * Returns: the newly created #HdyDialerCycleButton widget
 */
GtkWidget *hdy_dialer_cycle_button_new (const gchar* symbols)
{
  /* FIXME: we should call this 'symbols' in the base class already */
  return g_object_new (HDY_TYPE_DIALER_CYCLE_BUTTON, "symbols", symbols, NULL);
}

static void
hdy_dialer_cycle_button_init (HdyDialerCycleButton *self)
{
  GtkStyleContext *context;
  GObject *secondary_label;

  g_signal_connect (self, "clicked", G_CALLBACK (button_clicked_cb), NULL);

  end_cycle (self);

  secondary_label = gtk_widget_get_template_child (GTK_WIDGET (self), HDY_TYPE_DIALER_BUTTON, "secondary_label");
  context = gtk_widget_get_style_context (GTK_WIDGET (secondary_label));
  gtk_style_context_remove_class (context, "dim-label");
}

/**
 * hdy_dialer_cycle_button_get_current_symbol:
 * @self: a #HdyDialerCycleButton
 *
 * Get the symbol the dialer should display
 *
 * Returns: a pointer to the symbol
 *
 */
gunichar
hdy_dialer_cycle_button_get_current_symbol (HdyDialerCycleButton *self)
{
  HdyDialerCycleButtonPrivate *priv = hdy_dialer_cycle_button_get_instance_private (self);
  const gchar *symbols = hdy_dialer_button_get_symbols (HDY_DIALER_BUTTON (self));
  gint off = priv->num % g_utf8_strlen (symbols, -1);

  return g_utf8_get_char (g_utf8_offset_to_pointer (symbols, off));
}

/**
 * hdy_dialer_cycle_button_is_cycling:
 * @self: a #HdyDialerCycleButton
 *
 * Check whether the button is in cycling mode.
 *
 * Returns: #TRUE if the in cycling mode otherwise #FALSE
 *
 */
gboolean
hdy_dialer_cycle_button_is_cycling (HdyDialerCycleButton *self)
{
  HdyDialerCycleButtonPrivate *priv = hdy_dialer_cycle_button_get_instance_private (self);

  return !!priv->source_id;
}

/**
 * hdy_dialer_cycle_button_stop_cycle:
 * @self: a #HdyDialerCycleButton
 *
 * Stop the cycling mode.
 *
 */
void
hdy_dialer_cycle_button_stop_cycle (HdyDialerCycleButton *self)
{
  HdyDialerCycleButtonPrivate *priv = hdy_dialer_cycle_button_get_instance_private(self);

  g_return_if_fail (HDY_IS_DIALER_CYCLE_BUTTON (self));

  if (priv->source_id) {
      g_source_remove (priv->source_id);
      priv->source_id = 0;
    }

  end_cycle(self);
}

/**
 * hdy_dialer_cycle_button_get_cycle_timeout:
 * @self: a #HdyDialerCycleButton
 *
 * Get the cycle timeout in milliseconds.
 */
gint
hdy_dialer_cycle_button_get_cycle_timeout (HdyDialerCycleButton *self)
{
  HdyDialerCycleButtonPrivate *priv = hdy_dialer_cycle_button_get_instance_private(self);

  g_return_val_if_fail (HDY_IS_DIALER_CYCLE_BUTTON (self), 0);

  return priv->timeout;
}

/**
 * hdy_dialer_cycle_button_set_cycle_timeout:
 * @self: a #HdyDialerCycleButton
 * @timeout: the timeout in milliseconds
 *
 * Set the cycle timeout in milliseconds.
 */
void
hdy_dialer_cycle_button_set_cycle_timeout (HdyDialerCycleButton *self,
                                           gint                  timeout)
{
  g_return_if_fail (HDY_IS_DIALER_CYCLE_BUTTON (self));

  g_object_set (G_OBJECT (self), "cycle-timeout", timeout, NULL);
}
