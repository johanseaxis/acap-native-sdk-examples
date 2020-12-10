/**
 * Copyright (C) 2020, Axis Communications AB, Lund, Sweden
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * - axparameter -
 *
 * This application is a basic axparameter application, which can
 * add, remove, set and get parameters through axparameter API.
 * The API interface also supports registering callback functions
 * when a parameter value is updated.
 */ 
#include <glib.h>
#include <glib-object.h>
#include <axsdk/axparameter.h>
#include <glib/gprintf.h>
#include <syslog.h>
#include <signal.h>

#define APP_NAME "axparameter"

static GMainLoop *loop = NULL;

struct app_data {
  gchar *parameter_one;
  gchar *parameter_two;
} glob_app_data;

// Checking every 10th second
#define CHECK_PARAMETER_SECS 10

/**
 * brief Callback function registered by g_timeout_add_seconds(),
 *       which is trigged every 10th second and checks application
 *       data
 *
 * param data  User data to be passed to the callback.
 *
 * return True
 */
gboolean
check_application_parameter(void *data)
{
  struct app_data *app_data = data;

  // Callback functions shall not call another ax_parameter method, which will
  // cause a deadlock, instead check the values of parameters stored as application data
  if (app_data->parameter_one != NULL && app_data->parameter_two != NULL) {
    if (!strcmp(app_data->parameter_one, "param_default") == 0 &&
        !strcmp(app_data->parameter_one, "param_one") == 0) {
      // Get the value of the parameter "ParameterTwo" depending on value of "ParameterOne"
      syslog(LOG_INFO, "The value of \"ParameterTwo\" is \"%s\"", app_data->parameter_two);
    } else {
      syslog(LOG_INFO, "The value of \"ParameterOne\" is \"param_default\" or \"param_one\"");
    }
  } else {
    return FALSE;
  }
  return TRUE;
}

/**
 * brief Callback function registered by ax_parameter_register_callback()
 *
 * param name Name of the updated parameter.
 * param value Value of the updated parameter.
 * param data  User data to be passed to the callback.
 */
static void
parameter_callback(const gchar *name, const gchar *value, gpointer data)
{
  struct app_data *app_data = data;
  const gchar *parname = name += strlen("root." APP_NAME ".");

  syslog(LOG_INFO, "In callback, value of \"%s\" is \"%s\"", parname, value);

  // Callback functions shall not call another ax_parameter method, which will
  // cause a deadlock, instead save the value as application data.
  if (strcmp(parname, "ParameterOne") == 0) {
    free(app_data->parameter_one);
    app_data->parameter_one = strdup(value);
  } else if (strcmp(parname, "ParameterTwo") == 0) {
    free(app_data->parameter_two);
    app_data->parameter_two = strdup(value);
  } else {
    syslog(LOG_WARNING, "Parameter is not recognized");
  }
}

/**
 * brief Signals handling
 *
 * param signal_num Signal number.
 */
static void
handle_sigterm(int signal_num)
{
  g_main_loop_quit(loop);
}

/**
 * brief Initialize signals
 *
 * param signo Signal id (not used).
 */
static void
init_signals(void)
{
  struct sigaction sa;

  sa.sa_flags = 0;

  sigemptyset(&sa.sa_mask);
  sa.sa_handler = handle_sigterm;
  sigaction(SIGTERM, &sa, NULL);
  sigaction(SIGINT, &sa, NULL);
}

/**
 * brief Main function.
 *
 * This main function monitors application defined parameters,
 * by using the axparameter API.
 *
 * param argc Number of arguments.
 * param argv Arguments vector.
 *
 * return result
 */
