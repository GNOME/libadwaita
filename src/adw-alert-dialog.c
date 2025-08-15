/*
 * Copyright (C) 2022 Purism SPC
 * Copyright (C) 2024 GNOME Foundation Inc.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Author: Alice Mikhaylenko <alice.mikhaylenko@puri.sm>
 */

#include "config.h"

#include "adw-alert-dialog.h"

#include "adw-bin.h"
#include "adw-dialog-private.h"
#include "adw-gizmo-private.h"
#include "adw-gtkbuilder-utils-private.h"
#include "adw-length-unit.h"
#include "adw-marshalers.h"
#include "adw-widget-utils-private.h"

/**
 * AdwAlertDialog:
 *
 * A dialog presenting a message or a question.
 *
 * <picture>
 *   <source srcset="alert-dialog-dark.png" media="(prefers-color-scheme: dark)">
 *   <img src="alert-dialog.png" alt="alert-dialog">
 * </picture>
 *
 * Alert dialogs have a heading, a body, an optional child widget, and one or
 * multiple responses, each presented as a button.
 *
 * Each response has a unique string ID, and a button label. Additionally, each
 * response can be enabled or disabled, and can have a suggested or destructive
 * appearance.
 *
 * When one of the responses is activated, or the dialog is closed, the
 * [signal@AlertDialog::response] signal will be emitted. This signal is
 * detailed, and the detail, as well as the `response` parameter will be set to
 * the ID of the activated response, or to the value of the
 * [property@AlertDialog:close-response] property if the dialog had been closed
 * without activating any of the responses.
 *
 * Response buttons can be presented horizontally or vertically depending on
 * available space.
 *
 * When a response is activated, `AdwAlertDialog` is closed automatically.
 *
 * An example of using an alert dialog:
 *
 * ```c
 * AdwDialog *dialog;
 *
 * dialog = adw_alert_dialog_new (_("Replace File?"), NULL);
 *
 * adw_alert_dialog_format_body (ADW_ALERT_DIALOG (dialog),
 *                               _("A file named “%s” already exists. Do you want to replace it?"),
 *                               filename);
 *
 * adw_alert_dialog_add_responses (ADW_ALERT_DIALOG (dialog),
 *                                 "cancel",  _("_Cancel"),
 *                                 "replace", _("_Replace"),
 *                                 NULL);
 *
 * adw_alert_dialog_set_response_appearance (ADW_ALERT_DIALOG (dialog),
 *                                           "replace",
 *                                           ADW_RESPONSE_DESTRUCTIVE);
 *
 * adw_alert_dialog_set_default_response (ADW_ALERT_DIALOG (dialog), "cancel");
 * adw_alert_dialog_set_close_response (ADW_ALERT_DIALOG (dialog), "cancel");
 *
 * g_signal_connect (dialog, "response", G_CALLBACK (response_cb), self);
 *
 * adw_dialog_present (dialog, parent);
 * ```
 *
 * ## Async API
 *
 * `AdwAlertDialog` can also be used via the [method@AlertDialog.choose] method.
 * This API follows the GIO async pattern, for example:
 *
 * ```c
 * static void
 * dialog_cb (AdwAlertDialog *dialog,
 *            GAsyncResult   *result,
 *            MyWindow       *self)
 * {
 *   const char *response = adw_alert_dialog_choose_finish (dialog, result);
 *
 *   // ...
 * }
 *
 * static void
 * show_dialog (MyWindow *self)
 * {
 *   AdwDialog *dialog;
 *
 *   dialog = adw_alert_dialog_new (_("Replace File?"), NULL);
 *
 *   adw_alert_dialog_format_body (ADW_ALERT_DIALOG (dialog),
 *                                 _("A file named “%s” already exists. Do you want to replace it?"),
 *                                 filename);
 *
 *   adw_alert_dialog_add_responses (ADW_ALERT_DIALOG (dialog),
 *                                   "cancel",  _("_Cancel"),
 *                                   "replace", _("_Replace"),
 *                                   NULL);
 *
 *   adw_alert_dialog_set_response_appearance (ADW_ALERT_DIALOG (dialog),
 *                                             "replace",
 *                                             ADW_RESPONSE_DESTRUCTIVE);
 *
 *   adw_alert_dialog_set_default_response (ADW_ALERT_DIALOG (dialog), "cancel");
 *   adw_alert_dialog_set_close_response (ADW_ALERT_DIALOG (dialog), "cancel");
 *
 *   adw_alert_dialog_choose (ADW_ALERT_DIALOG (dialog), GTK_WIDGET (self),
 *                            NULL, (GAsyncReadyCallback) dialog_cb, self);
 * }
 * ```
 *
 * ## AdwAlertDialog as GtkBuildable
 *
 * `AdwAlertDialog` supports adding responses in UI definitions by via the
 * `<responses>` element that may contain multiple `<response>` elements, each
 * representing a response.
 *
 * Each of the `<response>` elements must have the `id` attribute specifying the
 * response ID. The contents of the element are used as the response label.
 *
 * Response labels can be translated with the usual `translatable`, `context`
 * and `comments` attributes.
 *
 * The `<response>` elements can also have `enabled` and/or `appearance`
 * attributes. See [method@AlertDialog.set_response_enabled] and
 * [method@AlertDialog.set_response_appearance] for details.
 *
 * Example of an `AdwAlertDialog` UI definition:
 *
 * ```xml
 * <object class="AdwAlertDialog" id="dialog">
 *   <property name="heading" translatable="yes">Save Changes?</property>
 *   <property name="body" translatable="yes">Open documents contain unsaved changes. Changes which are not saved will be permanently lost.</property>
 *   <property name="default-response">save</property>
 *   <property name="close-response">cancel</property>
 *   <signal name="response" handler="response_cb"/>
 *   <responses>
 *     <response id="cancel" translatable="yes">_Cancel</response>
 *     <response id="discard" translatable="yes" appearance="destructive">_Discard</response>
 *     <response id="save" translatable="yes" appearance="suggested" enabled="false">_Save</response>
 *   </responses>
 * </object>
 * ```
 *
 * Since: 1.5
 */

/**
 * AdwResponseAppearance:
 * @ADW_RESPONSE_DEFAULT: the default appearance.
 * @ADW_RESPONSE_SUGGESTED: used to denote important responses such as the
 *     affirmative action.
 * @ADW_RESPONSE_DESTRUCTIVE: used to draw attention to the potentially damaging
 *     consequences of using the response. This appearance acts as a warning to
 *     the user.
 *
 * Describes the possible styles of [class@AlertDialog] response buttons.
 *
 * See [method@AlertDialog.set_response_appearance].
 *
 * Since: 1.2
 */

#define DIALOG_MAX_WIDTH 372 /* sp, not px */
#define DIALOG_MAX_WIDE_WIDTH 600 /* sp, not px */
#define DIALOG_PREFERRED_WIDTH 300 /* sp, not px */
#define DIALOG_MIN_WIDTH 300
#define BUTTON_SPACING 12
#define RESPONSE_HORZ_PADDING 48
#define RESPONSE_HORZ_PADDING_SHORT 36

typedef struct {
  AdwAlertDialog *dialog;
  GQuark id;
  char *label;
  AdwResponseAppearance appearance;
  gboolean enabled;

  GtkWidget *button;
} ResponseInfo;

typedef struct
{
  GtkWidget *contents;
  GtkWidget *window_handle;
  GtkWidget *scrolled_window;
  GtkWidget *heading_bin;
  GtkWidget *heading_label;
  GtkWidget *heading_label_small;
  GtkWidget *body_label;
  GtkWidget *child_bin;
  GtkBox *message_area;
  GtkWidget *response_area;

  char *heading;
  gboolean heading_use_markup;
  char *body;
  gboolean body_use_markup;
  GtkWidget *child;
  gboolean prefer_wide_layout;

  GList *responses;
  GHashTable *id_to_response;
  GQuark default_response;
  GQuark close_response;

  gboolean block_close_response;
  gboolean is_short;
} AdwAlertDialogPrivate;

static void adw_alert_dialog_buildable_init (GtkBuildableIface *iface);

G_DEFINE_TYPE_WITH_CODE (AdwAlertDialog, adw_alert_dialog, ADW_TYPE_DIALOG,
                         G_ADD_PRIVATE (AdwAlertDialog)
                         G_IMPLEMENT_INTERFACE (GTK_TYPE_BUILDABLE, adw_alert_dialog_buildable_init))

static GtkBuildableIface *parent_buildable_iface;

enum {
  PROP_0,
  PROP_HEADING,
  PROP_HEADING_USE_MARKUP,
  PROP_BODY,
  PROP_BODY_USE_MARKUP,
  PROP_EXTRA_CHILD,
  PROP_PREFER_WIDE_LAYOUT,
  PROP_DEFAULT_RESPONSE,
  PROP_CLOSE_RESPONSE,
  LAST_PROP,
};

static GParamSpec *props[LAST_PROP];

enum {
  SIGNAL_RESPONSE,
  SIGNAL_LAST_SIGNAL,
};
static guint signals[SIGNAL_LAST_SIGNAL];

static void
response_info_free (ResponseInfo *info)
{
  g_free (info->label);
  g_free (info);
}

static inline ResponseInfo *
find_response (AdwAlertDialog *self,
               const char     *id)
{
  AdwAlertDialogPrivate *priv = adw_alert_dialog_get_instance_private (self);

  return g_hash_table_lookup (priv->id_to_response, id);
}


