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

#include "rt-webview.h"

#define APP_SCHEME "app"

#define GLOBAL_CSS_FILE "/usr/lib/runtime/style.css"
#define GLOBAL_SCRIPT_FILE "/usr/lib/runtime/script.js"

struct _RtWebView
{
  WebKitWebView parent;
  WebKitUserContentManager *content_manager;
  GFileMonitor *monitor;
};

G_DEFINE_TYPE (RtWebView, rt_webview, WEBKIT_TYPE_WEB_VIEW)

gchar *
get_local_app_path (const gchar *id, const gchar *path)
{
  gchar *app_path = NULL;
  GPathBuf *path_buf = NULL;

  const gchar *application_dirs_env = g_getenv ("RLXOS_APPLICATION_DIRS");
  if (application_dirs_env == NULL)
    application_dirs_env = "/apps";

  gchar **application_dirs = g_strsplit (application_dirs_env, ":", -1);
  for (gchar **i = application_dirs; *i; i++)
    {
      path_buf = g_path_buf_new ();
      g_path_buf_push (path_buf, *i);
      g_path_buf_push (path_buf, id);
      if (path && strlen (path) > 0)
        g_path_buf_push (path_buf, path);

      app_path = g_path_buf_to_path (path_buf);
      if (g_file_test (app_path, G_FILE_TEST_IS_REGULAR))
        {
          g_path_buf_free (path_buf);
          path_buf = NULL;
          break;
        }

      free (app_path);

      g_path_buf_push (path_buf, "index.html");
      app_path = g_path_buf_to_path (path_buf);
      if (g_file_test (app_path, G_FILE_TEST_IS_REGULAR))
        {
          g_path_buf_free (path_buf);
          path_buf = NULL;
          break;
        }

      free (app_path);
      app_path = NULL;
    }

  g_strfreev (application_dirs);

  return app_path;
}

static void
load_css_if_exists (WebKitUserContentManager *content_manager, const gchar *filepath)
{
  g_autofree gchar *contents = NULL;
  g_autoptr (GError) error = NULL;
  g_autoptr (WebKitUserStyleSheet) style_sheet = NULL;
  gsize length = 0;

  if (!g_file_test (filepath, G_FILE_TEST_IS_REGULAR))
    return;

  g_file_get_contents (filepath, &contents, &length, &error);
  if (error)
    {
      g_warning ("Couldn't open CSS file: %s", error->message);
      return;
    }

  style_sheet = webkit_user_style_sheet_new (
    contents,
    WEBKIT_USER_CONTENT_INJECT_ALL_FRAMES,
    WEBKIT_USER_STYLE_LEVEL_USER,
    NULL,
    NULL);

  g_message ("loading css file %s", filepath);
  webkit_user_content_manager_add_style_sheet (content_manager, style_sheet);
}

static void
load_script_if_exists (WebKitUserContentManager *content_manager, const gchar *filepath)
{
  g_autofree gchar *contents = NULL;
  g_autoptr (GError) error = NULL;
  g_autoptr (WebKitUserScript) script = NULL;
  gsize length = 0;

  if (!g_file_test (filepath, G_FILE_TEST_IS_REGULAR))
    return;

  g_file_get_contents (filepath, &contents, &length, &error);
  if (error)
    {
      g_warning ("Couldn't open script file: %s", error->message);
      return;
    }

  script = webkit_user_script_new (
    contents,
    WEBKIT_USER_CONTENT_INJECT_ALL_FRAMES,
    WEBKIT_USER_STYLE_LEVEL_USER,
    NULL,
    NULL);

  g_message ("loading script file %s", filepath);
  webkit_user_content_manager_add_script (content_manager, script);
}

static void
on_local_file_changed (GFileMonitor *self, GFile *file, GFile *other_file, GFileMonitorEvent event, gpointer user_data)
{
  RtWebView *webview = user_data;
  webkit_web_view_reload (WEBKIT_WEB_VIEW (webview));
}


