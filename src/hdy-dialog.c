/*
 * Copyright © 2018 Zander Brown <zbrown@gnome.org>
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#include "config.h"
#include <glib/gi18n-lib.h>

#include "hdy-dialog.h"

/**
 * SECTION:hdy-dialog
 * @short_description: An adaptive dialog
 * @title: HdyDialog
 * 
 * A #GtkDialog that adapts to smaller displays
 * 
 * Small is defined as:
 * |[<!-- language="C" -->
 * is_small = ((             width <= 400 && height <= 800) ||
 *             (maximized && width <= 800 && height <= 400));
 * ]|
 * 
 * In the smaller view a HdyDialog matches it's size to that of it's
 * parent and for ["Presentation Dialogs"](https://developer.gnome.org/hig/stable/dialogs.html)
 * uses a back button rather than close button to dismiss.
 * 
 * It's recommended that dialog contents are wrapped in a #GtkScrolledWindow
 * to ensure they don't overflow the screen
 * 
 * #HdyDialog works best when #GtkDialog:use-header-bar is %TRUE (which is 
 * the case when using hdy_dialog_new())
 * 
 * Design Information: [GitLab Issue](https://source.puri.sm/Librem5/libhandy/issues/52)
 * 
 * Ideally when using #HdyDialog you shouldn't need to know you are using
 * it rather than #GtkDialog however there are some notable differences:
 * #GtkWindow:modal is %TRUE by default as is #GtkWindow:destroy-with-parent as
 * the behaviour demonstrated by #HdyDialog would be a bad user experience
 * when not modal
 *
 * Since: 0.0.7
 */

/* Point at which we switch to mobile view */
#define SNAP_POINT_A 400
#define SNAP_POINT_B 800

typedef struct {
  GtkWindow *parent;
  gulong     size_handler;
  gint       old_width, old_height;
  GtkWidget *closebtn;
  gboolean   no_actions;
} HdyDialogPrivate;

G_DEFINE_TYPE_WITH_CODE (HdyDialog, hdy_dialog, GTK_TYPE_DIALOG,
                         G_ADD_PRIVATE (HdyDialog))

static void
update_titlebar (HdyDialog *self,
                 gboolean   is_small)
{
  HdyDialogPrivate *priv = hdy_dialog_get_instance_private (self);
  GtkWidget        *titlebar;

  titlebar = gtk_window_get_titlebar (GTK_WINDOW (self));

  /* We don't know what to do with things that aren't headerbars */
  g_return_if_fail (GTK_IS_HEADER_BAR (titlebar));

  /* Dialog already had close hidden (probably action dialog) */
  if (!priv->no_actions) {
    return;
  }

  /* When small show our custom button */
  if (is_small) {
    gtk_header_bar_set_show_close_button (GTK_HEADER_BAR (titlebar), FALSE);
    gtk_widget_show (priv->closebtn);
  } else {
    gtk_header_bar_set_show_close_button (GTK_HEADER_BAR (titlebar), TRUE);
    gtk_widget_hide (priv->closebtn);
  }
}

/* Controls the dialog size, called in reposnse to a GtkWidget::size-allocate
 * on the parent of GtkWidget::realize on the dialog */
static void
handle_size (HdyDialog *self, GtkWindow *parent)
{
  HdyDialogPrivate *priv = hdy_dialog_get_instance_private (self);
  gint              width, height;
  gboolean          maximized;
  gboolean          is_small;

  if (parent == NULL)
    return;

  /* Get the size of the parent */
  gtk_window_get_size (parent, &width, &height);
  maximized = gtk_window_is_maximized (parent);

  /* The "Should we use mobile view™" logic
   * Basically: When tall & narrow (but possibly desktop) or
   *            when short & long (but only mobile)
   * Of course we are assuming being short & long &
   * maximised only happens on mobile
   */
  is_small = ((             width <= SNAP_POINT_A && height <= SNAP_POINT_B) ||
              (maximized && width <= SNAP_POINT_B && height <= SNAP_POINT_A));

  /* When we are below the snap point */
  if (is_small) {
    /* When no size is cached, cache the current size */
    if (!priv->old_width || !priv->old_height) {
      gtk_window_get_size (GTK_WINDOW (self), &priv->old_width, &priv->old_height);
      update_titlebar (self, is_small);
    }
    /* Resize the dialog to match the parent */
    gtk_window_resize (GTK_WINDOW (self), width, height);
  } else if (priv->old_width || priv->old_height) {
    /* Restore the cached size */
    gtk_window_resize (GTK_WINDOW (self), priv->old_width, priv->old_height);
    update_titlebar (self, is_small);
    /* Clear cached size */
    priv->old_width = 0;
    priv->old_height = 0;
  }
}