static inline void
emit_response (AdwAlertDialog *self, GQuark response)
{
  AdwAlertDialogPrivate *priv = adw_alert_dialog_get_instance_private (self);

  g_object_ref (self);
  priv->block_close_response = TRUE;

  adw_dialog_close (ADW_DIALOG (self));
  g_signal_emit (self, signals[SIGNAL_RESPONSE], response, g_quark_to_string (response));

  priv->block_close_response = FALSE;
  g_object_unref (self);
}


static void
button_clicked_cb (ResponseInfo *info)
{
  emit_response (info->dialog, info->id);
}

static GtkWidget *
create_response_button (AdwAlertDialog *self,
                        ResponseInfo   *info)
{
  GtkWidget *button = gtk_button_new_with_mnemonic (info->label);

  gtk_button_set_can_shrink (GTK_BUTTON (button), TRUE);

  switch (info->appearance) {
  case ADW_RESPONSE_SUGGESTED:
    gtk_widget_add_css_class (button, "suggested-action");
    break;
  case ADW_RESPONSE_DESTRUCTIVE:
    gtk_widget_add_css_class (button, "destructive-action");
    break;
  case ADW_RESPONSE_DEFAULT:
  default:
    break;
  }

  gtk_widget_set_sensitive (button, info->enabled);

  g_signal_connect_swapped (button, "clicked", G_CALLBACK (button_clicked_cb), info);

  return button;
}

static void
update_window_title (AdwAlertDialog *self)
{
  AdwAlertDialogPrivate *priv = adw_alert_dialog_get_instance_private (self);

  if (priv->heading_use_markup) {
    char *heading = NULL;
    GError *error = NULL;

    pango_parse_markup (priv->heading, -1, 0, NULL, &heading, NULL, &error);

    if (error) {
      g_critical ("Couldn't parse markup: %s", error->message);
      g_clear_error (&error);

      heading = g_strdup (priv->heading);
    }

    adw_dialog_set_title (ADW_DIALOG (self), heading);

    g_free (heading);
  } else {
    adw_dialog_set_title (ADW_DIALOG (self), priv->heading);
  }
}

static void
adw_alert_dialog_closed (AdwDialog *dialog)
{
  AdwAlertDialog *self = ADW_ALERT_DIALOG (dialog);
  AdwAlertDialogPrivate *priv = adw_alert_dialog_get_instance_private (self);

  if (priv->block_close_response)
    return;

  g_signal_emit (self, signals[SIGNAL_RESPONSE],
                 priv->close_response,
                 g_quark_to_string (priv->close_response));
}

static void
adw_alert_dialog_map (GtkWidget *widget)
{
  AdwAlertDialog *self = ADW_ALERT_DIALOG (widget);
  GtkWidget *window;

  GTK_WIDGET_CLASS (adw_alert_dialog_parent_class)->map (widget);

  window = adw_dialog_get_window (ADW_DIALOG (self));

  if (window) {
    gtk_widget_add_css_class (window, "alert");
    gtk_widget_grab_focus (widget);
  }
}

static gboolean
adw_alert_dialog_grab_focus (GtkWidget *widget)
{
  AdwAlertDialog *self = ADW_ALERT_DIALOG (widget);
  AdwAlertDialogPrivate *priv = adw_alert_dialog_get_instance_private (self);
  GtkWidget *focus;
  gboolean ret;

  focus = adw_dialog_get_focus (ADW_DIALOG (self));

  ret = GTK_WIDGET_CLASS (adw_alert_dialog_parent_class)->grab_focus (widget);

  if (!focus) {
    GtkWidget *default_widget;
    GList *l;

    if (adw_widget_grab_focus_child (priv->scrolled_window)) {
      focus = adw_dialog_get_focus (ADW_DIALOG (self));

      if (GTK_IS_LABEL (focus) && !gtk_label_get_current_uri (GTK_LABEL (focus)))
        gtk_label_select_region (GTK_LABEL (focus), 0, 0);

      return TRUE;
    }

    default_widget = adw_dialog_get_default_widget (ADW_DIALOG (self));
    if (default_widget)
      return gtk_widget_grab_focus (default_widget);

    for (l = g_list_last (priv->responses); l; l = l->prev) {
      ResponseInfo *response = l->data;

      if (!response->enabled)
        continue;

      return gtk_widget_grab_focus (response->button);
    }
  }

  return ret;
}

static GtkSizeRequestMode
get_heading_request_mode (GtkWidget *widget)
{
  return GTK_SIZE_REQUEST_HEIGHT_FOR_WIDTH;
}

static void
measure_heading (GtkWidget      *widget,
                 GtkOrientation  orientation,
                 int             for_size,
                 int            *minimum,
                 int            *natural,
                 int            *minimum_baseline,
                 int            *natural_baseline)
{
  AdwAlertDialog *self = g_object_get_data (G_OBJECT (widget), "-adw-alert-dialog");
  AdwAlertDialogPrivate *priv = adw_alert_dialog_get_instance_private (self);
  int large_min, large_nat, small_min, small_nat, for_size_large_label;

  if (priv->is_short) {
    gtk_widget_measure (priv->heading_label_small, orientation, for_size,
                        minimum, natural, NULL, NULL);

    if (minimum_baseline)
      *minimum_baseline = -1;
    if (natural_baseline)
      *natural_baseline = -1;

    return;
  }

  if (orientation == GTK_ORIENTATION_HORIZONTAL) {
    gtk_widget_measure (priv->heading_label, GTK_ORIENTATION_HORIZONTAL, -1,
                        &for_size_large_label, NULL, NULL, NULL);
  } else {
    for_size_large_label = -1;
  }

  gtk_widget_measure (priv->heading_label, orientation, for_size_large_label,
                      &large_min, &large_nat, NULL, NULL);
  gtk_widget_measure (priv->heading_label_small, orientation, for_size,
                      &small_min, &small_nat, NULL, NULL);

  if (orientation == GTK_ORIENTATION_HORIZONTAL) {
    if (minimum)
      *minimum = MIN (large_min, small_min);
    if (natural)
      *natural = MAX (large_nat, small_nat);
  } else {
    if (for_size < 0) {
      if (minimum)
        *minimum = MAX (large_min, small_min);
      if (natural)
        *natural = MAX (large_nat, small_nat);
    } else {
      int large_width;

      gtk_widget_measure (priv->heading_label, GTK_ORIENTATION_HORIZONTAL, -1,
                          NULL, &large_width, NULL, NULL);

      if (minimum)
        *minimum = (large_width > for_size) ? small_min : large_min;
      if (natural)
        *natural = (large_width > for_size) ? small_nat : large_nat;
    }
  }

  if (minimum_baseline)
    *minimum_baseline = -1;
  if (natural_baseline)
    *natural_baseline = -1;
}

static void
allocate_heading (GtkWidget *widget,
                  int        width,
                  int        height,
                  int        baseline)
{
  AdwAlertDialog *self = g_object_get_data (G_OBJECT (widget), "-adw-alert-dialog");
  AdwAlertDialogPrivate *priv = adw_alert_dialog_get_instance_private (self);
  gboolean small_label;

  if (priv->is_short) {
    small_label = TRUE;
  } else {
    int large_nat;

    gtk_widget_measure (priv->heading_label, GTK_ORIENTATION_HORIZONTAL, -1,
                        NULL, &large_nat, NULL, NULL);

    small_label = large_nat > width;
  }

  if (gtk_widget_get_child_visible (priv->heading_label) == small_label)
    gtk_widget_set_child_visible (priv->heading_label, !small_label);

  if (gtk_widget_get_child_visible (priv->heading_label_small) != small_label)
    gtk_widget_set_child_visible (priv->heading_label_small, small_label);

  if (small_label)
    gtk_widget_allocate (priv->heading_label_small, width, height, baseline, NULL);
  else
    gtk_widget_allocate (priv->heading_label, width, height, baseline, NULL);
}

static void
measure_responses_do (AdwAlertDialog *self,
                      gboolean        compact,
                      GtkOrientation  orientation,
                      int            *minimum,
                      int            *natural)
{
  AdwAlertDialogPrivate *priv = adw_alert_dialog_get_instance_private (self);
  GList *l;
  int min = 0, nat = 0;
  int button_min = 0, button_nat = 0;
  int n_buttons = 0;
  gboolean horiz = (orientation == GTK_ORIENTATION_HORIZONTAL);

  for (l = priv->responses; l; l = l->next) {
    ResponseInfo *response = l->data;
    int child_min, child_nat;

    gtk_widget_measure (response->button, orientation, -1,
                        &child_min, &child_nat, NULL, NULL);

    if (horiz == compact) {
      min = MAX (min, child_min);
      nat = MAX (nat, child_nat);
    } else if (horiz) {
      button_min = MAX (button_min, child_min);
      button_nat = MAX (button_nat, child_nat);
      n_buttons++;
    } else {
      min += child_min;
      nat += child_nat;
    }

    if (horiz != compact && l->next) {
      min += BUTTON_SPACING;
      nat += BUTTON_SPACING;
    }
  }

  if (horiz && !compact) {
    min += button_min * n_buttons;
    nat += button_nat * n_buttons;
  }

  if (minimum)
    *minimum = min;
  if (natural)
    *natural = nat;
}

static GtkSizeRequestMode
get_responses_request_mode (GtkWidget *widget)
{
  return GTK_SIZE_REQUEST_HEIGHT_FOR_WIDTH;
}

