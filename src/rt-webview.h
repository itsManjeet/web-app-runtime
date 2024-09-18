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

#ifndef RT_WEBVIEW_H
#define RT_WEBVIEW_H

#include <webkit2/webkit2.h>

G_BEGIN_DECLS

#define RT_TYPE_WEBVIEW (rt_webview_get_type ())

G_DECLARE_FINAL_TYPE (RtWebView, rt_webview, RT, WEBVIEW, WebKitWebView)

GtkWidget *
rt_webview_new (void);

G_END_DECLS

#endif // RT_WEBVIEW_H
