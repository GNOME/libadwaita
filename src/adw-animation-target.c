/*
 * Copyright (C) 2021 Manuel Genovés <manuel.genoves@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "config.h"

#include "adw-animation-target-private.h"

struct _AdwAnimationTarget
{
  GObject parent_instance;
};

struct _AdwAnimationTargetClass
{
  GObjectClass parent_class;

  void (*set_value) (AdwAnimationTarget *self,
                     double              value);
};

G_DEFINE_ABSTRACT_TYPE (AdwAnimationTarget, adw_animation_target, G_TYPE_OBJECT)

static void
adw_animation_target_class_init (AdwAnimationTargetClass *klass)
{
}

static void
adw_animation_target_init (AdwAnimationTarget *self)
{
}

void
adw_animation_target_set_value (AdwAnimationTarget *self,
                                double              value)
{
  g_return_if_fail (ADW_IS_ANIMATION_TARGET (self));

  ADW_ANIMATION_TARGET_GET_CLASS (self)->set_value (self, value);
}

struct _AdwCallbackAnimationTarget
{
  AdwAnimationTarget parent_instance;

  AdwAnimationTargetFunc callback;
  gpointer user_data;
};

struct _AdwCallbackAnimationTargetClass
{
  AdwAnimationTargetClass parent_class;
};

G_DEFINE_TYPE (AdwCallbackAnimationTarget, adw_callback_animation_target, ADW_TYPE_ANIMATION_TARGET)

static void
adw_callback_animation_target_set_value (AdwAnimationTarget *target,
                                         double              value)
{
  AdwCallbackAnimationTarget *self = ADW_CALLBACK_ANIMATION_TARGET (target);

  self->callback (value, self->user_data);
}

static void
adw_callback_animation_target_class_init (AdwCallbackAnimationTargetClass *klass)
{
  AdwAnimationTargetClass *target_class = ADW_ANIMATION_TARGET_CLASS (klass);

  target_class->set_value = adw_callback_animation_target_set_value;
}

static void
adw_callback_animation_target_init (AdwCallbackAnimationTarget *self)
{
}

AdwAnimationTarget *
adw_callback_animation_target_new (AdwAnimationTargetFunc callback,
                                   gpointer               user_data)
{
  AdwCallbackAnimationTarget *self;

  g_return_val_if_fail (callback != NULL, NULL);

  self = g_object_new (ADW_TYPE_CALLBACK_ANIMATION_TARGET, NULL);

  self->callback = callback;
  self->user_data = user_data;

  return ADW_ANIMATION_TARGET (self);
}