static void
measure_responses (GtkWidget      *widget,
                   GtkOrientation  orientation,
                   int             for_size,
                   int            *minimum,
                   int            *natural,
                   int            *minimum_baseline,
                   int            *natural_baseline)
{
  AdwAlertDialog *self = g_object_get_data (G_OBJECT (widget), "-adw-alert-dialog");
  AdwAlertDialogPrivate *priv = adw_alert_dialog_get_instance_private (self);

  if (orientation == GTK_ORIENTATION_HORIZONTAL) {
    measure_responses_do (self, TRUE, orientation, minimum, NULL);
    measure_responses_do (self, FALSE, orientation, NULL, natural);
  } else {
    int wide_nat = 0;
    gboolean use_compact_layout;

    if (for_size >= 0)
      measure_responses_do (self, FALSE, GTK_ORIENTATION_HORIZONTAL, NULL, &wide_nat);

    use_compact_layout = for_size >= 0 && wide_nat > for_size && !priv->is_short;
    measure_responses_do (self, use_compact_layout, orientation, minimum, natural);
  }

  if (minimum_baseline)
    *minimum_baseline = -1;
  if (natural_baseline)
    *natural_baseline = -1;
}

static void
allocate_responses (GtkWidget *widget,
                    int        width,
                    int        height,
                    int        baseline)
{
  AdwAlertDialog *self = ADW_ALERT_DIALOG (gtk_widget_get_ancestor (widget, ADW_TYPE_ALERT_DIALOG));
  AdwAlertDialogPrivate *priv = adw_alert_dialog_get_instance_private (self);
  gboolean compact;
  int wide_nat;

  measure_responses_do (self, FALSE, GTK_ORIENTATION_HORIZONTAL, NULL, &wide_nat);

  compact = wide_nat > width && !priv->is_short;

  if (compact)
    gtk_widget_add_css_class (widget, "compact");
  else
    gtk_widget_remove_css_class (widget, "compact");

  if (compact) {
    int pos = height;
    GList *l;

    for (l = priv->responses; l; l = l->next) {
      ResponseInfo *response = l->data;
      int child_height;

      gtk_widget_measure (response->button, GTK_ORIENTATION_VERTICAL, -1,
                          &child_height, NULL, NULL, NULL);

      pos -= child_height;

      gtk_widget_allocate (response->button, width, child_height, -1,
                           gsk_transform_translate (NULL, &GRAPHENE_POINT_INIT (0, pos)));

      pos -= BUTTON_SPACING;
    }
  } else {
    gboolean is_rtl = gtk_widget_get_direction (widget) == GTK_TEXT_DIR_RTL;
    int pos = is_rtl ? width : 0;
    int n_buttons = g_list_length (priv->responses);
    int total_width = width - BUTTON_SPACING * MAX (0, (n_buttons - 1));
    int button_width = (int) ceil ((double) total_width / n_buttons);
    GList *l;

    for (l = priv->responses; l; l = l->next) {
      ResponseInfo *response = l->data;

      button_width = MIN (button_width, total_width);

      total_width -= button_width;

      if (is_rtl)
        pos -= button_width;

      gtk_widget_allocate (response->button, button_width, height, -1,
                           gsk_transform_translate (NULL, &GRAPHENE_POINT_INIT (pos, 0)));

      if (!is_rtl)
        pos += button_width + BUTTON_SPACING;
      else
        pos -= BUTTON_SPACING;
    }
  }
}

static void
measure_child (GtkWidget      *widget,
               GtkOrientation  orientation,
               int             for_size,
               int            *min,
               int            *nat,
               int            *min_baseline,
               int            *nat_baseline)
{
  AdwAlertDialog *self = g_object_get_data (G_OBJECT (widget), "-adw-alert-dialog");
  AdwAlertDialogPrivate *priv = adw_alert_dialog_get_instance_private (self);
  int max_size, min_size, base_nat;

  gtk_widget_measure (priv->window_handle, orientation, for_size,
                      &min_size, &base_nat, NULL, NULL);

  if (orientation == GTK_ORIENTATION_HORIZONTAL) {
    int pref_width, wide_nat, narrow_nat, heading_nat;
    int regular_max_size;

    min_size = MAX (min_size, DIALOG_MIN_WIDTH);

    pref_width = adw_length_unit_to_px (ADW_LENGTH_UNIT_SP,
                                        DIALOG_PREFERRED_WIDTH,
                                        gtk_widget_get_settings (widget));

    if (gtk_widget_get_visible (priv->heading_bin)) {
      gtk_widget_measure (priv->heading_bin, orientation, -1,
                          NULL, &heading_nat, NULL, NULL);
    } else {
      heading_nat = 0;
    }

    measure_responses_do (self, FALSE, orientation, NULL, &wide_nat);
    measure_responses_do (self, TRUE, orientation, NULL, &narrow_nat);

    if (priv->is_short) {
      wide_nat += RESPONSE_HORZ_PADDING_SHORT;
      narrow_nat += RESPONSE_HORZ_PADDING_SHORT;
    } else {
      wide_nat += RESPONSE_HORZ_PADDING;
      narrow_nat += RESPONSE_HORZ_PADDING;
    }

    narrow_nat = MAX (narrow_nat, pref_width);
    heading_nat = CLAMP (heading_nat, narrow_nat, wide_nat);

    regular_max_size = adw_length_unit_to_px (ADW_LENGTH_UNIT_SP,
                                              DIALOG_MAX_WIDTH,
                                              gtk_widget_get_settings (widget));

    if (priv->prefer_wide_layout || priv->is_short) {
      max_size = adw_length_unit_to_px (ADW_LENGTH_UNIT_SP,
                                        DIALOG_MAX_WIDE_WIDTH,
                                        gtk_widget_get_settings (widget));

      max_size = MIN (max_size, wide_nat);
    } else if (wide_nat > min_size) {
      max_size = MIN (regular_max_size, heading_nat);
    } else {
      max_size = regular_max_size;
    }
  } else {
    max_size = G_MAXINT;
  }

  max_size = MAX (min_size, max_size);

  if (min)
    *min = min_size;
  if (nat)
    *nat = CLAMP (base_nat, min_size, max_size);
  if (min_baseline)
    *min_baseline = -1;
  if (nat_baseline)
    *nat_baseline = -1;
}

static void
allocate_child (GtkWidget *widget,
                int        width,
                int        height,
                int        baseline)
{
  AdwAlertDialog *self = g_object_get_data (G_OBJECT (widget), "-adw-alert-dialog");
  AdwAlertDialogPrivate *priv = adw_alert_dialog_get_instance_private (self);

  gtk_widget_allocate (priv->window_handle, width, height, baseline, NULL);
}

static void
css_classes_changed_cb (AdwAlertDialog *self)
{
  AdwAlertDialogPrivate *priv = adw_alert_dialog_get_instance_private (self);
  gboolean is_short = gtk_widget_has_css_class (GTK_WIDGET (self), "short");

  if (is_short == priv->is_short)
    return;

  priv->is_short = is_short;

  gtk_widget_queue_resize (priv->contents);
}

