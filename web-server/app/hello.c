/* Monkey HTTP Daemon
 * ------------------
 * Copyright (C) 2012, Lauri Kasanen <cand@gmx.com>
 *  Modified 2021, Axis Communications AB, Lund, Sweden
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Library General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include <libmonkey.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <syslog.h>
#include <glib.h>

/*
 * This example shows how to start the server, and point it to /tmp.
 * It also creates /tmp/index.html.
*/

static void write_index() {

	const char *path = "/tmp/index.html";

	FILE *f = fopen(path, "w");
	if (!f) exit(1);

	fprintf(f, "<html><body><h2>Hello Monkey</h2></body></html>");

	fclose(f);
}

int main(int argc, char* argv[]) {

  	GMainLoop *loop;
  	int ret;

	openlog("hello", LOG_PID|LOG_CONS, LOG_USER);
    syslog(LOG_INFO, "starting  %s", argv[0]);
  	loop = g_main_loop_new(NULL, FALSE);
  	write_index();

	// All defaults. Bind to all interfaces, port 2001, default plugins, /tmp.
	// No callbacks are used.
	mklib_ctx ctx = mklib_init(NULL, 0, 0, "/tmp");
	if (!ctx) {
      syslog(LOG_INFO, "mklib_init failed");
      exit(0);
	}
    syslog(LOG_INFO, "mklib_init success");

	// The default has no index files, let's set index.html as one.
	ret = mklib_config(ctx, MKC_INDEXFILE, "index.html", NULL);
	if (!ret) {
      syslog(LOG_INFO, "mklib_config failed");
      exit(0);
	}
    syslog(LOG_INFO, "mklib_config success");

	// Start the server.
	mklib_start(ctx);
    syslog(LOG_INFO, "mklib_start success");

  	g_main_loop_run(loop);

	mklib_stop(ctx);

    syslog(LOG_INFO, "Stop Monkey hello");
	return 0;
}