gint
main(gint argc, gchar *argv[])
{
  AXParameter *parameter = NULL;
  GError *error = NULL;
  gchar *value_one = NULL;
  gchar *value_two = NULL;
  GList *list = NULL;
  GList *list_tmp = NULL;

  init_signals();

  openlog(APP_NAME, LOG_PID, LOG_USER);

  loop = g_main_loop_new(NULL, FALSE);

  // New parameter instance is created
  parameter = ax_parameter_new(APP_NAME, &error);
  if (parameter == NULL) {
    goto error_out;
  }

  // Add parameter "ParameterOne"
  if (!ax_parameter_add(parameter, "ParameterOne", "param_default", NULL, &error)) {
    if (error->code == AX_PARAMETER_PARAM_ADDED_ERROR) {
      // Parameter is already added. Nothing to care about
      g_error_free(error);
      error = NULL;
    } else {
      goto error_out;
    }
  }

  // Add parameter "ParameterTwo"
  if (!ax_parameter_add(parameter, "ParameterTwo", "param_two", NULL, &error)) {
    if (error->code == AX_PARAMETER_PARAM_ADDED_ERROR) {
      // parameter is already added. Nothing to care about
      g_error_free(error);
      error = NULL;
    } else {
      goto error_out;
    }
  }

  // Add parameter "ParameterThree"
  if (!ax_parameter_add(parameter, "ParameterThree", "param_three", NULL,
                        &error)) {
    if (error->code == AX_PARAMETER_PARAM_ADDED_ERROR) {
      // Parameter is already added. Nothing to care about
      g_error_free(error);
      error = NULL;
    } else {
      goto error_out;
    }
  }

  // Get the value of the parameter "ParameterOne" and save it locally
  if (!ax_parameter_get(parameter, "ParameterOne", &value_one, &error)) {
    goto error_out;
  }
  syslog(LOG_INFO, "The value of \"ParameterOne\" is \"%s\"", value_one);
  g_free(glob_app_data.parameter_one);
  glob_app_data.parameter_one = g_strdup(value_one);

  // Get the value of the parameter "ParameterTwo" and save it locally
  if (!ax_parameter_get(parameter, "ParameterTwo", &value_two, &error)) {
    goto error_out;
  }
  syslog(LOG_INFO, "The value of \"ParameterTwo\" is \"%s\"", value_two);
  g_free(glob_app_data.parameter_two);
  glob_app_data.parameter_two = g_strdup(value_two);

  // Remove parameter "ParameterThree"
  if (!ax_parameter_remove(parameter, "ParameterThree", &error)) {
    goto error_out;
  }

  // Write parameters to file and initiate callbacks by using do_sync set to TRUE
  if (!ax_parameter_set(parameter, "ParameterOne", value_one, TRUE, &error)) {
      goto error_out;
  }

  // List all parameter belonging to this application
  list = ax_parameter_list(parameter, &error);
  if (list == NULL) {
    goto error_out;
  }

  list_tmp = list;
  while (list_tmp != NULL) {
    syslog(LOG_INFO, "Parameter in list: \"%s\"", (gchar*)list_tmp->data);
    list_tmp = g_list_next(list_tmp);
  }

  // Free the list and it's members
  list_tmp = list;
  while (list_tmp != NULL) {
    // Free the string
    g_free(list_tmp->data);
    list_tmp = g_list_next(list_tmp);
  }
  g_list_free(list);

  // Register a callback for application defined parameters,
  // that will be called each time "ParameterOne" is changed
  if (!ax_parameter_register_callback(parameter,
                                      "ParameterOne",
                                      parameter_callback,
                                      &glob_app_data,
                                      &error)) {
    goto error_out;
  }

  // Register same callback for application defined parameters,
  // that will be called each time "ParameterTwo" is changed
  if (!ax_parameter_register_callback(parameter,
                                      "ParameterTwo",
                                      parameter_callback,
                                      &glob_app_data,
                                      &error)) {
    goto error_out;
  }

  // Register a callback which checks the locally stored application parameters periodically
  check_application_parameter(&glob_app_data);
  g_timeout_add_seconds(CHECK_PARAMETER_SECS, check_application_parameter, &glob_app_data);

  // Start the main loop
  g_main_loop_run(loop);

  // Unref the main loop when the main loop has been quit
  g_main_loop_unref(loop);

error_out:

  if (error) {
    syslog(LOG_INFO, "Failed %s", error->message);
    g_error_free(error);
  }

  // Clean up
  g_free(value_one);
  g_free(value_two);
  ax_parameter_free(parameter);

  return 0;
}