static void
adw_alert_dialog_get_property (GObject    *object,
                               guint       prop_id,
                               GValue     *value,
                               GParamSpec *pspec)
{
  AdwAlertDialog *self = ADW_ALERT_DIALOG (object);

  switch (prop_id) {
  case PROP_HEADING:
    g_value_set_string (value, adw_alert_dialog_get_heading (self));
    break;
  case PROP_HEADING_USE_MARKUP:
    g_value_set_boolean (value, adw_alert_dialog_get_heading_use_markup (self));
    break;
  case PROP_BODY:
    g_value_set_string (value, adw_alert_dialog_get_body (self));
    break;
  case PROP_BODY_USE_MARKUP:
    g_value_set_boolean (value, adw_alert_dialog_get_body_use_markup (self));
    break;
  case PROP_EXTRA_CHILD:
    g_value_set_object (value, adw_alert_dialog_get_extra_child (self));
    break;
  case PROP_PREFER_WIDE_LAYOUT:
    g_value_set_boolean (value, adw_alert_dialog_get_prefer_wide_layout (self));
    break;
  case PROP_DEFAULT_RESPONSE:
    g_value_set_string (value, adw_alert_dialog_get_default_response (self));
    break;
  case PROP_CLOSE_RESPONSE:
    g_value_set_string (value, adw_alert_dialog_get_close_response (self));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_alert_dialog_set_property (GObject      *object,
                               guint         prop_id,
                               const GValue *value,
                               GParamSpec   *pspec)
{
  AdwAlertDialog *self = ADW_ALERT_DIALOG (object);

  switch (prop_id) {
  case PROP_HEADING:
    adw_alert_dialog_set_heading (self, g_value_get_string (value));
    break;
  case PROP_HEADING_USE_MARKUP:
    adw_alert_dialog_set_heading_use_markup (self, g_value_get_boolean (value));
    break;
  case PROP_BODY:
    adw_alert_dialog_set_body (self, g_value_get_string (value));
    break;
  case PROP_BODY_USE_MARKUP:
    adw_alert_dialog_set_body_use_markup (self, g_value_get_boolean (value));
    break;
  case PROP_EXTRA_CHILD:
    adw_alert_dialog_set_extra_child (self, g_value_get_object (value));
    break;
  case PROP_PREFER_WIDE_LAYOUT:
    adw_alert_dialog_set_prefer_wide_layout (self, g_value_get_boolean (value));
    break;
  case PROP_DEFAULT_RESPONSE:
    adw_alert_dialog_set_default_response (self, g_value_get_string (value));
    break;
  case PROP_CLOSE_RESPONSE:
    adw_alert_dialog_set_close_response (self, g_value_get_string (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_alert_dialog_dispose (GObject *object)
{
  AdwAlertDialog *self = ADW_ALERT_DIALOG (object);
  AdwAlertDialogPrivate *priv = adw_alert_dialog_get_instance_private (self);

  priv->child = NULL;

  if (priv->responses) {
    g_list_free_full (priv->responses, (GDestroyNotify) response_info_free);
    priv->responses = NULL;
  }

  g_clear_pointer (&priv->id_to_response, g_hash_table_unref);

  G_OBJECT_CLASS (adw_alert_dialog_parent_class)->dispose (object);
}

static void
adw_alert_dialog_finalize (GObject *object)
{
  AdwAlertDialog *self = ADW_ALERT_DIALOG (object);
  AdwAlertDialogPrivate *priv = adw_alert_dialog_get_instance_private (self);

  g_clear_pointer (&priv->heading, g_free);
  g_clear_pointer (&priv->body, g_free);

  G_OBJECT_CLASS (adw_alert_dialog_parent_class)->finalize (object);
}

static void
adw_alert_dialog_class_init (AdwAlertDialogClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
  AdwDialogClass *dialog_class = ADW_DIALOG_CLASS (klass);

  object_class->get_property = adw_alert_dialog_get_property;
  object_class->set_property = adw_alert_dialog_set_property;
  object_class->dispose = adw_alert_dialog_dispose;
  object_class->finalize = adw_alert_dialog_finalize;

  widget_class->map = adw_alert_dialog_map;
  widget_class->grab_focus = adw_alert_dialog_grab_focus;

  dialog_class->closed = adw_alert_dialog_closed;

  /**
   * AdwAlertDialog:heading:
   *
   * The heading of the dialog.
   *
   * Since: 1.5
   */
  props[PROP_HEADING] =
    g_param_spec_string ("heading", NULL, NULL,
                         "",
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwAlertDialog:heading-use-markup:
   *
   * Whether the heading includes Pango markup.
   *
   * See [func@Pango.parse_markup].
   *
   * Since: 1.5
   */
  props[PROP_HEADING_USE_MARKUP] =
    g_param_spec_boolean ("heading-use-markup", NULL, NULL,
                          FALSE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwAlertDialog:body:
   *
   * The body text of the dialog.
   *
   * Since: 1.5
   */
  props[PROP_BODY] =
    g_param_spec_string ("body", NULL, NULL,
                         "",
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwAlertDialog:body-use-markup:
   *
   * Whether the body text includes Pango markup.
   *
   * See [func@Pango.parse_markup].
   *
   * Since: 1.5
   */
  props[PROP_BODY_USE_MARKUP] =
    g_param_spec_boolean ("body-use-markup", NULL, NULL,
                          FALSE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwAlertDialog:extra-child:
   *
   * The child widget.
   *
   * Displayed below the heading and body.
   *
   * Since: 1.5
   */
  props[PROP_EXTRA_CHILD] =
    g_param_spec_object ("extra-child", NULL, NULL,
                         GTK_TYPE_WIDGET,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwAlertDialog:prefer-wide-layout:
   *
   * Whether to prefer wide layout.
   *
   * Prefer horizontal button layout when possible, and wider dialog width
   * otherwise.
   *
   * Since: 1.6
   */
  props[PROP_PREFER_WIDE_LAYOUT] =
    g_param_spec_boolean ("prefer-wide-layout", NULL, NULL,
                          FALSE,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwAlertDialog:default-response:
   *
   * The response ID of the default response.
   *
   * The button corresponding to this response will be set as the default widget
   * of the dialog.
   *
   * If not set, the default widget will not be set, and the last added response
   * will be focused by default.
   *
   * See [property@Dialog:default-widget].
   *
   * Since: 1.5
   */
  props[PROP_DEFAULT_RESPONSE] =
    g_param_spec_string ("default-response", NULL, NULL,
                          NULL,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwAlertDialog:close-response:
   *
   * The ID of the close response.
   *
   * It will be passed to [signal@AlertDialog::response] if the dialog is
   * closed by pressing <kbd>Escape</kbd> or with a system action.
   *
   * It doesn't have to correspond to any of the responses in the dialog.
   *
   * The default close response is `close`.
   *
   * Since: 1.5
   */
  props[PROP_CLOSE_RESPONSE] =
    g_param_spec_string ("close-response", NULL, NULL,
                          "close",
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  g_object_class_install_properties (object_class, LAST_PROP, props);

  /**
   * AdwAlertDialog::response:
   * @self: an alert dialog
   * @response: the response ID
   *
   * This signal is emitted when the dialog is closed.
   *
   * @response will be set to the response ID of the button that had been
   * activated.
   *
   * if the dialog was closed by pressing <kbd>Escape</kbd> or with a system
   * action, @response will be set to the value of
   * [property@AlertDialog:close-response].
   *
   * Since: 1.5
   */
  signals[SIGNAL_RESPONSE] =
    g_signal_new ("response",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_DETAILED,
                  G_STRUCT_OFFSET (AdwAlertDialogClass, response),
                  NULL, NULL,
                  adw_marshal_VOID__STRING,
                  G_TYPE_NONE,
                  1,
                  G_TYPE_STRING);
  g_signal_set_va_marshaller (signals[SIGNAL_RESPONSE],
                              G_TYPE_FROM_CLASS (klass),
                              adw_marshal_VOID__STRINGv);

  gtk_widget_class_set_template_from_resource (widget_class,
                                               "/org/gnome/Adwaita/ui/adw-alert-dialog.ui");

  gtk_widget_class_bind_template_child_private (widget_class, AdwAlertDialog, contents);
  gtk_widget_class_bind_template_child_private (widget_class, AdwAlertDialog, window_handle);
  gtk_widget_class_bind_template_child_private (widget_class, AdwAlertDialog, scrolled_window);
  gtk_widget_class_bind_template_child_private (widget_class, AdwAlertDialog, heading_bin);
  gtk_widget_class_bind_template_child_private (widget_class, AdwAlertDialog, heading_label);
  gtk_widget_class_bind_template_child_private (widget_class, AdwAlertDialog, heading_label_small);
  gtk_widget_class_bind_template_child_private (widget_class, AdwAlertDialog, body_label);
  gtk_widget_class_bind_template_child_private (widget_class, AdwAlertDialog, child_bin);
  gtk_widget_class_bind_template_child_private (widget_class, AdwAlertDialog, message_area);
  gtk_widget_class_bind_template_child_private (widget_class, AdwAlertDialog, response_area);

  gtk_widget_class_set_accessible_role (widget_class, GTK_ACCESSIBLE_ROLE_ALERT_DIALOG);

  g_type_ensure (ADW_TYPE_GIZMO);
}

static void
adw_alert_dialog_init (AdwAlertDialog *self)
{
  AdwAlertDialogPrivate *priv = adw_alert_dialog_get_instance_private (self);

  priv->close_response = g_quark_from_string ("close");

  priv->heading = g_strdup ("");
  priv->body = g_strdup ("");
  priv->id_to_response = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);

  gtk_widget_init_template (GTK_WIDGET (self));

  g_object_set_data (G_OBJECT (priv->contents), "-adw-alert-dialog", self);
  g_object_set_data (G_OBJECT (priv->heading_bin), "-adw-alert-dialog", self);
  g_object_set_data (G_OBJECT (priv->response_area), "-adw-alert-dialog", self);

  gtk_widget_set_layout_manager (priv->contents,
                                 gtk_custom_layout_new (adw_widget_get_request_mode,
                                                        measure_child,
                                                        allocate_child));

  gtk_widget_set_layout_manager (priv->heading_bin,
                                 gtk_custom_layout_new (get_heading_request_mode,
                                                        measure_heading,
                                                        allocate_heading));

  gtk_widget_set_layout_manager (priv->response_area,
                                 gtk_custom_layout_new (get_responses_request_mode,
                                                        measure_responses,
                                                        allocate_responses));

  adw_gizmo_set_focus_func (ADW_GIZMO (priv->contents),
                            (AdwGizmoFocusFunc) adw_widget_focus_child);
  adw_gizmo_set_grab_focus_func (ADW_GIZMO (priv->contents),
                            (AdwGizmoGrabFocusFunc) adw_widget_grab_focus_child);

  adw_gizmo_set_focus_func (ADW_GIZMO (priv->heading_bin),
                            (AdwGizmoFocusFunc) adw_widget_focus_child);
  adw_gizmo_set_grab_focus_func (ADW_GIZMO (priv->heading_bin),
                            (AdwGizmoGrabFocusFunc) adw_widget_grab_focus_child);

  adw_gizmo_set_focus_func (ADW_GIZMO (priv->response_area),
                            (AdwGizmoFocusFunc) adw_widget_focus_child);
  adw_gizmo_set_grab_focus_func (ADW_GIZMO (priv->response_area),
                            (AdwGizmoGrabFocusFunc) adw_widget_grab_focus_child);

  g_signal_connect (self, "notify::css-classes",
                    G_CALLBACK (css_classes_changed_cb), self);

  css_classes_changed_cb (self);
}

/* Custom tag handling was copied and modified
 * from gtk-size-group.c and gtk-scale.c */

typedef struct {
  GObject *object;
  GtkBuilder *builder;
  GSList *responses;
} ResponseParserData;

typedef struct {
  char *id;

  GString *label;
  char *context;
  gboolean translatable;

  AdwResponseAppearance appearance;
  gboolean enabled;

  int line;
  int col;
} ResponseData;

static void
response_data_free (gpointer data)
{
  ResponseData *response = data;

  g_free (response->id);
  g_string_free (response->label, TRUE);
  g_free (response->context);
  g_free (response);
}

static void
response_start_element (GtkBuildableParseContext  *context,
                        const char                *element_name,
                        const char               **names,
                        const char               **values,
                        gpointer                   user_data,
                        GError                   **error)
{
  ResponseParserData *data = user_data;

  if (strcmp (element_name, "response") == 0) {
    const char *id;
    const char *msg_context = NULL;
    gboolean translatable = FALSE;
    const char *appearance_str = NULL;
    AdwResponseAppearance appearance = ADW_RESPONSE_DEFAULT;
    gboolean enabled = TRUE;
    ResponseData *response;

    if (!_gtk_builder_check_parent (data->builder, context, "responses", error))
      return;

    if (!g_markup_collect_attributes (element_name, names, values, error,
                                      G_MARKUP_COLLECT_STRING, "id", &id,
                                      G_MARKUP_COLLECT_STRING | G_MARKUP_COLLECT_OPTIONAL, "appearance", &appearance_str,
                                      G_MARKUP_COLLECT_TRISTATE | G_MARKUP_COLLECT_OPTIONAL, "enabled", &enabled,
                                      G_MARKUP_COLLECT_BOOLEAN | G_MARKUP_COLLECT_OPTIONAL, "translatable", &translatable,
                                      G_MARKUP_COLLECT_STRING | G_MARKUP_COLLECT_OPTIONAL, "comments", NULL,
                                      G_MARKUP_COLLECT_STRING | G_MARKUP_COLLECT_OPTIONAL, "context", &msg_context,
                                      G_MARKUP_COLLECT_INVALID)) {
      _gtk_builder_prefix_error (data->builder, context, error);
      return;
    }

    if (appearance_str) {
      GValue gvalue = G_VALUE_INIT;

      if (!gtk_builder_value_from_string_type (data->builder, ADW_TYPE_RESPONSE_APPEARANCE, appearance_str, &gvalue, error)) {
        _gtk_builder_prefix_error (data->builder, context, error);
        return;
      }

      appearance = g_value_get_enum (&gvalue);
    }

    /* Normalize a tri-state value */
    enabled = enabled != FALSE;

    response = g_new (ResponseData, 1);
    response->id = g_strdup (id);
    response->context = g_strdup (msg_context);
    response->translatable = translatable;
    response->label = g_string_new ("");
    response->appearance = appearance;
    response->enabled = enabled;

    gtk_buildable_parse_context_get_position (context, &response->line, &response->col);
    data->responses = g_slist_prepend (data->responses, response);
  } else if (strcmp (element_name, "responses") == 0) {
    if (!_gtk_builder_check_parent (data->builder, context, "object", error))
      return;

    if (!g_markup_collect_attributes (element_name, names, values, error,
                                      G_MARKUP_COLLECT_INVALID, NULL, NULL,
                                      G_MARKUP_COLLECT_INVALID))
      _gtk_builder_prefix_error (data->builder, context, error);
  } else {
    _gtk_builder_error_unhandled_tag (data->builder, context,
                                      "AdwAlertDialog", element_name,
                                      error);
  }
}

static void
response_text (GtkBuildableParseContext  *context,
               const char                *text,
               gsize                      text_len,
               gpointer                   user_data,
               GError                   **error)
{
  ResponseParserData *data = user_data;

  if (strcmp (gtk_buildable_parse_context_get_element (context), "response") == 0) {
    ResponseData *response = data->responses->data;

    g_string_append_len (response->label, text, text_len);
  }
}

static const GtkBuildableParser response_parser = {
  response_start_element,
  NULL,
  response_text,
  NULL
};

static gboolean
adw_alert_dialog_buildable_custom_tag_start (GtkBuildable       *buildable,
                                             GtkBuilder         *builder,
                                             GObject            *child,
                                             const char         *tagname,
                                             GtkBuildableParser *parser,
                                             gpointer           *parser_data)
{
  ResponseParserData *data;

  if (child)
    return FALSE;

  if (strcmp (tagname, "responses") == 0) {
    data = g_new0 (ResponseParserData, 1);
    data->responses = NULL;
    data->object = G_OBJECT (buildable);
    data->builder = builder;

    *parser = response_parser;
    *parser_data = data;

    return TRUE;
  }

  return parent_buildable_iface->custom_tag_start (buildable, builder, child,
                                                   tagname, parser, parser_data);
}

static void
adw_alert_dialog_buildable_custom_finished (GtkBuildable *buildable,
                                            GtkBuilder   *builder,
                                            GObject      *child,
                                            const char   *tagname,
                                            gpointer      user_data)
{
  GSList *l;
  ResponseParserData *data;

  if (strcmp (tagname, "responses") != 0) {
    parent_buildable_iface->custom_finished (buildable, builder, child,
                                             tagname, user_data);
    return;
  }

  data = (ResponseParserData*)user_data;
  data->responses = g_slist_reverse (data->responses);

  for (l = data->responses; l; l = l->next) {
    ResponseData *response = l->data;
    const char *label;

    if (response->translatable && response->label->len)
      label = _gtk_builder_parser_translate (gtk_builder_get_translation_domain (builder),
                                             response->context,
                                             response->label->str);
    else
      label = response->label->str;

    adw_alert_dialog_add_response (ADW_ALERT_DIALOG (data->object),
                                   response->id, label);

    if (response->appearance != ADW_RESPONSE_DEFAULT)
      adw_alert_dialog_set_response_appearance (ADW_ALERT_DIALOG (data->object),
                                                response->id, response->appearance);

    if (!response->enabled)
      adw_alert_dialog_set_response_enabled (ADW_ALERT_DIALOG (data->object),
                                             response->id, FALSE);
  }

  g_slist_free_full (data->responses, response_data_free);
  g_free (data);
}

static void
adw_alert_dialog_buildable_add_child (GtkBuildable *buildable,
                                      GtkBuilder   *builder,
                                      GObject      *child,
                                      const char   *type)
{
  AdwAlertDialog *self = ADW_ALERT_DIALOG (buildable);

  if (GTK_IS_WIDGET (child))
    adw_alert_dialog_set_extra_child (self, GTK_WIDGET (child));
  else
    parent_buildable_iface->add_child (buildable, builder, child, type);
}

static void
adw_alert_dialog_buildable_init (GtkBuildableIface *iface)
{
  parent_buildable_iface = g_type_interface_peek_parent (iface);

  iface->add_child = adw_alert_dialog_buildable_add_child;
  iface->custom_tag_start = adw_alert_dialog_buildable_custom_tag_start;
  iface->custom_finished = adw_alert_dialog_buildable_custom_finished;
}

/**
 * adw_alert_dialog_new:
 * @heading: (nullable): the heading
 * @body: (nullable): the body text
 *
 * Creates a new `AdwAlertDialog`.
 *
 * @heading and @body can be set to `NULL`. This can be useful if they need to
 * be formatted or use markup. In that case, set them to `NULL` and call
 * [method@AlertDialog.format_body] or similar methods afterwards:
 *
 * ```c
 * AdwDialog *dialog;
 *
 * dialog = adw_alert_dialog_new (_("Replace File?"), NULL);
 * adw_alert_dialog_format_body (ADW_ALERT_DIALOG (dialog),
 *                               _("A file named “%s” already exists.  Do you want to replace it?"),
 *                               filename);
 * ```
 *
 * Returns: the newly created `AdwAlertDialog`
 *
 * Since: 1.5
 */
AdwDialog *
adw_alert_dialog_new (const char *heading,
                      const char *body)
{
  AdwDialog *dialog;

  dialog = g_object_new (ADW_TYPE_ALERT_DIALOG, NULL);

  if (heading)
    adw_alert_dialog_set_heading (ADW_ALERT_DIALOG (dialog), heading);

  if (body)
    adw_alert_dialog_set_body (ADW_ALERT_DIALOG (dialog), body);

  return dialog;
}

/**
 * adw_alert_dialog_get_heading:
 * @self: an alert dialog
 *
 * Gets the heading of @self.
 *
 * Returns: (nullable): the heading of @self.
 *
 * Since: 1.5
 */
const char *
adw_alert_dialog_get_heading (AdwAlertDialog *self)
{
  AdwAlertDialogPrivate *priv;

  g_return_val_if_fail (ADW_IS_ALERT_DIALOG (self), NULL);

  priv = adw_alert_dialog_get_instance_private (self);

  return priv->heading;
}

/**
 * adw_alert_dialog_set_heading:
 * @self: an alert dialog
 * @heading: (nullable): the heading of @self
 *
 * Sets the heading of @self.
 *
 * Since: 1.5
 */
void
adw_alert_dialog_set_heading (AdwAlertDialog *self,
                              const char     *heading)
{
  AdwAlertDialogPrivate *priv;

  g_return_if_fail (ADW_IS_ALERT_DIALOG (self));
  g_return_if_fail (heading != NULL);

  priv = adw_alert_dialog_get_instance_private (self);

  if (!g_set_str (&priv->heading, heading))
    return;

  gtk_label_set_label (GTK_LABEL (priv->heading_label), heading);
  gtk_label_set_label (GTK_LABEL (priv->heading_label_small), heading);

  gtk_widget_set_visible (priv->heading_bin, heading && *heading);

  if (heading && *heading)
    gtk_widget_add_css_class (GTK_WIDGET (priv->message_area), "has-heading");
  else
    gtk_widget_remove_css_class (GTK_WIDGET (priv->message_area), "has-heading");

  update_window_title (self);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_HEADING]);
}

/**
 * adw_alert_dialog_get_heading_use_markup:
 * @self: an alert dialog
 *
 * Gets whether the heading of @self includes Pango markup.
 *
 * Returns: whether @self uses markup for heading
 *
 * Since: 1.5
 */
gboolean
adw_alert_dialog_get_heading_use_markup (AdwAlertDialog *self)
{
  AdwAlertDialogPrivate *priv;

  g_return_val_if_fail (ADW_IS_ALERT_DIALOG (self), FALSE);

  priv = adw_alert_dialog_get_instance_private (self);

  return priv->heading_use_markup;
}

/**
 * adw_alert_dialog_set_heading_use_markup:
 * @self: an alert dialog
 * @use_markup: whether to use markup for heading
 *
 * Sets whether the heading of @self includes Pango markup.
 *
 * See [func@Pango.parse_markup].
 *
 * Since: 1.5
 */
void
adw_alert_dialog_set_heading_use_markup (AdwAlertDialog *self,
                                         gboolean        use_markup)
{
  AdwAlertDialogPrivate *priv;

  g_return_if_fail (ADW_IS_ALERT_DIALOG (self));

  priv = adw_alert_dialog_get_instance_private (self);

  use_markup = !!use_markup;

  if (use_markup == priv->heading_use_markup)
    return;

  priv->heading_use_markup = use_markup;

  gtk_label_set_use_markup (GTK_LABEL (priv->heading_label), use_markup);
  gtk_label_set_use_markup (GTK_LABEL (priv->heading_label_small), use_markup);

  update_window_title (self);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_HEADING_USE_MARKUP]);
}


/**
 * adw_alert_dialog_format_heading:
 * @self: an alert dialog
 * @format: the formatted string for the heading
 * @...: the parameters to insert into @format
 *
 * Sets the formatted heading of @self.
 *
 * See [property@AlertDialog:heading].
 *
 * Since: 1.5
 */
void
adw_alert_dialog_format_heading (AdwAlertDialog *self,
                                 const char     *format,
                                 ...)
{
  va_list args;

  g_return_if_fail (ADW_IS_ALERT_DIALOG (self));
  g_return_if_fail (format != NULL);

  g_object_freeze_notify (G_OBJECT (self));

  adw_alert_dialog_set_heading_use_markup (self, FALSE);

  if (format) {
    char *heading;

    va_start (args, format);
    heading = g_strdup_vprintf (format, args);
    va_end (args);

    adw_alert_dialog_set_heading (self, heading);

    g_free (heading);
  } else {
    adw_alert_dialog_set_heading (self, NULL);
  }

  g_object_thaw_notify (G_OBJECT (self));
}

/**
 * adw_alert_dialog_format_heading_markup:
 * @self: an alert dialog
 * @format: the formatted string for the heading with Pango markup
 * @...: the parameters to insert into @format
 *
 * Sets the formatted heading of @self with Pango markup.
 *
 * The @format is assumed to contain Pango markup.
 *
 * Special XML characters in the `printf()` arguments passed to this function
 * will automatically be escaped as necessary, see
 * [func@GLib.markup_printf_escaped].
 *
 * See [property@AlertDialog:heading].
 *
 * Since: 1.5
 */
void
adw_alert_dialog_format_heading_markup (AdwAlertDialog *self,
                                        const char     *format,
                                        ...)
{
  va_list args;

  g_return_if_fail (ADW_IS_ALERT_DIALOG (self));
  g_return_if_fail (format != NULL);

  g_object_freeze_notify (G_OBJECT (self));

  adw_alert_dialog_set_heading_use_markup (self, TRUE);

  if (format) {
    char *heading;

    va_start (args, format);
    heading = g_markup_vprintf_escaped (format, args);
    va_end (args);

    adw_alert_dialog_set_heading (self, heading);

    g_free (heading);
  } else {
    adw_alert_dialog_set_heading (self, "");
  }

  g_object_thaw_notify (G_OBJECT (self));
}

/**
 * adw_alert_dialog_get_body:
 * @self: an alert dialog
 *
 * Gets the body text of @self.
 *
 * Returns: the body of @self.
 *
 * Since: 1.5
 */
const char *
adw_alert_dialog_get_body (AdwAlertDialog *self)
{
  AdwAlertDialogPrivate *priv;

  g_return_val_if_fail (ADW_IS_ALERT_DIALOG (self), NULL);

  priv = adw_alert_dialog_get_instance_private (self);

  return priv->body;
}

/**
 * adw_alert_dialog_set_body:
 * @self: an alert dialog
 * @body: the body of @self
 *
 * Sets the body text of @self.
 *
 * Since: 1.5
 */
void
adw_alert_dialog_set_body (AdwAlertDialog *self,
                           const char     *body)
{
  AdwAlertDialogPrivate *priv;

  g_return_if_fail (ADW_IS_ALERT_DIALOG (self));
  g_return_if_fail (body != NULL);

  priv = adw_alert_dialog_get_instance_private (self);

  if (!g_set_str (&priv->body, body))
    return;

  gtk_label_set_label (GTK_LABEL (priv->body_label), body);
  gtk_widget_set_visible (priv->body_label, body && *body);

  if (body && *body)
    gtk_widget_add_css_class (GTK_WIDGET (priv->message_area), "has-body");
  else
    gtk_widget_remove_css_class (GTK_WIDGET (priv->message_area), "has-body");

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_BODY]);
}

/**
 * adw_alert_dialog_get_body_use_markup:
 * @self: an alert dialog
 *
 * Gets whether the body text of @self includes Pango markup.
 *
 * Returns: whether @self uses markup for body text
 *
 * Since: 1.5
 */
gboolean
adw_alert_dialog_get_body_use_markup (AdwAlertDialog *self)
{
  AdwAlertDialogPrivate *priv;

  g_return_val_if_fail (ADW_IS_ALERT_DIALOG (self), FALSE);

  priv = adw_alert_dialog_get_instance_private (self);

  return priv->body_use_markup;
}

/**
 * adw_alert_dialog_set_body_use_markup:
 * @self: an alert dialog
 * @use_markup: whether to use markup for body text
 *
 * Sets whether the body text of @self includes Pango markup.
 *
 * See [func@Pango.parse_markup].
 *
 * Since: 1.5
 */
void
adw_alert_dialog_set_body_use_markup (AdwAlertDialog *self,
                                      gboolean        use_markup)
{
  AdwAlertDialogPrivate *priv;

  g_return_if_fail (ADW_IS_ALERT_DIALOG (self));

  priv = adw_alert_dialog_get_instance_private (self);

  use_markup = !!use_markup;

  if (use_markup == priv->body_use_markup)
    return;

  priv->body_use_markup = use_markup;

  gtk_label_set_use_markup (GTK_LABEL (priv->body_label), use_markup);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_BODY_USE_MARKUP]);
}

/**
 * adw_alert_dialog_format_body:
 * @self: an alert dialog
 * @format: the formatted string for the body text
 * @...: the parameters to insert into @format
 *
 * Sets the formatted body text of @self.
 *
 * See [property@AlertDialog:body].
 *
 * Since: 1.5
 */
void
adw_alert_dialog_format_body (AdwAlertDialog *self,
                              const char     *format,
                              ...)
{
  va_list args;

  g_return_if_fail (ADW_IS_ALERT_DIALOG (self));
  g_return_if_fail (format != NULL);

  g_object_freeze_notify (G_OBJECT (self));

  adw_alert_dialog_set_body_use_markup (self, FALSE);

  if (format) {
    char *body;

    va_start (args, format);
    body = g_strdup_vprintf (format, args);
    va_end (args);

    adw_alert_dialog_set_body (self, body);

    g_free (body);
  } else {
    adw_alert_dialog_set_body (self, "");
  }

  g_object_thaw_notify (G_OBJECT (self));
}

/**
 * adw_alert_dialog_format_body_markup:
 * @self: an alert dialog
 * @format: the formatted string for the body text with Pango markup
 * @...: the parameters to insert into @format
 *
 * Sets the formatted body text of @self with Pango markup.
 *
 * The @format is assumed to contain Pango markup.
 *
 * Special XML characters in the `printf()` arguments passed to this function
 * will automatically be escaped as necessary, see
 * [func@GLib.markup_printf_escaped].
 *
 * See [property@AlertDialog:body].
 *
 * Since: 1.5
 */
void
adw_alert_dialog_format_body_markup (AdwAlertDialog *self,
                                     const char     *format,
                                     ...)
{
  va_list args;

  g_return_if_fail (ADW_IS_ALERT_DIALOG (self));
  g_return_if_fail (format != NULL);

  g_object_freeze_notify (G_OBJECT (self));

  adw_alert_dialog_set_body_use_markup (self, TRUE);

  if (format) {
    char *body;

    va_start (args, format);
    body = g_markup_vprintf_escaped (format, args);
    va_end (args);

    adw_alert_dialog_set_body (self, body);

    g_free (body);
  } else {
    adw_alert_dialog_set_body (self, NULL);
  }

  g_object_thaw_notify (G_OBJECT (self));
}

/**
 * adw_alert_dialog_get_extra_child:
 * @self: an alert dialog
 *
 * Gets the child widget of @self.
 *
 * Returns: (nullable) (transfer none): the child widget of @self.
 *
 * Since: 1.5
 */
GtkWidget *
adw_alert_dialog_get_extra_child (AdwAlertDialog *self)
{
  AdwAlertDialogPrivate *priv;

  g_return_val_if_fail (ADW_IS_ALERT_DIALOG (self), NULL);

  priv = adw_alert_dialog_get_instance_private (self);

  return priv->child;
}

/**
 * adw_alert_dialog_set_extra_child:
 * @self: an alert dialog
 * @child: (nullable): the child widget
 *
 * Sets the child widget of @self.
 *
 * The child widget is displayed below the heading and body.
 *
 * Since: 1.5
 */
void
adw_alert_dialog_set_extra_child (AdwAlertDialog *self,
                                  GtkWidget      *child)
{
  AdwAlertDialogPrivate *priv;

  g_return_if_fail (ADW_IS_ALERT_DIALOG (self));
  g_return_if_fail (child == NULL || GTK_IS_WIDGET (child));

  priv = adw_alert_dialog_get_instance_private (self);

  if (child == priv->child)
    return;

  if (child)
    g_return_if_fail (gtk_widget_get_parent (child) == NULL);

  priv->child = child;
  adw_bin_set_child (ADW_BIN (priv->child_bin), child);
  gtk_widget_set_visible (priv->child_bin, child != NULL);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_EXTRA_CHILD]);
}

/**
 * adw_alert_dialog_get_prefer_wide_layout:
 * @self: an alert dialog
 *
 * Gets whether @self prefers wide layout.
 *
 * Returns: whether to prefer wide layout
 *
 * Since: 1.5
 */
gboolean
adw_alert_dialog_get_prefer_wide_layout (AdwAlertDialog *self)
{
  AdwAlertDialogPrivate *priv;

  g_return_val_if_fail (ADW_IS_ALERT_DIALOG (self), FALSE);

  priv = adw_alert_dialog_get_instance_private (self);

  return priv->prefer_wide_layout;
}

/**
 * adw_alert_dialog_set_prefer_wide_layout:
 * @self: an alert dialog
 * @prefer_wide_layout: whether to prefer wide layout
 *
 * Sets whether @self prefers wide layout.
 *
 * Prefer horizontal button layout when possible, and wider dialog width
 * otherwise.
 *
 * Since: 1.6
 */
void
adw_alert_dialog_set_prefer_wide_layout (AdwAlertDialog *self,
                                         gboolean        prefer_wide_layout)
{
  AdwAlertDialogPrivate *priv;

  g_return_if_fail (ADW_IS_ALERT_DIALOG (self));

  priv = adw_alert_dialog_get_instance_private (self);

  prefer_wide_layout = !!prefer_wide_layout;

  if (prefer_wide_layout == priv->prefer_wide_layout)
    return;

  priv->prefer_wide_layout = prefer_wide_layout;

  gtk_widget_queue_resize (priv->contents);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_PREFER_WIDE_LAYOUT]);
}

/**
 * adw_alert_dialog_add_response:
 * @self: an alert dialog
 * @id: the response ID
 * @label: the response label
 *
 * Adds a response with @id and @label to @self.
 *
 * Responses are represented as buttons in the dialog.
 *
 * Response ID must be unique. It will be used in [signal@AlertDialog::response]
 * to tell which response had been activated, as well as to inspect and modify
 * the response later.
 *
 * An embedded underline in @label indicates a mnemonic.
 *
 * [method@AlertDialog.set_response_label] can be used to change the response
 * label after it had been added.
 *
 * [method@AlertDialog.set_response_enabled] and
 * [method@AlertDialog.set_response_appearance] can be used to customize the
 * responses further.
 *
 * Since: 1.5
 */
void
adw_alert_dialog_add_response (AdwAlertDialog *self,
                               const char     *id,
                               const char     *label)
{
  AdwAlertDialogPrivate *priv;
  ResponseInfo *info;

  g_return_if_fail (ADW_IS_ALERT_DIALOG (self));
  g_return_if_fail (id != NULL);
  g_return_if_fail (label != NULL);

  priv = adw_alert_dialog_get_instance_private (self);

  if (find_response (self, id)) {
    g_critical ("Trying to add a response with id '%s' to an "
                "AdwAlertDialog, but such a response already exists", id);
    return;
  }

  info = g_new0 (ResponseInfo, 1);

  info->dialog = self;
  info->id = g_quark_from_string (id);
  info->label = g_strdup (label);
  info->appearance = ADW_RESPONSE_DEFAULT;
  info->enabled = TRUE;

  info->button = create_response_button (self, info);
  gtk_widget_set_parent (info->button, priv->response_area);

  priv->responses = g_list_append (priv->responses, info);
  g_hash_table_insert (priv->id_to_response, g_strdup (id), info);

  if (priv->default_response == info->id)
    adw_dialog_set_default_widget (ADW_DIALOG (self), info->button);
}

/**
 * adw_alert_dialog_add_responses: (skip)
 * @self: an alert dialog
 * @first_id: response id
 * @...: label for first response, then more id-label pairs
 *
 * Adds multiple responses to @self.
 *
 * This is the same as calling [method@AlertDialog.add_response] repeatedly. The
 * variable argument list should be `NULL`-terminated list of response IDs and
 * labels.
 *
 * Example:
 *
 * ```c
 * adw_alert_dialog_add_responses (dialog,
 *                                 "cancel",  _("_Cancel"),
 *                                 "discard", _("_Discard"),
 *                                 "save",    _("_Save"),
 *                                 NULL);
 * ```
 *
 * Since: 1.5
 */
void
adw_alert_dialog_add_responses (AdwAlertDialog *self,
                                const char     *first_id,
                                ...)
{
  va_list args;
  const char *id, *label;

  g_return_if_fail (ADW_IS_ALERT_DIALOG (self));

  if (!first_id)
    return;

  va_start (args, first_id);

  id = first_id;
  label = va_arg (args, const char *);

  while (id) {
    adw_alert_dialog_add_response (self, id, label);

    id = va_arg (args, const char *);
    if (!id)
      break;

    label = va_arg (args, const char *);
  }

  va_end (args);
}

/**
 * adw_alert_dialog_remove_response:
 * @self: an alert dialog
 * @id: the response ID
 *
 * Removes a response from @self.
 *
 * Since: 1.5
 */
void
adw_alert_dialog_remove_response (AdwAlertDialog *self,
                                  const char     *id)
{
  AdwAlertDialogPrivate *priv;
  ResponseInfo *info;

  g_return_if_fail (ADW_IS_ALERT_DIALOG (self));
  g_return_if_fail (id != NULL);

  priv = adw_alert_dialog_get_instance_private (self);
  info = find_response (self, id);

  if (!info) {
    g_critical ("Trying to remove a response with id '%s' from an "
                "AdwAlertDialog, but such a response does not exist",
                id);
    return;
  }

  if (priv->default_response == info->id)
    adw_dialog_set_default_widget (ADW_DIALOG (self), NULL);

  gtk_widget_unparent (info->button);

  priv->responses = g_list_remove (priv->responses, info);
  g_hash_table_remove (priv->id_to_response, id);

  response_info_free (info);
}

/**
 * adw_alert_dialog_get_response_label:
 * @self: an alert dialog
 * @response: a response ID
 *
 * Gets the label of @response.
 *
 * See [method@AlertDialog.set_response_label].
 *
 * Returns: the label of @response
 *
 * Since: 1.5
 */
const char *
adw_alert_dialog_get_response_label (AdwAlertDialog *self,
                                     const char     *response)
{
  ResponseInfo *info;

  g_return_val_if_fail (ADW_IS_ALERT_DIALOG (self), NULL);
  g_return_val_if_fail (response != NULL, NULL);
  g_return_val_if_fail (adw_alert_dialog_has_response (self, response), NULL);

  info = find_response (self, response);

  return info->label;
}

/**
 * adw_alert_dialog_set_response_label:
 * @self: an alert dialog
 * @response: a response ID
 * @label: the label of @response
 *
 * Sets the label of @response to @label.
 *
 * Labels are displayed on the dialog buttons. An embedded underline in @label
 * indicates a mnemonic.
 *
 * Since: 1.5
 */
void
adw_alert_dialog_set_response_label (AdwAlertDialog *self,
                                     const char     *response,
                                     const char     *label)
{
  ResponseInfo *info;

  g_return_if_fail (ADW_IS_ALERT_DIALOG (self));
  g_return_if_fail (response != NULL);
  g_return_if_fail (label != NULL);
  g_return_if_fail (adw_alert_dialog_has_response (self, response));

  info = find_response (self, response);

  g_set_str (&info->label, label);

  gtk_button_set_label (GTK_BUTTON (info->button), label);
}

/**
 * adw_alert_dialog_get_response_appearance:
 * @self: an alert dialog
 * @response: a response ID
 *
 * Gets the appearance of @response.
 *
 * See [method@AlertDialog.set_response_appearance].
 *
 * Returns: the appearance of @response
 *
 * Since: 1.5
 */
AdwResponseAppearance
adw_alert_dialog_get_response_appearance (AdwAlertDialog *self,
                                          const char     *response)
{
  ResponseInfo *info;

  g_return_val_if_fail (ADW_IS_ALERT_DIALOG (self), FALSE);
  g_return_val_if_fail (response != NULL, FALSE);
  g_return_val_if_fail (adw_alert_dialog_has_response (self, response), FALSE);

  info = find_response (self, response);

  return info->appearance;
}

/**
 * adw_alert_dialog_set_response_appearance:
 * @self: an alert dialog
 * @response: a response ID
 * @appearance: appearance for @response
 *
 * Sets the appearance for @response.
 *
 * <picture>
 *   <source srcset="alert-dialog-appearance-dark.png" media="(prefers-color-scheme: dark)">
 *   <img src="alert-dialog-appearance.png" alt="alert-dialog-appearance">
 * </picture>
 *
 * Use `ADW_RESPONSE_SUGGESTED` to mark important responses such as the
 * affirmative action, like the Save button in the example.
 *
 * Use `ADW_RESPONSE_DESTRUCTIVE` to draw attention to the potentially damaging
 * consequences of using @response. This appearance acts as a warning to the
 * user. The Discard button in the example is using this appearance.
 *
 * The default appearance is `ADW_RESPONSE_DEFAULT`.
 *
 * Negative responses like Cancel or Close should use the default appearance.
 *
 * Since: 1.5
 */
void
adw_alert_dialog_set_response_appearance (AdwAlertDialog        *self,
                                          const char            *response,
                                          AdwResponseAppearance  appearance)
{
  ResponseInfo *info;

  g_return_if_fail (ADW_IS_ALERT_DIALOG (self));
  g_return_if_fail (response != NULL);
  g_return_if_fail (appearance >= ADW_RESPONSE_DEFAULT &&
                    appearance <= ADW_RESPONSE_DESTRUCTIVE);
  g_return_if_fail (adw_alert_dialog_has_response (self, response));

  info = find_response (self, response);

  if (appearance == info->appearance)
    return;

  info->appearance = appearance;

  if (info->appearance == ADW_RESPONSE_SUGGESTED)
    gtk_widget_add_css_class (info->button, "suggested-action");
  else
    gtk_widget_remove_css_class (info->button, "suggested-action");

  if (info->appearance == ADW_RESPONSE_DESTRUCTIVE)
    gtk_widget_add_css_class (info->button, "destructive-action");
  else
    gtk_widget_remove_css_class (info->button, "destructive-action");
}

/**
 * adw_alert_dialog_get_response_enabled:
 * @self: an alert dialog
 * @response: a response ID
 *
 * Gets whether @response is enabled.
 *
 * See [method@AlertDialog.set_response_enabled].
 *
 * Returns: whether @response is enabled
 *
 * Since: 1.5
 */
gboolean
adw_alert_dialog_get_response_enabled (AdwAlertDialog *self,
                                       const char     *response)
{
  ResponseInfo *info;

  g_return_val_if_fail (ADW_IS_ALERT_DIALOG (self), FALSE);
  g_return_val_if_fail (response != NULL, FALSE);
  g_return_val_if_fail (adw_alert_dialog_has_response (self, response), FALSE);

  info = find_response (self, response);

  return info->enabled;
}

/**
 * adw_alert_dialog_set_response_enabled:
 * @self: an alert dialog
 * @response: a response ID
 * @enabled: whether to enable @response
 *
 * Sets whether @response is enabled.
 *
 * If @response is not enabled, the corresponding button will have
 * [property@Gtk.Widget:sensitive] set to `FALSE` and it can't be activated as
 * a default response.
 *
 * @response can still be used as [property@AlertDialog:close-response] while
 * it's not enabled.
 *
 * Responses are enabled by default.
 *
 * Since: 1.5
 */
void
adw_alert_dialog_set_response_enabled (AdwAlertDialog *self,
                                       const char     *response,
                                       gboolean        enabled)
{
  ResponseInfo *info;

  g_return_if_fail (ADW_IS_ALERT_DIALOG (self));
  g_return_if_fail (response != NULL);
  g_return_if_fail (adw_alert_dialog_has_response (self, response));

  info = find_response (self, response);

  enabled = !!enabled;

  if (enabled == info->enabled)
    return;

  info->enabled = enabled;

  gtk_widget_set_sensitive (info->button, info->enabled);
}

/**
 * adw_alert_dialog_get_default_response:
 * @self: an alert dialog
 *
 * Gets the ID of the default response of @self.
 *
 * Returns: (nullable): the default response ID
 *
 * Since: 1.5
 */
const char *
adw_alert_dialog_get_default_response (AdwAlertDialog *self)
{
  AdwAlertDialogPrivate *priv;

  g_return_val_if_fail (ADW_IS_ALERT_DIALOG (self), NULL);

  priv = adw_alert_dialog_get_instance_private (self);

  if (!priv->default_response)
    return NULL;

  return g_quark_to_string (priv->default_response);
}

/**
 * adw_alert_dialog_set_default_response:
 * @self: an alert dialog
 * @response: (nullable): the default response ID
 *
 * Sets the ID of the default response of @self.
 *
 * The button corresponding to this response will be set as the default widget
 * of @self.
 *
 * If not set, the default widget will not be set, and the last added response
 * will be focused by default.
 *
 * See [property@Dialog:default-widget].
 *
 * Since: 1.5
 */
void
adw_alert_dialog_set_default_response (AdwAlertDialog *self,
                                       const char     *response)
{
  AdwAlertDialogPrivate *priv;
  GQuark quark;
  ResponseInfo *info;

  g_return_if_fail (ADW_IS_ALERT_DIALOG (self));

  priv = adw_alert_dialog_get_instance_private (self);
  quark = g_quark_from_string (response);

  if (quark == priv->default_response)
    return;

  priv->default_response = quark;

  info = find_response (self, response);

  if (info)
    adw_dialog_set_default_widget (ADW_DIALOG (self), info->button);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_DEFAULT_RESPONSE]);
}

