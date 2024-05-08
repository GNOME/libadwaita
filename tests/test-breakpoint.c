/*
 * Copyright (C) 2023 Purism SPC
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Author: Alice Mikhaylenko <alice.mikhaylenko@puri.sm>
 */

#include <adwaita.h>

static inline void
check_to_string (AdwBreakpointCondition *condition,
                 const char             *expected)
{
  char *str = adw_breakpoint_condition_to_string (condition);

  g_assert_cmpstr (str, ==, expected);

  g_free (str);
  adw_breakpoint_condition_free (condition);
}

static void
test_adw_breakpoint_condition_to_string (void)
{
  AdwBreakpointCondition *condition_1, *condition_2, *condition_3;

  check_to_string (adw_breakpoint_condition_new_length (ADW_BREAKPOINT_CONDITION_MAX_WIDTH,
                                                        400,
                                                        ADW_LENGTH_UNIT_PX),
                   "max-width: 400px");

  check_to_string (adw_breakpoint_condition_new_length (ADW_BREAKPOINT_CONDITION_MIN_HEIGHT,
                                                        200,
                                                        ADW_LENGTH_UNIT_PT),
                   "min-height: 200pt");

  check_to_string (adw_breakpoint_condition_new_ratio (ADW_BREAKPOINT_CONDITION_MIN_ASPECT_RATIO,
                                                       4, 3),
                   "min-aspect-ratio: 4/3");

  check_to_string (adw_breakpoint_condition_new_ratio (ADW_BREAKPOINT_CONDITION_MIN_ASPECT_RATIO,
                                                       2, 1),
                   "min-aspect-ratio: 2");
  check_to_string (adw_breakpoint_condition_new_ratio (ADW_BREAKPOINT_CONDITION_MIN_ASPECT_RATIO,
                                                       0, 2),
                   "min-aspect-ratio: 0");

  condition_1 = adw_breakpoint_condition_new_length (ADW_BREAKPOINT_CONDITION_MAX_WIDTH,
                                                     400,
                                                     ADW_LENGTH_UNIT_PX);
  condition_2 = adw_breakpoint_condition_new_ratio (ADW_BREAKPOINT_CONDITION_MIN_ASPECT_RATIO,
                                                    4, 3);
  condition_3 = adw_breakpoint_condition_new_ratio (ADW_BREAKPOINT_CONDITION_MAX_ASPECT_RATIO,
                                                    2, 1);

  check_to_string (adw_breakpoint_condition_new_and (adw_breakpoint_condition_copy (condition_1),
                                                     adw_breakpoint_condition_copy (condition_2)),
                   "max-width: 400px and min-aspect-ratio: 4/3");

  check_to_string (adw_breakpoint_condition_new_or (adw_breakpoint_condition_copy (condition_1),
                                                    adw_breakpoint_condition_copy (condition_2)),
                   "max-width: 400px or min-aspect-ratio: 4/3");

  check_to_string (adw_breakpoint_condition_new_and (adw_breakpoint_condition_copy (condition_1),
                                                     adw_breakpoint_condition_new_and (adw_breakpoint_condition_copy (condition_2),
                                                                                       adw_breakpoint_condition_copy (condition_3))),
                   "max-width: 400px and min-aspect-ratio: 4/3 and max-aspect-ratio: 2");

  check_to_string (adw_breakpoint_condition_new_and (adw_breakpoint_condition_copy (condition_1),
                                                     adw_breakpoint_condition_new_or (adw_breakpoint_condition_copy (condition_2),
                                                                                       adw_breakpoint_condition_copy (condition_3))),
                   "max-width: 400px and (min-aspect-ratio: 4/3 or max-aspect-ratio: 2)");

  check_to_string (adw_breakpoint_condition_new_or (adw_breakpoint_condition_new_and (adw_breakpoint_condition_copy (condition_1),
                                                                                      adw_breakpoint_condition_copy (condition_2)),
                                                    adw_breakpoint_condition_copy (condition_3)),
                   "(max-width: 400px and min-aspect-ratio: 4/3) or max-aspect-ratio: 2");

  adw_breakpoint_condition_free (condition_1);
  adw_breakpoint_condition_free (condition_2);
  adw_breakpoint_condition_free (condition_3);
}

