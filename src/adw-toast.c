/*
 * Copyright (C) 2021 Maximiliano Sandoval <msandova@gnome.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "config.h"

#include "adw-toast-private.h"

#include "adw-macros-private.h"

/**
 * AdwToastPriority:
 * @ADW_TOAST_PRIORITY_NORMAL: the toast will be queued if another toast is
 *   already displayed.
 * @ADW_TOAST_PRIORITY_HIGH: the toast will be displayed immediately, pushing
 *   the previous toast into the queue instead.
 *
 * [class@Toast] behavior when another toast is already displayed.
 *
 * Since: 1.0
 */

/**
 * AdwToast:
 *
 * A helper object for [class@ToastOverlay].
 *
 * Toasts are meant to be passed into [method@ToastOverlay.add_toast] as
 * follows:
 *
 * ```c
 * adw_toast_overlay_add_toast (overlay, adw_toast_new (_("Simple Toast"));
 * ```
 *
 * <picture>
 *   <source srcset="toast-simple-dark.png" media="(prefers-color-scheme: dark)">
 *   <img src="toast-simple.png" alt="toast-simple">
 * </picture>
 *
 * Toasts always have a close button. They emit the [signal@Toast::dismissed]
 * signal when disappearing.
 *
 * [property@Toast:timeout] determines how long the toast stays on screen, while
 * [property@Toast:priority] determines how it behaves if another toast is
 * already being displayed.
 *
 * ## Actions
 *
 * Toasts can have one button on them, with a label and an attached
 * [iface@Gio.Action].
 *
 * ```c
 * AdwToast *toast = adw_toast_new (_("Toast with Action"));
 *
 * adw_toast_set_button_label (toast, _("_Example"));
 * adw_toast_set_action_name (toast, "win.example");
 *
 * adw_toast_overlay_add_toast (overlay, toast);
 * ```
 *
 * <picture>
 *   <source srcset="toast-action-dark.png" media="(prefers-color-scheme: dark)">
 *   <img src="toast-action.png" alt="toast-action">
 * </picture>
 *
 * ## Modifying toasts
 *
 * Toasts can be modified after they have been shown. For this, an `AdwToast`
 * reference must be kept around while the toast is visible.
 *
 * A common use case for this is using toasts as undo prompts that stack with
 * each other, allowing to batch undo the last deleted items:
 *
 * ```c
 *
 * static void
 * toast_undo_cb (GtkWidget  *sender,
 *                const char *action,
 *                GVariant   *param)
 * {
 *   // Undo the deletion
 * }
 *
 * static void
 * dismissed_cb (MyWindow *self)
 * {
 *   self->undo_toast = NULL;
 *
 *   // Permanently delete the items
 * }
 *
 * static void
 * delete_item (MyWindow *self,
 *              MyItem   *item)
 * {
 *   g_autofree char *title = NULL;
 *   int n_items;
 *
 *   // Mark the item as waiting for deletion
 *   n_items = ... // The number of waiting items
 *
 *   if (!self->undo_toast) {
 *     title = g_strdup_printf (_("‘%s’ deleted"), ...);
 *
 *     self->undo_toast = adw_toast_new (title);
 *
 *     adw_toast_set_priority (self->undo_toast, ADW_TOAST_PRIORITY_HIGH);
 *     adw_toast_set_button_label (self->undo_toast, _("_Undo"));
 *     adw_toast_set_action_name (self->undo_toast, "toast.undo");
 *
 *     g_signal_connect_swapped (self->undo_toast, "dismissed",
 *                               G_CALLBACK (dismissed_cb), self);
 *
 *     adw_toast_overlay_add_toast (self->toast_overlay, self->undo_toast);
 *
 *     return;
 *   }
 *
 *   title =
 *     g_strdup_printf (ngettext ("<span font_features='tnum=1'>%d</span> item deleted",
 *                                "<span font_features='tnum=1'>%d</span> items deleted",
 *                                n_items), n_items);
 *
 *   adw_toast_set_title (self->undo_toast, title);
 * }
 *
 * static void
 * my_window_class_init (MyWindowClass *klass)
 * {
 *   GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
 *
 *   gtk_widget_class_install_action (widget_class, "toast.undo", NULL, toast_undo_cb);
 * }
 * ```
 *
 * <picture>
 *   <source srcset="toast-undo-dark.png" media="(prefers-color-scheme: dark)">
 *   <img src="toast-undo.png" alt="toast-undo">
 * </picture>
 *
 * Since: 1.0
 */