/**
 * adw_alert_dialog_get_close_response:
 * @self: an alert dialog
 *
 * Gets the ID of the close response of @self.
 *
 * Returns: the close response ID
 *
 * Since: 1.5
 */
const char *
adw_alert_dialog_get_close_response (AdwAlertDialog *self)
{
  AdwAlertDialogPrivate *priv;

  g_return_val_if_fail (ADW_IS_ALERT_DIALOG (self), NULL);

  priv = adw_alert_dialog_get_instance_private (self);

  return g_quark_to_string (priv->close_response);
}

/**
 * adw_alert_dialog_set_close_response:
 * @self: an alert dialog
 * @response: the close response ID
 *
 * Sets the ID of the close response of @self.
 *
 * It will be passed to [signal@AlertDialog::response] if the dialog is closed
 * by pressing <kbd>Escape</kbd> or with a system action.
 *
 * It doesn't have to correspond to any of the responses in the dialog.
 *
 * The default close response is `close`.
 *
 * Since: 1.5
 */
void
adw_alert_dialog_set_close_response (AdwAlertDialog *self,
                                     const char     *response)
{
  AdwAlertDialogPrivate *priv;
  GQuark quark;

  g_return_if_fail (ADW_IS_ALERT_DIALOG (self));
  g_return_if_fail (response != NULL);

  priv = adw_alert_dialog_get_instance_private (self);
  quark = g_quark_from_string (response);

  if (quark == priv->close_response)
    return;

  priv->close_response = quark;

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_CLOSE_RESPONSE]);
}