static void
hdy_dialog_realize (GtkWidget *widget)
{
  HdyDialog        *self = HDY_DIALOG (widget);
  HdyDialogPrivate *priv = hdy_dialog_get_instance_private (self);
  GtkWidget        *titlebar;

  titlebar = gtk_window_get_titlebar (GTK_WINDOW (self));

  /* If no titlebar was set, add a headerbar */
  if (!titlebar) {
    titlebar = gtk_header_bar_new ();
    gtk_header_bar_set_show_close_button (GTK_HEADER_BAR (titlebar), TRUE);
    gtk_header_bar_set_title (GTK_HEADER_BAR (titlebar),
                              gtk_window_get_title (GTK_WINDOW (self)));
    gtk_widget_show (titlebar);
    gtk_window_set_titlebar (GTK_WINDOW (self), titlebar);
  }

  /* If the titlebar is a headerbar add the back button to it */
  if (GTK_IS_HEADER_BAR (titlebar)) {
    priv->no_actions = gtk_header_bar_get_show_close_button (GTK_HEADER_BAR (titlebar));
    if (priv->no_actions) {
      gtk_header_bar_pack_start (GTK_HEADER_BAR (titlebar), priv->closebtn);
    }
  }

  handle_size (self, gtk_window_get_transient_for (GTK_WINDOW (self)));

  GTK_WIDGET_CLASS (hdy_dialog_parent_class)->realize (widget);
}

static void
parent_freed_cb (gpointer data,
                 GObject *where_the_object_was)
{
  HdyDialog        *self = HDY_DIALOG (data);
  HdyDialogPrivate *priv = hdy_dialog_get_instance_private (self);

  priv->parent = NULL;
}

static void
hdy_dialog_finalize (GObject *object)
{
  HdyDialog        *self = HDY_DIALOG (object);
  HdyDialogPrivate *priv = hdy_dialog_get_instance_private (self);

  /* If we had a parent disconnect from it */
  if (priv->parent) {
    g_signal_handler_disconnect (G_OBJECT (priv->parent), priv->size_handler);
    g_object_weak_unref (G_OBJECT (priv->parent), parent_freed_cb, self);
  }

  G_OBJECT_CLASS (hdy_dialog_parent_class)->finalize (object);
}

/* <= 3.24.1 never actually emits notify::transient-for so 
 * we have this hacky workaround */
#if !GTK_CHECK_VERSION(3, 24, 2)

enum {
  PROP_0,
  /* Wrap the property on GtkWindow */
  PROP_TRANSIENT_FOR,
  LAST_PROP = PROP_TRANSIENT_FOR,
};