struct _AdwToast {
  GObject parent_instance;

  char *title;
  char *button_label;
  char *action_name;
  GVariant *action_target;
  AdwToastPriority priority;
  guint timeout;

  gboolean added;
};

enum {
  PROP_0,
  PROP_TITLE,
  PROP_BUTTON_LABEL,
  PROP_ACTION_NAME,
  PROP_ACTION_TARGET,
  PROP_PRIORITY,
  PROP_TIMEOUT,
  LAST_PROP,
};

static GParamSpec *props[LAST_PROP];

enum {
  SIGNAL_DISMISSED,
  SIGNAL_LAST_SIGNAL,
};

static guint signals[SIGNAL_LAST_SIGNAL];

G_DEFINE_FINAL_TYPE (AdwToast, adw_toast, G_TYPE_OBJECT)

static void
dismissed_cb (AdwToast *self)
{
  self->added = FALSE;
}

static void
adw_toast_finalize (GObject *object)
{
  AdwToast *self = ADW_TOAST (object);

  g_clear_pointer (&self->title, g_free);
  g_clear_pointer (&self->button_label, g_free);
  g_clear_pointer (&self->action_name, g_free);
  g_clear_pointer (&self->action_target, g_variant_unref);

  G_OBJECT_CLASS (adw_toast_parent_class)->finalize (object);
}

