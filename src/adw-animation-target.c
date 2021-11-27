/*
 * Copyright (C) 2021 Manuel Genovés <manuel.genoves@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "config.h"

#include "adw-animation-target-private.h"

#include "adw-macros-private.h"

/**
 * AdwAnimationTarget:
 *
 * Represents a value [class@Adw.Animation] can animate.
 *
 * Currently the only implementation is [class@Adw.CallbackAnimationTarget].
 *
 * Since: 1.0
 */

/**
 * AdwCallbackAnimationTarget:
 *
 * An [class@Adw.AnimationTarget] that calls a given callback during the
 * animation.
 *
 * Since: 1.0
 */

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
  GDestroyNotify destroy_notify;
};

struct _AdwCallbackAnimationTargetClass
{
  AdwAnimationTargetClass parent_class;
};

G_DEFINE_FINAL_TYPE (AdwCallbackAnimationTarget, adw_callback_animation_target, ADW_TYPE_ANIMATION_TARGET)

static void
adw_callback_animation_target_set_value (AdwAnimationTarget *target,
                                         double              value)
{
  AdwCallbackAnimationTarget *self = ADW_CALLBACK_ANIMATION_TARGET (target);

  self->callback (value, self->user_data);
}

static void
adw_callback_animation_finalize (GObject *object)
{
  AdwCallbackAnimationTarget *self = ADW_CALLBACK_ANIMATION_TARGET (object);

  if (self->destroy_notify)
    self->destroy_notify (self->user_data);

  G_OBJECT_CLASS (adw_callback_animation_target_parent_class)->finalize (object);
}

static void
adw_callback_animation_target_class_init (AdwCallbackAnimationTargetClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  AdwAnimationTargetClass *target_class = ADW_ANIMATION_TARGET_CLASS (klass);

  object_class->finalize = adw_callback_animation_finalize;

  target_class->set_value = adw_callback_animation_target_set_value;
}

static void
adw_callback_animation_target_init (AdwCallbackAnimationTarget *self)
{
}

/**
 * adw_callback_animation_target_new:
 * @callback: (scope notified): the callback to call
 * @user_data: (closure callback): the data to be passed to @callback
 * @destroy: (destroy user_data): the function to be called when the
 *   callback action is finalized
 *
 * Creates a new `AdwAnimationTarget` that calls the given @callback during
 * the animation.
 *
 * Returns: the newly created callback target
 *
 * Since: 1.0
 */
AdwAnimationTarget *
adw_callback_animation_target_new (AdwAnimationTargetFunc callback,
                                   gpointer               user_data,
                                   GDestroyNotify         destroy)
{
  AdwCallbackAnimationTarget *self;

  g_return_val_if_fail (callback != NULL, NULL);

  self = g_object_new (ADW_TYPE_CALLBACK_ANIMATION_TARGET, NULL);

  self->callback = callback;
  self->user_data = user_data;
  self->destroy_notify = destroy;

  return ADW_ANIMATION_TARGET (self);
}