/**
 * adw_alert_dialog_has_response:
 * @self: an alert dialog
 * @response: response ID
 *
 * Gets whether @self has a response with the ID @response.
 *
 * Returns: whether @self has a response with the ID @response.
 *
 * Since: 1.5
 */
gboolean
adw_alert_dialog_has_response (AdwAlertDialog *self,
                               const char     *response)
{
  g_return_val_if_fail (ADW_IS_ALERT_DIALOG (self), FALSE);
  g_return_val_if_fail (response != NULL, FALSE);

  return find_response (self, response) != NULL;
}

static void choose_cancelled_cb (GCancellable *cancellable,
                                 GTask        *task);

static void
choose_response_cb (AdwAlertDialog *dialog,
                    const char     *response,
                    GTask          *task)
{
  GCancellable *cancellable = g_task_get_cancellable (task);

  if (cancellable)
    g_signal_handlers_disconnect_by_func (cancellable, choose_cancelled_cb, task);

  g_signal_handlers_disconnect_by_func (dialog, choose_response_cb, task);

  g_task_return_int (task, g_quark_from_string (response));

  g_object_unref (task);
}

static void
choose_cancelled_cb (GCancellable *cancellable,
                     GTask        *task)
{
  AdwAlertDialog *self = g_task_get_source_object (task);
  AdwAlertDialogPrivate *priv = adw_alert_dialog_get_instance_private (self);

  emit_response (self, priv->close_response);
}

