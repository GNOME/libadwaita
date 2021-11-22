/*
 * Copyright (C) 2021 Manuel Genovés <manuel.genoves@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "config.h"

#include "adw-easing.h"

#include "adw-animation-util-private.h"

/**
 * AdwEasing:
 * @ADW_EASING_EASE_IN_CUBIC: Starts slowly and accelerates, cubic curve.
 * @ADW_EASING_EASE_OUT_CUBIC: Starts quickly and decelerates, cubic curve.
 *   This is an inverse of `ADW_EASING_EASE_IN_CUBIC`.
 * @ADW_EASING_EASE_IN_OUT_CUBIC: Starts slowly, accelerates, decelerates and
 *   ends slowly again, combines `ADW_EASING_EASE_IN_CUBIC` and
 *   `ADW_EASING_EASE_OUT_CUBIC`.
 *
 * Describes the available easing functions for use with
 * [class@Adw.TimedAnimation].
 *
 * New values may be added to this enumeration over time.
 *
 * Since: 1.0
 */

/**
 * adw_easing_ease:
 * @self: a `AdwEasing`
 * @value: a value to ease
 *
 * Computes easing with @easing for @value.
 *
 * @value should generally be in the [0, 1] range.
 *
 * Returns: the easing for @value
 *
 * Since: 1.0
 */
double
adw_easing_ease (AdwEasing self,
                 double    value)
{
  switch (self) {
    case ADW_EASING_EASE_IN_CUBIC:
      return adw_ease_in_cubic (value);
    case ADW_EASING_EASE_OUT_CUBIC:
      return adw_ease_out_cubic (value);
    case ADW_EASING_EASE_IN_OUT_CUBIC:
      return adw_ease_in_out_cubic (value);
    default:
      g_assert_not_reached ();
  }
}