static void
adw_toast_get_property (GObject    *object,
                        guint       prop_id,
                        GValue     *value,
                        GParamSpec *pspec)
{
  AdwToast *self = ADW_TOAST (object);

  switch (prop_id) {
  case PROP_TITLE:
    g_value_set_string (value, adw_toast_get_title (self));
    break;
  case PROP_BUTTON_LABEL:
    g_value_set_string (value, adw_toast_get_button_label (self));
    break;
  case PROP_ACTION_NAME:
    g_value_set_string (value, adw_toast_get_action_name (self));
    break;
  case PROP_ACTION_TARGET:
    g_value_set_variant (value, adw_toast_get_action_target_value (self));
    break;
  case PROP_PRIORITY:
    g_value_set_enum (value, adw_toast_get_priority (self));
    break;
  case PROP_TIMEOUT:
    g_value_set_uint (value, adw_toast_get_timeout (self));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_toast_set_property (GObject      *object,
                        guint         prop_id,
                        const GValue *value,
                        GParamSpec   *pspec)
{
  AdwToast *self = ADW_TOAST (object);

  switch (prop_id) {
  case PROP_TITLE:
    adw_toast_set_title (self, g_value_get_string (value));
    break;
  case PROP_BUTTON_LABEL:
    adw_toast_set_button_label (self, g_value_get_string (value));
    break;
  case PROP_ACTION_NAME:
    adw_toast_set_action_name (self, g_value_get_string (value));
    break;
  case PROP_ACTION_TARGET:
    adw_toast_set_action_target_value (self, g_value_get_variant (value));
    break;
  case PROP_PRIORITY:
    adw_toast_set_priority (self, g_value_get_enum (value));
    break;
  case PROP_TIMEOUT:
    adw_toast_set_timeout (self, g_value_get_uint (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_toast_class_init (AdwToastClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = adw_toast_finalize;
  object_class->get_property = adw_toast_get_property;
  object_class->set_property = adw_toast_set_property;

  /**
   * AdwToast:title: (attributes org.gtk.Property.get=adw_toast_get_title org.gtk.Property.set=adw_toast_set_title)
   *
   * The title of the toast.
   *
   * The title can be marked up with the Pango text markup language.
   *
   * Since: 1.0
   */
  props[PROP_TITLE] =
    g_param_spec_string ("title",
                         "Title",
                         "The title of the toast",
                         "",
                         G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwToast:button-label: (attributes org.gtk.Property.get=adw_toast_get_button_label org.gtk.Property.set=adw_toast_set_button_label)
   *
   * The label to show on the button.
   *
   * Underlines in the button text can be used to indicate a mnemonic.
   *
   * If set to `NULL`, the button won't be shown.
   *
   * See [property@Toast:action-name].
   *
   * Since: 1.0
   */
  props[PROP_BUTTON_LABEL] =
    g_param_spec_string ("button-label",
                         "Button Label",
                         "The label to show on the button",
                         NULL,
                         G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwToast:action-name: (attributes org.gtk.Property.get=adw_toast_get_action_name org.gtk.Property.set=adw_toast_set_action_name)
   *
   * The name of the associated action.
   *
   * It will be activated when clicking the button.
   *
   * See [property@Toast:action-target].
   *
   * Since: 1.0
   */
  props[PROP_ACTION_NAME] =
    g_param_spec_string ("action-name",
                         "Action Name",
                         "The name of the associated action",
                         NULL,
                         G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwToast:action-target: (attributes org.gtk.Property.get=adw_toast_get_action_target_value org.gtk.Property.set=adw_toast_set_action_target_value)
   *
   * The parameter for action invocations.
   *
   * See [property@Toast:action-name].
   *
   * Since: 1.0
   */
  props[PROP_ACTION_TARGET] =
    g_param_spec_variant ("action-target",
                          "Action Target Value",
                          "The parameter for action invocations",
                          G_VARIANT_TYPE_ANY,
                          NULL,
                          G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwToast:priority: (attributes org.gtk.Property.get=adw_toast_get_priority org.gtk.Property.set=adw_toast_set_priority)
   *
   * The priority of the toast.
   *
   * Priority controls how the toast behaves when another toast is already
   * being displayed.
   *
   * If the priority is `ADW_TOAST_PRIORITY_NORMAL`, the toast will be queued.
   *
   * If the priority is `ADW_TOAST_PRIORITY_HIGH`, the toast will be displayed
   * immediately, pushing the previous toast into the queue instead.
   *
   * Since: 1.0
   */
  props[PROP_PRIORITY] =
    g_param_spec_enum ("priority",
                       "Priority",
                       "The priority of the toast",
                       ADW_TYPE_TOAST_PRIORITY,
                       ADW_TOAST_PRIORITY_NORMAL,
                       G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwToast:timeout: (attributes org.gtk.Property.get=adw_toast_get_timeout org.gtk.Property.set=adw_toast_set_timeout)
   *
   * The timeout of the toast, in seconds.
   *
   * If timeout is 0, the toast is displayed indefinitely until manually
   * dismissed.
   *
   * Toasts cannot disappear while being hovered, pressed (on touchscreen), or
   * have keyboard focus inside them.
   *
   * Since: 1.0
   */
  props[PROP_TIMEOUT] =
    g_param_spec_uint ("timeout",
                       "Timeout",
                       "The timeout of the toast, in seconds",
                       0, G_MAXUINT, 5,
                       G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  g_object_class_install_properties (object_class, LAST_PROP, props);

  /**
   * AdwToast::dismissed:
   *
   * Emitted when the toast has been dismissed.
   *
   * Since: 1.0
   */
  signals[SIGNAL_DISMISSED] =
    g_signal_new ("dismissed",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  0,
                  NULL, NULL, NULL,
                  G_TYPE_NONE, 0);
}

static void
adw_toast_init (AdwToast *self)
{
  self->title = g_strdup ("");
  self->priority = ADW_TOAST_PRIORITY_NORMAL;
  self->timeout = 5;

  g_signal_connect (self, "dismissed", G_CALLBACK (dismissed_cb), self);
}

/**
 * adw_toast_new:
 * @title: the title to be displayed
 *
 * Creates a new `AdwToast`.
 *
 * The toast will use @title as its title.
 *
 * @title can be marked up with the Pango text markup language.
 *
 * Returns: the new created `AdwToast`
 *
 * Since: 1.0
 */
AdwToast *
adw_toast_new (const char *title)
{
  g_return_val_if_fail (title != NULL, NULL);

  return g_object_new (ADW_TYPE_TOAST,
                       "title", title,
                       NULL);
}

/**
 * adw_toast_get_title: (attributes org.gtk.Method.get_property=title)
 * @self: a `AdwToast`
 *
 * Gets the title that will be displayed on the toast.
 *
 * Returns: the title
 *
 * Since: 1.0
 */
const char *
adw_toast_get_title (AdwToast *self)
{
  g_return_val_if_fail (ADW_IS_TOAST (self), NULL);

  return self->title;
}

/**
 * adw_toast_set_title: (attributes org.gtk.Method.set_property=title)
 * @self: a `AdwToast`
 * @title: a title
 *
 * Sets the title that will be displayed on the toast.
 *
 * Since: 1.0
 */
void
adw_toast_set_title (AdwToast   *self,
                     const char *title)
{
  g_return_if_fail (ADW_IS_TOAST (self));
  g_return_if_fail (title != NULL);

  if (!g_strcmp0 (self->title, title))
    return;

  g_clear_pointer (&self->title, g_free);
  self->title = g_strdup (title);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_TITLE]);
}

/**
 * adw_toast_get_button_label: (attributes org.gtk.Method.get_property=button-label)
 * @self: a `AdwToast`
 *
 * Gets the label to show on the button.
 *
 * Returns: (nullable): the button label
 *
 * Since: 1.0
 */
const char *
adw_toast_get_button_label (AdwToast *self)
{
  g_return_val_if_fail (ADW_IS_TOAST (self), NULL);

  return self->button_label;
}

/**
 * adw_toast_set_button_label: (attributes org.gtk.Method.set_property=button-label)
 * @self: a `AdwToast`
 * @button_label: (nullable): a button label
 *
 * Sets the label to show on the button.
 *
 * It set to `NULL`, the button won't be shown.
 *
 * Since: 1.0
 */
void
adw_toast_set_button_label (AdwToast   *self,
                            const char *button_label)
{
  g_return_if_fail (ADW_IS_TOAST (self));

  if (!g_strcmp0 (self->button_label, button_label))
    return;

  g_clear_pointer (&self->button_label, g_free);
  self->button_label = g_strdup (button_label);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_BUTTON_LABEL]);
}

/**
 * adw_toast_get_action_name: (attributes org.gtk.Method.get_property=action-name)
 * @self: a `AdwToast`
 *
 * Gets the name of the associated action.
 *
 * Returns: (nullable): the action name
 *
 * Since: 1.0
 */
const char *
adw_toast_get_action_name (AdwToast *self)
{
  g_return_val_if_fail (ADW_IS_TOAST (self), NULL);

  return self->action_name;
}

/**
 * adw_toast_set_action_name: (attributes org.gtk.Method.set_property=action-name)
 * @self: a `AdwToast`
 * @action_name: (nullable): the action name
 *
 * Sets the name of the associated action.
 *
 * Since: 1.0
 */
void
adw_toast_set_action_name (AdwToast   *self,
                           const char *action_name)
{
  g_return_if_fail (ADW_IS_TOAST (self));

  if (!g_strcmp0 (self->action_name, action_name))
    return;

  g_clear_pointer (&self->action_name, g_free);
  self->action_name = g_strdup (action_name);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_ACTION_NAME]);
}

/**
 * adw_toast_get_action_target_value: (attributes org.gtk.Method.get_property=action-target)
 * @self: a `AdwToast`
 *
 * Gets the parameter for action invocations.
 *
 * Returns: (transfer none) (nullable): the action target
 *
 * Since: 1.0
 */
GVariant *
adw_toast_get_action_target_value (AdwToast *self)
{
  g_return_val_if_fail (ADW_IS_TOAST (self), NULL);

  return self->action_target;
}

/**
 * adw_toast_set_action_target_value: (attributes org.gtk.Method.set_property=action-target)
 * @self: a `AdwToast`
 * @action_target: (nullable): the action target
 *
 * Sets the parameter for action invocations.
 *
 * If the @action_target variant has a floating reference this function
 * will sink it.
 *
 * Since: 1.0
 */
void
adw_toast_set_action_target_value (AdwToast *self,
                                   GVariant *action_target)
{
  g_return_if_fail (ADW_IS_TOAST (self));

  if (action_target == self->action_target)
    return;

  if (action_target && self->action_target &&
      g_variant_equal (action_target, self->action_target))
    return;

  g_clear_pointer (&self->action_target, g_variant_unref);
  if (action_target != NULL)
    self->action_target = g_variant_ref_sink (action_target);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_ACTION_TARGET]);
}

/**
 * adw_toast_set_action_target: (skip)
 * @self: a `AdwToast`
 * @format_string: (nullable): a [struct@GLib.Variant] format string
 * @...: arguments appropriate for @target_format
 *
 * Sets the parameter for action invocations.
 *
 * This is a convenience function that calls [ctor@GLib.Variant.new] for
 * @format_string and uses the result to call
 * [method@Toast.set_action_target_value].
 *
 * If you are setting a string-valued target and want to set
 * the action name at the same time, you can use
 * [method@Toast.set_detailed_action_name].

 * Since: 1.0
 */
void
adw_toast_set_action_target (AdwToast   *self,
                             const char *format_string,
                             ...)
{
  va_list args;

  va_start (args, format_string);
  adw_toast_set_action_target_value (self,
                                     g_variant_new_va (format_string,
                                                       NULL, &args));
  va_end (args);
}

/**
 * adw_toast_set_detailed_action_name:
 * @self: a `AdwToast`
 * @detailed_action_name: (nullable): the detailed action name
 *
 * Sets the action name and its parameter.
 *
 * @detailed_action_name is a string in the format accepted by
 * [func@Gio.Action.parse_detailed_name].
 *
 * Since: 1.0
 */
void
adw_toast_set_detailed_action_name (AdwToast   *self,
                                    const char *detailed_action_name)
{
  g_autofree char *name = NULL;
  g_autoptr (GVariant) target = NULL;
  g_autoptr (GError) error = NULL;

  g_return_if_fail (ADW_IS_TOAST (self));

  if (!detailed_action_name) {
    adw_toast_set_action_name (self, NULL);
    adw_toast_set_action_target_value (self, NULL);

    return;
  }

  if (!g_action_parse_detailed_name (detailed_action_name, &name, &target, &error)) {
    g_critical ("Couldn't parse detailed action name: %s", error->message);

    return;
  }

  adw_toast_set_action_name (self, name);
  adw_toast_set_action_target_value (self, target);
}

/**
 * adw_toast_get_priority: (attributes org.gtk.Method.get_property=priority)
 * @self: a `AdwToast`
 *
 * Gets priority for @self.
 *
 * Returns: the priority
 *
 * Since: 1.0
 */
AdwToastPriority
adw_toast_get_priority (AdwToast *self)
{
  g_return_val_if_fail (ADW_IS_TOAST (self), ADW_TOAST_PRIORITY_NORMAL);

  return self->priority;
}

/**
 * adw_toast_set_priority: (attributes org.gtk.Method.set_property=priority)
 * @self: a `AdwToast`
 * @priority: the priority
 *
 * Sets priority for @self.
 *
 * Priority controls how the toast behaves when another toast is already
 * being displayed.
 *
 * If @priority is `ADW_TOAST_PRIORITY_NORMAL`, the toast will be queued.
 *
 * If @priority is `ADW_TOAST_PRIORITY_HIGH`, the toast will be displayed immediately,
 * pushing the previous toast into the queue instead.
 *
 * Since: 1.0
 */
void
adw_toast_set_priority (AdwToast         *self,
                        AdwToastPriority  priority)
{
  g_return_if_fail (ADW_IS_TOAST (self));
  g_return_if_fail (priority >= ADW_TOAST_PRIORITY_NORMAL &&
                    priority <= ADW_TOAST_PRIORITY_HIGH);

  if (self->priority == priority)
    return;

  self->priority = priority;

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_PRIORITY]);
}

/**
 * adw_toast_get_timeout: (attributes org.gtk.Method.get_property=timeout)
 * @self: a `AdwToast`
 *
 * Gets timeout for @self.
 *
 * Returns: the timeout
 *
 * Since: 1.0
 */
guint
adw_toast_get_timeout (AdwToast *self)
{
  g_return_val_if_fail (ADW_IS_TOAST (self), 0);

  return self->timeout;
}

/**
 * adw_toast_set_timeout: (attributes org.gtk.Method.set_property=timeout)
 * @self: a `AdwToast`
 * @timeout: the timeout
 *
 * Sets timeout for @self.
 *
 * If @timeout is 0, the toast is displayed indefinitely until manually
 * dismissed.
 *
 * Toasts cannot disappear while being hovered, pressed (on touchscreen), or
 * have keyboard focus inside them.
 *
 * Since: 1.0
 */
void
adw_toast_set_timeout (AdwToast *self,
                       guint     timeout)
{
  g_return_if_fail (ADW_IS_TOAST (self));

  if (self->timeout == timeout)
    return;

  self->timeout = timeout;

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_TIMEOUT]);
}

/**
 * adw_toast_dismiss:
 * @self: a `AdwToast`
 *
 * Dismisses @self.
 *
 * Since: 1.0
 */
void
adw_toast_dismiss (AdwToast *self)
{
  g_return_if_fail (ADW_IS_TOAST (self));

  if (!self->added) {
    g_critical ("Trying to dismiss the toast '%s', but it isn't in an "
                "AdwToastOverlay yet", self->title);

    return;
  }

  g_signal_emit (self, signals[SIGNAL_DISMISSED], 0, NULL);
}

gboolean
adw_toast_get_added (AdwToast *self)
{
  g_return_val_if_fail (ADW_IS_TOAST (self), FALSE);

  return self->added;
}

void
adw_toast_set_added (AdwToast *self,
                     gboolean  added)
{
  g_return_if_fail (ADW_IS_TOAST (self));

  self->added = !!added;
}
