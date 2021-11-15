/*
 * Copyright (C) 2021 Manuel Genovés <manuel.genoves@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "config.h"

#include "adw-animation-target-private.h"
#include "adw-macros-private.h"

struct _AdwAnimationTarget
{
  GObject parent_instance;

  AdwAnimationTargetFunc callback;
  gpointer user_data;
};

struct _AdwAnimationTargetClass
{
  GObjectClass parent_class;

  void (*set_value) (AdwAnimationTarget *self,
                     double              value);
};

G_DEFINE_FINAL_TYPE (AdwAnimationTarget, adw_animation_target, G_TYPE_OBJECT)

static void
adw_animation_target_class_init (AdwAnimationTargetClass *klass)
{
}

static void
adw_animation_target_init (AdwAnimationTarget *self)
{
}

AdwAnimationTarget *
adw_animation_target_new (AdwAnimationTargetFunc callback,
                          gpointer               user_data)
{
  AdwAnimationTarget *self;

  g_return_val_if_fail (callback != NULL, NULL);

  self = g_object_new (ADW_TYPE_ANIMATION_TARGET, NULL);

  self->callback = callback;
  self->user_data = user_data;

  return self;
}

void
adw_animation_target_set_value (AdwAnimationTarget *self,
                                double              value)
{
  g_return_if_fail (ADW_IS_ANIMATION_TARGET (self));

  self->callback (value, self->user_data);
}