/**
 * adw_alert_dialog_choose:
 * @self: an alert dialog
 * @parent: (nullable): the parent widget
 * @cancellable: (nullable): a `GCancellable` to cancel the operation
 * @callback: (scope async): a callback to call when the operation is complete
 * @user_data: data to pass to @callback
 *
 * This function shows @self to the user.
 *
 * If the window is an [class@Window] or [class@ApplicationWindow], the dialog
 * will be shown within it. Otherwise, it will be a separate window.
 *
 * Since: 1.5
 */
void
adw_alert_dialog_choose (AdwAlertDialog      *self,
                         GtkWidget           *parent,
                         GCancellable        *cancellable,
                         GAsyncReadyCallback  callback,
                         gpointer             user_data)
{
  GTask *task;

  g_return_if_fail (ADW_IS_ALERT_DIALOG (self));
  g_return_if_fail (parent == NULL || GTK_IS_WIDGET (parent));

  task = g_task_new (self, cancellable, callback, user_data);
  g_task_set_source_tag (task, adw_alert_dialog_choose);

  if (cancellable)
    g_signal_connect (cancellable, "cancelled", G_CALLBACK (choose_cancelled_cb), task);

  g_signal_connect (self, "response", G_CALLBACK (choose_response_cb), task);

  adw_dialog_present (ADW_DIALOG (self), parent);
}

/**
 * adw_alert_dialog_choose_finish:
 * @self: an alert dialog
 * @result: a `GAsyncResult`
 *
 * Finishes the [method@AlertDialog.choose] call and returns the response ID.
 *
 * Returns: the ID of the response that was selected, or
 *   [property@AlertDialog:close-response] if the call was cancelled.
 *
 * Since: 1.5
 */
const char *
adw_alert_dialog_choose_finish (AdwAlertDialog *self,
                                GAsyncResult   *result)
{
  GQuark id;
  g_return_val_if_fail (ADW_IS_ALERT_DIALOG (self), NULL);
  g_return_val_if_fail (g_task_is_valid (result, self), NULL);
  g_return_val_if_fail (g_task_get_source_tag (G_TASK (result)) == adw_alert_dialog_choose, NULL);

  id = g_task_propagate_int (G_TASK (result), NULL);

  return g_quark_to_string (id);
}