static void
rt_webview_app_protocol (WebKitURISchemeRequest *request, gpointer user_data)
{
  RtWebView *webview = user_data;
  g_autoptr (GError) error = NULL;
  g_autoptr (GFile) file = NULL;
  g_autoptr (GFileInputStream) stream = NULL;
  const gchar *uri = NULL;
  g_autofree gchar *app_id = NULL;
  g_autofree gchar *app_filepath = NULL;

  uri = webkit_uri_scheme_request_get_uri (request);
  if (!g_str_has_prefix (uri, APP_SCHEME "://"))
    {
      webkit_uri_scheme_request_finish_error (request, g_error_new (G_URI_ERROR, G_URI_ERROR_BAD_SCHEME, "expected app:// uri prefix"));
      return;
    }

  const gchar *path = webkit_uri_scheme_request_get_path (request);
  guint uri_len = strlen (uri);
  guint scheme_len = strlen (APP_SCHEME "://");
  guint path_len = strlen (path);

  app_id = g_utf8_substring (uri, scheme_len, uri_len - path_len);
  app_filepath = get_local_app_path (app_id, path);
  if (app_filepath == NULL)
    {
      webkit_uri_scheme_request_finish_error (request, g_error_new (G_URI_ERROR, G_URI_ERROR_FAILED, "path not exists"));
      return;
    }


  file = g_file_new_for_path (app_filepath);
  stream = g_file_read (file, NULL, &error);
  if (error)
    {
      webkit_uri_scheme_request_finish_error (request, error);
      return;
    }

  webview->monitor = g_file_monitor_file (file, G_FILE_MONITOR_EVENT_CHANGED, NULL, NULL);
  g_signal_connect (G_OBJECT (webview->monitor), "changed", G_CALLBACK (on_local_file_changed), webview);
  webkit_uri_scheme_request_finish (request, G_INPUT_STREAM (stream), -1, "text/html");
}

static void
rt_webview_finalize (GObject *object)
{
  RtWebView *webview = RT_WEBVIEW (object);
  if (webview->monitor)
    {
      g_file_monitor_cancel (webview->monitor);
      g_object_unref (webview->monitor);
    }

  G_OBJECT_CLASS (rt_webview_parent_class)->finalize (object);
}

static void
rt_webview_constructed (GObject *object)
{
  RtWebView *webview = RT_WEBVIEW (object);

  WebKitUserContentManager *content_manager = webkit_web_view_get_user_content_manager (WEBKIT_WEB_VIEW (webview));

  load_css_if_exists (content_manager, GLOBAL_CSS_FILE);
  load_script_if_exists (content_manager, GLOBAL_SCRIPT_FILE);

  {
    GPathBuf *path_buf = g_path_buf_new ();
    g_path_buf_push (path_buf, getenv ("HOME"));
    g_path_buf_push (path_buf, ".style.css");
    gchar *user_css_path = g_path_buf_to_path (path_buf);
    g_path_buf_free (path_buf);

    load_css_if_exists (content_manager, user_css_path);
    g_free (user_css_path);
  }

  {
    GPathBuf *path_buf = g_path_buf_new ();
    g_path_buf_push (path_buf, getenv ("HOME"));
    g_path_buf_push (path_buf, ".script.js");
    gchar *user_css_path = g_path_buf_to_path (path_buf);
    g_path_buf_free (path_buf);

    load_script_if_exists (content_manager, user_css_path);
    g_free (user_css_path);
  }
  G_OBJECT_CLASS (rt_webview_parent_class)->constructed (object);
}

static void
rt_webview_class_init (RtWebViewClass *klass)
{
  G_OBJECT_CLASS (klass)->constructed = rt_webview_constructed;
  G_OBJECT_CLASS (klass)->finalize = rt_webview_finalize;
}

static void
rt_webview_init (RtWebView *webview)
{
  WebKitWebContext *context = webkit_web_context_get_default ();

  webkit_web_context_register_uri_scheme (
    context, APP_SCHEME,
    rt_webview_app_protocol, webview, NULL);
}


GtkWidget *
rt_webview_new (void)
{
  WebKitUserContentManager *content_manager = webkit_user_content_manager_new ();
  return g_object_new (RT_TYPE_WEBVIEW,
                       "user-content-manager", content_manager,
                       NULL);
}