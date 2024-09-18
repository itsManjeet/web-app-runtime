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

#include "rt-window.h"
#include "rt-webview.h"

struct _RtWindow
{
  GtkApplicationWindow parent;
  RtWebView *webview;
  gchar *uri;
};

G_DEFINE_TYPE (RtWindow, rt_window, GTK_TYPE_APPLICATION_WINDOW)

enum
{
  PROP_URI = 1,
  N_PROPS,
};

static GParamSpec *properties[N_PROPS] = { NULL };

static void
rt_window_set_property (GObject *object,
                        guint property_id,
                        const GValue *value,
                        GParamSpec *pspec)
{
  RtWindow *self = RT_WINDOW (object);
  switch (property_id)
    {
    case PROP_URI:
      self->uri = g_value_dup_string (value);
      webkit_web_view_load_uri (WEBKIT_WEB_VIEW (self->webview), self->uri);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
rt_window_constructed (GObject *object)
{
  RtWindow *self = RT_WINDOW (object);

  G_OBJECT_CLASS (rt_window_parent_class)->constructed (object);
}

static void
rt_window_class_init (RtWindowClass *klass)
{
  G_OBJECT_CLASS (klass)->set_property = rt_window_set_property;
  G_OBJECT_CLASS (klass)->constructed = rt_window_constructed;

  properties[PROP_URI] = g_param_spec_string ("uri",
                                              "Uri",
                                              "Web View URI",
                                              "",
                                              G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);
  g_object_class_install_properties (G_OBJECT_CLASS (klass), N_PROPS, properties);
}


static void
rt_window_init (RtWindow *self)
{
  self->webview = RT_WEBVIEW (rt_webview_new ());
  gtk_container_add (GTK_CONTAINER (self), GTK_WIDGET (self->webview));
  gtk_widget_show (GTK_WIDGET (self->webview));
}

GtkWidget *
rt_window_new (GtkApplication *app, const gchar *uri)
{
  g_assert (GTK_IS_APPLICATION (app));
  return g_object_new (RT_TYPE_WINDOW,
                       "application", app,
                       "uri", uri, NULL);
}