static void
hdy_dialog_get_property (GObject    *object,
                         guint       prop_id,
                         GValue     *value,
                         GParamSpec *pspec)
{
  HdyDialog *self = HDY_DIALOG (object);

  switch (prop_id) {
  case PROP_TRANSIENT_FOR:
    g_value_set_object (value, gtk_window_get_transient_for (GTK_WINDOW (self)));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
hdy_dialog_set_property (GObject      *object,
                         guint         prop_id,
                         const GValue *value,
                         GParamSpec   *pspec)
{
  HdyDialog *self = HDY_DIALOG (object);

  switch (prop_id) {
  case PROP_TRANSIENT_FOR:
    gtk_window_set_transient_for (GTK_WINDOW (self), g_value_get_object (value));
    g_object_notify (G_OBJECT (self), "transient-for");
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
hdy_dialog_class_init (HdyDialogClass *klass)
{
  GObjectClass   *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->get_property = hdy_dialog_get_property;
  object_class->set_property = hdy_dialog_set_property;
  object_class->finalize = hdy_dialog_finalize;

  widget_class->realize = hdy_dialog_realize;

  g_object_class_override_property (object_class,
                                    PROP_TRANSIENT_FOR,
                                    "transient-for");
}

#else

static void
hdy_dialog_class_init (HdyDialogClass *klass)
{
  GObjectClass   *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->finalize = hdy_dialog_finalize;

  widget_class->realize = hdy_dialog_realize;
}

#endif

/* Handle GtkWidget::size-allocate on (HdyDialog) GtkWindow:transient-for */
static void
size_cb (GtkWidget    *widget,
         GdkRectangle *allocation,
         gpointer      user_data)
{
  HdyDialog *self = HDY_DIALOG (user_data);

  handle_size (self, GTK_WINDOW (widget));
}

/* Handle (HdyDialog) GObject::notify::transient-for */
static void
transient_cb (GObject    *object,
              GParamSpec *pspec,
              gpointer    user_data)
{
  HdyDialog        *self = HDY_DIALOG (object);
  HdyDialogPrivate *priv = hdy_dialog_get_instance_private (self);

  /* If we are being reparented disconnect from the old one */
  if (priv->parent) {
    g_signal_handler_disconnect (G_OBJECT (priv->parent), priv->size_handler);
    g_object_weak_unref (G_OBJECT (priv->parent), parent_freed_cb, self);
  }

  /* Get the dialogs new parent */
  priv->parent = gtk_window_get_transient_for (GTK_WINDOW (self));

  /* Check we actually have a parent */
  if (priv->parent) {
    /* Listen for the parent resizing */
    priv->size_handler = g_signal_connect (G_OBJECT (priv->parent),
                                           "size-allocate",
                                           G_CALLBACK (size_cb),
                                           self);
    gtk_widget_queue_allocate (GTK_WIDGET (priv->parent));
    g_object_weak_ref (G_OBJECT (priv->parent), parent_freed_cb, self);
  }
}

/* Handle GtkButton::clicked on our custom back button */
static void
back_clicked_cb (GtkButton *back,
                 gpointer   user_data)
{
  HdyDialog *self = HDY_DIALOG (user_data);

  g_signal_emit_by_name (self, "close", NULL);
}

static void
hdy_dialog_init (HdyDialog *self)
{
  HdyDialogPrivate *priv = hdy_dialog_get_instance_private (self);

  /* Set the inital values of our private data */
  priv->parent = NULL;
  priv->size_handler = 0;

  priv->old_width = 0;
  priv->old_height = 0;

  priv->no_actions = TRUE;

  /* Prepare the back button for the mobile view */
  priv->closebtn = gtk_button_new_from_icon_name ("go-previous-symbolic",
                                                  GTK_ICON_SIZE_BUTTON);
  gtk_widget_hide (priv->closebtn);
  g_signal_connect (G_OBJECT (priv->closebtn), "clicked",
                    G_CALLBACK (back_clicked_cb), self);

  /* Listen to changes in our parent */
  g_signal_connect (G_OBJECT (self), "notify::transient-for",
                    G_CALLBACK (transient_cb), NULL);

  /* Change some properties default values */
  g_object_set (G_OBJECT (self),
                "modal", TRUE,
                "destroy-with-parent", TRUE,
                NULL);
}

/**
 * hdy_dialog_new:
 * @parent: #GtkWindow this dialog is a child of
 *
 * Create a #HdyDialog with #GtkWindow:transient-for set to parent
 *
 * C Usage
 * |[<!-- language="C" -->
 * GtkWidget *dlg = hdy_dialog_new (GTK_WINDOW (main_window));
 * ]|
 * 
 * Vala Usage
 * |[<!-- language="Vala" -->
 * var dlg = new Hdy.Dialog (main_window);
 * ]|
 * 
 * Python Usage
 * |[<!-- language="Python" -->
 * dlg = Handy.Dialog.new (main_window);
 * ]|
 *
 * Since: 0.0.7
 */
GtkWidget *
hdy_dialog_new (GtkWindow *parent)
{
  return g_object_new (HDY_TYPE_DIALOG,
                       "use-header-bar", TRUE,
                       "transient-for", parent,
                       NULL);
}
