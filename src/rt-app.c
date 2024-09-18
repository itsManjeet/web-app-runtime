/*
 * Copyright 2024 Manjeet Singh <itsmanjeet1998@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Author(s):
 *   Manjeet Singh <itsmanjeet1998@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "rt-app.h"
#include "rt-window.h"

struct _RtApp
{
  GtkApplication parent;
};

G_DEFINE_TYPE (RtApp, rt_app, GTK_TYPE_APPLICATION)

static gchar *uri = NULL;

static GOptionEntry rt_app_options[] = {
  { "quit", 0, G_OPTION_FLAG_NONE, G_OPTION_ARG_NONE, NULL, "Quit all running instances for this application", NULL },
  { "uri", 0, G_OPTION_FLAG_NONE, G_OPTION_ARG_FILENAME, &uri, "Application uri", "app-uri" },
  { NULL },
};


static int
rt_app_command_line (GApplication *application,
                     GApplicationCommandLine *command_line)
{
  GVariantDict *options;

  options = g_application_command_line_get_options_dict (command_line);

  if (g_variant_dict_contains (options, "quit"))
    {
      g_application_quit (application);
      return 0;
    }

  g_application_activate (application);
  return 0;
}

static void
rt_app_startup (GApplication *application)
{
  G_APPLICATION_CLASS (rt_app_parent_class)->startup (application);
}

static void
rt_app_activate (GApplication *application)
{
  GtkWindow *window = NULL;
  window = GTK_WINDOW (rt_window_new (GTK_APPLICATION (application), uri));
  gtk_window_present (GTK_WINDOW (window));
}

static void
rt_app_class_init (RtAppClass *app_class)
{
  G_APPLICATION_CLASS (app_class)->activate = rt_app_activate;
  G_APPLICATION_CLASS (app_class)->startup = rt_app_startup;
  G_APPLICATION_CLASS (app_class)->command_line = rt_app_command_line;
}

static void
rt_app_init (RtApp *app)
{
  g_application_add_main_option_entries (G_APPLICATION (app), rt_app_options);
}

RtApp *
rt_app_new (void)
{
  return g_object_new (RT_TYPE_APP, "application-id", "dev.rlxos.runtime",
                       "flags", G_APPLICATION_HANDLES_COMMAND_LINE, NULL);
}