static struct parse_test {
  const char *input;
  const char *expected;
} parse_tests[] = {
  { "", NULL },
  { "()", NULL },
  { "foo", NULL },

  /* Length */
  { "max-width: 400px",   "max-width: 400px" },
  { "max-width: 400",     "max-width: 400px" },
  { "max-width: 400pt",   "max-width: 400pt" },
  { "max-width:400pt",    "max-width: 400pt" },
  { "max-width: 400.0px", "max-width: 400px" },
  { "max-width: 400.5px", "max-width: 400.5px" },
  { "      max-width        :        400     pt       ", "max-width: 400pt" },

  { "max-width:",        NULL },
  { "max-width: px",     NULL },
  { "max-length: 400px", NULL },
  { "max-width 400px",   NULL },
  { "max-width: -1px",   NULL },
  { "max-width: 400p",   NULL },
  { "max-width: 400px;", NULL },

  /* Ratio */
  { "max-aspect-ratio: 4/3", "max-aspect-ratio: 4/3" },
  { "max-aspect-ratio: 2",   "max-aspect-ratio: 2" },
  { "max-aspect-ratio: 2/1", "max-aspect-ratio: 2" },
  { "max-aspect-ratio: 0/3", "max-aspect-ratio: 0" },
  { "max-aspect-ratio:4/3",  "max-aspect-ratio: 4/3" },
  { "       max-aspect-ratio   :         4/3       ", "max-aspect-ratio: 4/3" },

  { "max-aspect-ratio:",       NULL },
  { "max-aspect-ratio: 4/3px", NULL },
  { "max-aspect-ratio: 4px",   NULL },
  { "max-aspect-ratio: -4",    NULL },
  { "max-aspect-ratio: -4/3",  NULL },
  { "max-aspect-ratio: 4/0",   NULL },
  { "max-aspect-ratio: 4/3;",  NULL },

  /* Single + parentheses */
  { "(max-width: 100px)",     "max-width: 100px" },
  { "(((max-width: 100px)))", "max-width: 100px" },
  { "   (   max-width   :   100px   )   ", "max-width: 100px" },

  { "(max-width: 100px",   NULL },
  { "(max-width: 100px(",  NULL },
  { "(max-width): 100px",  NULL },
  { "(max-width: 100px))", NULL },

  /* Multi */
  { "max-width: 100px and max-height: 200px", "max-width: 100px and max-height: 200px" },
  { "max-width: 100px or max-height: 200px",  "max-width: 100px or max-height: 200px" },
  { "   max-width   :   100px   or   max-height   :   200px   ", "max-width: 100px or max-height: 200px" },

  { "(max-width: 100px) and max-height: 200px",   "max-width: 100px and max-height: 200px" },
  { "max-width: 100px and (max-height: 200px)",   "max-width: 100px and max-height: 200px" },
  { "(max-width: 100px) and (max-height: 200px)", "max-width: 100px and max-height: 200px" },
  { "(max-width: 100px and max-height: 200px)",   "max-width: 100px and max-height: 200px" },

  { "(max-width: 100px and max-height: 200px) or max-aspect-ratio: 3/2",
               "(max-width: 100px and max-height: 200px) or max-aspect-ratio: 3/2" },
  { "max-width: 100px and (max-height: 200px or max-aspect-ratio: 3/2)",
               "max-width: 100px and (max-height: 200px or max-aspect-ratio: 3/2)" },

  { "(max-width: 100px and max-height: 200px) or max-aspect-ratio: 3/2",
               "(max-width: 100px and max-height: 200px) or max-aspect-ratio: 3/2" },
  { "max-width: 100px and (max-height: 200px or max-aspect-ratio: 3/2)",
               "max-width: 100px and (max-height: 200px or max-aspect-ratio: 3/2)" },

  { "max-width: 100px and max-height: 200px and max-aspect-ratio: 3/2 and min-aspect-ratio: 1/2",
               "max-width: 100px and max-height: 200px and max-aspect-ratio: 3/2 and min-aspect-ratio: 1/2" },
  { "max-width: 100px or max-height: 200px or max-aspect-ratio: 3/2 or min-aspect-ratio: 1/2",
               "max-width: 100px or max-height: 200px or max-aspect-ratio: 3/2 or min-aspect-ratio: 1/2" },

  { "max-width: 100px and max-height: 200px or max-aspect-ratio: 3/2 and min-aspect-ratio: 1/2",
               "((max-width: 100px and max-height: 200px) or max-aspect-ratio: 3/2) and min-aspect-ratio: 1/2" },

  { "max-width: 100pxor max-height: 200px", NULL },
  { "max-width: 100px ormax-height: 200px", NULL },
  { "max-width: 100px max-height: 200px",   NULL },
  { "max-width: 100px or max-height",       NULL },
  { "max-width: 100px o",                   NULL },
  { "max-width: 100px or ()",               NULL },
  { "() or max-height: 200px",              NULL },
};

static void
test_adw_breakpoint_condition_parse (gconstpointer user_data)
{
  const struct parse_test *test = user_data;

  if (g_test_subprocess ()) {
    AdwBreakpointCondition *condition;

    condition = adw_breakpoint_condition_parse (test->input);

    if (test->expected) {
      g_assert_nonnull (condition);

      check_to_string (condition, test->expected);
    } else {
      if (condition != NULL) {
        char *str = adw_breakpoint_condition_to_string (condition);

        g_test_message ("'%s' is invalid, but was parsed as '%s'", test->input, str);
        g_free (str);
      }

      g_assert_null (condition);
    }

    return;
  }

  g_test_trap_subprocess (NULL, 0, 0);

  if (test->expected) {
    g_test_trap_assert_passed ();
  } else {
    g_test_trap_assert_failed ();
    g_test_trap_assert_stderr ("*Unable to parse condition*");
  }
}

int
main (int   argc,
      char *argv[])
{
  gtk_test_init (&argc, &argv, NULL);
  adw_init ();

  g_test_add_func ("/Adwaita/BreakpointCondition/to_string", test_adw_breakpoint_condition_to_string);

  for (size_t i = 0; i < G_N_ELEMENTS (parse_tests); i++) {
    char *path = g_strdup_printf ("/Adwaita/BreakpointCondition/parse_%li", i);
    g_test_add_data_func (path, &parse_tests[i], test_adw_breakpoint_condition_parse);
    g_free (path);
  }

  return g_test_run ();
}
