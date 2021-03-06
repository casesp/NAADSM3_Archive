/** @file destruction-list-monitor.c
 * Tracks the number of units waiting to be destroyed.
 *
 * @author Neil Harvey <neilharvey@gmail.com><br>
 *   Department of Computing & Information Science, University of Guelph<br>
 *   Guelph, ON N1G 2W1<br>
 *   CANADA
 * @version 0.1
 * @date April 2004
 *
 * Copyright &copy; University of Guelph, 2004-2009
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 */

#if HAVE_CONFIG_H
#  include <config.h>
#endif

/* To avoid name clashes when multiple modules have the same interface. */
#define new destruction_list_monitor_new
#define run destruction_list_monitor_run
#define reset destruction_list_monitor_reset
#define events_listened_for destruction_list_monitor_events_listened_for
#define is_listening_for destruction_list_monitor_is_listening_for
#define has_pending_actions destruction_list_monitor_has_pending_actions
#define has_pending_infections destruction_list_monitor_has_pending_infections
#define to_string destruction_list_monitor_to_string
#define local_printf destruction_list_monitor_printf
#define local_fprintf destruction_list_monitor_fprintf
#define local_free destruction_list_monitor_free
#define handle_before_any_simulations_event destruction_list_monitor_handle_before_any_simulations_event
#define handle_new_day_event destruction_list_monitor_handle_new_day_event
#define handle_commitment_to_destroy_event destruction_list_monitor_handle_commitment_to_destroy_event
#define handle_destruction_event destruction_list_monitor_handle_destruction_event

#include "model.h"

#if STDC_HEADERS
#  include <string.h>
#endif

#include "destruction-list-monitor.h"

/** This must match an element name in the DTD. */
#define MODEL_NAME "destruction-list-monitor"



#define NEVENTS_LISTENED_FOR 4
EVT_event_type_t events_listened_for[] = { EVT_BeforeAnySimulations, EVT_NewDay,
  EVT_CommitmentToDestroy,
  EVT_Destruction
};



/** Specialized information for this model. */
typedef struct
{
  GPtrArray *production_types;
  unsigned int nherds;          /* Number of herds. */
  GHashTable *status; /**< The status of each unit with respect to destruction.
    If the unit is not awaiting destruction, it will not be present in the
    table. */
  unsigned int peak_nherds, peak_nanimals;
  unsigned int peak_wait;
  double sum; /**< The numerator for calculating the average wait time. */
  unsigned int count; /**< The denominator for calculating the average wait time. */
  RPT_reporting_t *nherds_awaiting_destruction;
  RPT_reporting_t *nherds_awaiting_destruction_by_prodtype;
  RPT_reporting_t *nanimals_awaiting_destruction;
  RPT_reporting_t *nanimals_awaiting_destruction_by_prodtype;
  RPT_reporting_t *peak_nherds_awaiting_destruction;
  RPT_reporting_t *peak_nherds_awaiting_destruction_day;
  RPT_reporting_t *peak_nanimals_awaiting_destruction;
  RPT_reporting_t *peak_nanimals_awaiting_destruction_day;
  RPT_reporting_t *peak_wait_time;
  RPT_reporting_t *average_wait_time;
  RPT_reporting_t *unit_days_in_queue;
  RPT_reporting_t *animal_days_in_queue;
}
local_data_t;



/**
 * Before any simulations, this module announces the output variables it is
 * recording.
 *
 * @param self this module.
 * @param queue for any new events this function creates.
 */
void
handle_before_any_simulations_event (struct naadsm_model_t_ *self,
                                     EVT_event_queue_t *queue)
{
  unsigned int n, i;
  RPT_reporting_t *output;
  GPtrArray *outputs = NULL;

  n = self->outputs->len;
  for (i = 0; i < n; i++)
    {
      output = (RPT_reporting_t *) g_ptr_array_index (self->outputs, i);
      if (output->frequency != RPT_never)
        {
          if (outputs == NULL)
            outputs = g_ptr_array_new();
          g_ptr_array_add (outputs, output);
        }
    }

  if (outputs != NULL)
    EVT_event_enqueue (queue, EVT_new_declaration_of_outputs_event (outputs));
  /* We don't free the pointer array, that will be done when the event is freed
   * after all interested modules have processed it. */

  return;
}



/**
 * Responds to a new day event by updating unit-days in queue and animal-days
 * in queue.
 *
 * @param self the model.
 */
void
handle_new_day_event (struct naadsm_model_t_ *self)
{
  local_data_t *local_data;

#if DEBUG
  g_debug ("----- ENTER handle_new_day_event (%s)", MODEL_NAME);
#endif

  local_data = (local_data_t *) (self->model_data);

  RPT_reporting_add_integer (local_data->unit_days_in_queue,
                             RPT_reporting_get_integer (local_data->nherds_awaiting_destruction, NULL),
                             NULL);
  RPT_reporting_add_integer (local_data->animal_days_in_queue,
                             RPT_reporting_get_integer (local_data->nanimals_awaiting_destruction, NULL),
                             NULL);

#if DEBUG
  g_debug ("----- EXIT handle_new_day_event (%s)", MODEL_NAME);
#endif

  return;
}



/**
 * Responds to a commitment to destroy event by recording the herd's status as
 * "waiting".
 *
 * @param self the model.
 * @param event a commitment to destroy event.
 */
void
handle_commitment_to_destroy_event (struct naadsm_model_t_ *self,
                                    EVT_commitment_to_destroy_event_t * event)
{
  local_data_t *local_data;
  HRD_herd_t *herd;
  unsigned int nherds, nanimals;

#if DEBUG
  g_debug ("----- ENTER handle_commitment_to_destroy_event (%s)", MODEL_NAME);
#endif

  local_data = (local_data_t *) (self->model_data);
  herd = event->herd;

  if (g_hash_table_lookup (local_data->status, herd) == NULL)
    {
      g_hash_table_insert (local_data->status, herd, GINT_TO_POINTER(1));

      if (NULL != naadsm_queue_herd_for_destruction)
        {
          naadsm_queue_herd_for_destruction (herd->index);
        }

      /* Increment the count of herds awaiting destruction. */
      RPT_reporting_add_integer (local_data->nherds_awaiting_destruction, 1, NULL);
      if (local_data->nherds_awaiting_destruction_by_prodtype->frequency != RPT_never)
        RPT_reporting_add_integer1 (local_data->nherds_awaiting_destruction_by_prodtype, 1,
                                    herd->production_type_name);
      nherds = RPT_reporting_get_integer (local_data->nherds_awaiting_destruction, NULL);
      if (nherds > local_data->peak_nherds)
        {
          local_data->peak_nherds = nherds;
          RPT_reporting_set_integer (local_data->peak_nherds_awaiting_destruction, nherds, NULL);
          RPT_reporting_set_integer (local_data->peak_nherds_awaiting_destruction_day, event->day, NULL);
        }

      /* Increment the count of animals awaiting destruction. */
      RPT_reporting_add_integer (local_data->nanimals_awaiting_destruction, herd->size, NULL);
      if (local_data->nanimals_awaiting_destruction_by_prodtype->frequency != RPT_never)
        RPT_reporting_add_integer1 (local_data->nanimals_awaiting_destruction_by_prodtype,
                                    herd->size, herd->production_type_name);
      nanimals = RPT_reporting_get_integer (local_data->nanimals_awaiting_destruction, NULL);
      if (nanimals > local_data->peak_nanimals)
        {
          local_data->peak_nanimals = nanimals;
          RPT_reporting_set_integer (local_data->peak_nanimals_awaiting_destruction, nanimals,
                                     NULL);
          RPT_reporting_set_integer (local_data->peak_nanimals_awaiting_destruction_day, event->day,
                                     NULL);
        }
    }

#if DEBUG
  g_debug ("----- EXIT handle_commitment_to_destroy_event (%s)", MODEL_NAME);
#endif
}



/**
 * Responds to a destruction event by removing the herd's "waiting" status and
 * updating the peak and average wait times.
 *
 * @param self the model.
 * @param event a destruction event.
 */
void
handle_destruction_event (struct naadsm_model_t_ *self, EVT_destruction_event_t * event)
{
  local_data_t *local_data;
  HRD_herd_t *herd;
  unsigned int wait;

#if DEBUG
  g_debug ("----- ENTER handle_destruction_event (%s)", MODEL_NAME);
#endif

  local_data = (local_data_t *) (self->model_data);
  herd = event->herd;

  /* Special case: if a unit's starting state is Destroyed, it won't be on a
   * waiting list and it won't affect the various counts maintained by this
   * monitor. */
  if (g_hash_table_lookup (local_data->status, herd) != NULL)
    {
      /* The day when the unit went onto the waiting list is recorded in the
       * destruction event. */
      wait = event->day - event->day_commitment_made;

      /* Update the peak wait time. */
      local_data->peak_wait = MAX (local_data->peak_wait, wait);
      RPT_reporting_set_integer (local_data->peak_wait_time, local_data->peak_wait, NULL);

      /* Update the average wait time. */
      local_data->sum += wait;
      local_data->count += 1;
      RPT_reporting_set_real (local_data->average_wait_time,
                              local_data->sum / local_data->count, NULL);

      /* Mark the herd as no longer on a waiting list. */
      g_hash_table_remove (local_data->status, herd);

      /* Decrement the counts of herds and animals awaiting destruction. */
      RPT_reporting_sub_integer (local_data->nherds_awaiting_destruction, 1, NULL);
      if (local_data->nherds_awaiting_destruction_by_prodtype->frequency != RPT_never)
        RPT_reporting_sub_integer1 (local_data->nherds_awaiting_destruction_by_prodtype, 1,
                                    herd->production_type_name);
      RPT_reporting_sub_integer (local_data->nanimals_awaiting_destruction, herd->size, NULL);
      if (local_data->nanimals_awaiting_destruction_by_prodtype->frequency != RPT_never)
        RPT_reporting_sub_integer1 (local_data->nanimals_awaiting_destruction_by_prodtype,
                                    herd->size, herd->production_type_name);
    }

#if DEBUG
  g_debug ("----- EXIT handle_destruction_event (%s)", MODEL_NAME);
#endif
}



/**
 * Runs this model.
 *
 * @param self the model.
 * @param herds a herd list.
 * @param zones a zone list.
 * @param event the event that caused the model to run.
 * @param rng a random number generator.
 * @param queue for any new events the model creates.
 */
void
run (struct naadsm_model_t_ *self, HRD_herd_list_t * herds, ZON_zone_list_t * zones,
     EVT_event_t * event, RAN_gen_t * rng, EVT_event_queue_t * queue)
{
#if DEBUG
  g_debug ("----- ENTER run (%s)", MODEL_NAME);
#endif

  switch (event->type)
    {
    case EVT_BeforeAnySimulations:
      handle_before_any_simulations_event (self, queue);
      break;
    case EVT_NewDay:
      handle_new_day_event (self);
      break;
    case EVT_CommitmentToDestroy:
      handle_commitment_to_destroy_event (self, &(event->u.commitment_to_destroy));
      break;
    case EVT_Destruction:
      handle_destruction_event (self, &(event->u.destruction));
      break;
    default:
      g_error
        ("%s has received a %s event, which it does not listen for.  This should never happen.  Please contact the developer.",
         MODEL_NAME, EVT_event_type_name[event->type]);
    }

#if DEBUG
  g_debug ("----- EXIT run (%s)", MODEL_NAME);
#endif
}



/**
 * Resets this model after a simulation run.
 *
 * @param self the model.
 */
void
reset (struct naadsm_model_t_ *self)
{
  local_data_t *local_data;

#if DEBUG
  g_debug ("----- ENTER reset (%s)", MODEL_NAME);
#endif

  local_data = (local_data_t *) (self->model_data);

  g_hash_table_remove_all (local_data->status);

  RPT_reporting_zero (local_data->nherds_awaiting_destruction);
  RPT_reporting_zero (local_data->nherds_awaiting_destruction_by_prodtype);
  RPT_reporting_zero (local_data->nanimals_awaiting_destruction);
  RPT_reporting_zero (local_data->nanimals_awaiting_destruction_by_prodtype);
  RPT_reporting_zero (local_data->peak_nherds_awaiting_destruction);
  RPT_reporting_set_null (local_data->peak_nherds_awaiting_destruction_day, NULL);
  RPT_reporting_zero (local_data->peak_nanimals_awaiting_destruction);
  RPT_reporting_set_null (local_data->peak_nanimals_awaiting_destruction_day, NULL);
  RPT_reporting_set_null (local_data->peak_wait_time, NULL);
  RPT_reporting_set_null (local_data->average_wait_time, NULL);
  RPT_reporting_zero (local_data->unit_days_in_queue);
  RPT_reporting_zero (local_data->animal_days_in_queue);

  local_data->peak_nherds = local_data->peak_nanimals = 0;
  local_data->peak_wait = 0;
  local_data->sum = local_data->count = 0;

#if DEBUG
  g_debug ("----- EXIT reset (%s)", MODEL_NAME);
#endif
}



/**
 * Reports whether this model is listening for a given event type.
 *
 * @param self the model.
 * @param event_type an event type.
 * @return TRUE if the model is listening for the event type.
 */
gboolean
is_listening_for (struct naadsm_model_t_ *self, EVT_event_type_t event_type)
{
  int i;

  for (i = 0; i < self->nevents_listened_for; i++)
    if (self->events_listened_for[i] == event_type)
      return TRUE;
  return FALSE;
}



/**
 * Reports whether this model has any pending actions to carry out.
 *
 * @param self the model.
 * @return TRUE if the model has pending actions.
 */
gboolean
has_pending_actions (struct naadsm_model_t_ * self)
{
  return FALSE;
}



/**
 * Reports whether this model has any pending infections to cause.
 *
 * @param self the model.
 * @return TRUE if the model has pending infections.
 */
gboolean
has_pending_infections (struct naadsm_model_t_ * self)
{
  return FALSE;
}



/**
 * Returns a text representation of this model.
 *
 * @param self the model.
 * @return a string.
 */
char *
to_string (struct naadsm_model_t_ *self)
{
  GString *s;
  char *chararray;

  s = g_string_new (NULL);
  g_string_sprintf (s, "<%s>", MODEL_NAME);

  /* don't return the wrapper object */
  chararray = s->str;
  g_string_free (s, FALSE);
  return chararray;
}



/**
 * Prints this model to a stream.
 *
 * @param stream a stream to write to.
 * @param self the model.
 * @return the number of characters printed (not including the trailing '\\0').
 */
int
local_fprintf (FILE * stream, struct naadsm_model_t_ *self)
{
  char *s;
  int nchars_written;

  s = to_string (self);
  nchars_written = fprintf (stream, "%s", s);
  free (s);
  return nchars_written;
}



/**
 * Prints this model.
 *
 * @param self the model.
 * @return the number of characters printed (not including the trailing '\\0').
 */
int
local_printf (struct naadsm_model_t_ *self)
{
  return local_fprintf (stdout, self);
}



/**
 * Frees this model.
 *
 * @param self the model.
 */
void
local_free (struct naadsm_model_t_ *self)
{
  local_data_t *local_data;

#if DEBUG
  g_debug ("----- ENTER free (%s)", MODEL_NAME);
#endif

  /* Free the dynamically-allocated parts. */
  local_data = (local_data_t *) (self->model_data);
  g_hash_table_destroy (local_data->status);

  RPT_free_reporting (local_data->nherds_awaiting_destruction);
  RPT_free_reporting (local_data->nherds_awaiting_destruction_by_prodtype);
  RPT_free_reporting (local_data->nanimals_awaiting_destruction);
  RPT_free_reporting (local_data->nanimals_awaiting_destruction_by_prodtype);
  RPT_free_reporting (local_data->peak_nherds_awaiting_destruction);
  RPT_free_reporting (local_data->peak_nherds_awaiting_destruction_day);
  RPT_free_reporting (local_data->peak_nanimals_awaiting_destruction);
  RPT_free_reporting (local_data->peak_nanimals_awaiting_destruction_day);
  RPT_free_reporting (local_data->peak_wait_time);
  RPT_free_reporting (local_data->average_wait_time);
  RPT_free_reporting (local_data->unit_days_in_queue);
  RPT_free_reporting (local_data->animal_days_in_queue);

  g_free (local_data);
  g_ptr_array_free (self->outputs, TRUE);
  g_free (self);

#if DEBUG
  g_debug ("----- EXIT free (%s)", MODEL_NAME);
#endif
}



/**
 * Returns a new destruction list monitor.
 */
naadsm_model_t *
new (scew_element * params, HRD_herd_list_t * herds, projPJ projection,
     ZON_zone_list_t * zones)
{
  naadsm_model_t *self;
  local_data_t *local_data;
  scew_element *e, **ee;
  unsigned int n;
  const XML_Char *variable_name;
  RPT_frequency_t freq;
  gboolean success;
  gboolean broken_down;
  unsigned int i;      /* loop counter */

#if DEBUG
  g_debug ("----- ENTER new (%s)", MODEL_NAME);
#endif

  self = g_new (naadsm_model_t, 1);
  local_data = g_new (local_data_t, 1);

  self->name = MODEL_NAME;
  self->events_listened_for = events_listened_for;
  self->nevents_listened_for = NEVENTS_LISTENED_FOR;
  self->outputs = g_ptr_array_sized_new (10);
  self->model_data = local_data;
  self->run = run;
  self->reset = reset;
  self->is_listening_for = is_listening_for;
  self->has_pending_actions = has_pending_actions;
  self->has_pending_infections = has_pending_infections;
  self->to_string = to_string;
  self->printf = local_printf;
  self->fprintf = local_fprintf;
  self->free = local_free;

  /* Make sure the right XML subtree was sent. */
  g_assert (strcmp (scew_element_name (params), MODEL_NAME) == 0);

  local_data->nherds_awaiting_destruction =
    RPT_new_reporting ("deswUAll", RPT_integer, RPT_never);
  local_data->nherds_awaiting_destruction_by_prodtype =
    RPT_new_reporting ("deswU", RPT_group, RPT_never);
  local_data->nanimals_awaiting_destruction =
    RPT_new_reporting ("deswAAll", RPT_integer, RPT_never);
  local_data->nanimals_awaiting_destruction_by_prodtype =
    RPT_new_reporting ("deswA", RPT_group, RPT_never);
  local_data->peak_nherds_awaiting_destruction =
    RPT_new_reporting ("deswUMax", RPT_integer, RPT_never);
  local_data->peak_nherds_awaiting_destruction_day =
    RPT_new_reporting ("deswUMaxDay", RPT_integer, RPT_never);
  local_data->peak_nanimals_awaiting_destruction =
    RPT_new_reporting ("deswAMax", RPT_integer, RPT_never);
  local_data->peak_nanimals_awaiting_destruction_day =
    RPT_new_reporting ("deswAMaxDay", RPT_integer, RPT_never);
  local_data->peak_wait_time =
    RPT_new_reporting ("deswUTimeMax", RPT_integer, RPT_never);
  local_data->average_wait_time =
    RPT_new_reporting ("deswUTimeAvg", RPT_real, RPT_never);
  local_data->unit_days_in_queue =
    RPT_new_reporting ("deswUDaysInQueue", RPT_integer, RPT_never);
  local_data->animal_days_in_queue =
    RPT_new_reporting ("deswADaysInQueue", RPT_integer, RPT_never);
  g_ptr_array_add (self->outputs, local_data->nherds_awaiting_destruction);
  g_ptr_array_add (self->outputs, local_data->nherds_awaiting_destruction_by_prodtype);
  g_ptr_array_add (self->outputs, local_data->nanimals_awaiting_destruction);
  g_ptr_array_add (self->outputs, local_data->nanimals_awaiting_destruction_by_prodtype);
  g_ptr_array_add (self->outputs, local_data->peak_nherds_awaiting_destruction);
  g_ptr_array_add (self->outputs, local_data->peak_nherds_awaiting_destruction_day);
  g_ptr_array_add (self->outputs, local_data->peak_nanimals_awaiting_destruction);
  g_ptr_array_add (self->outputs, local_data->peak_nanimals_awaiting_destruction_day);
  g_ptr_array_add (self->outputs, local_data->peak_wait_time);
  g_ptr_array_add (self->outputs, local_data->average_wait_time);
  g_ptr_array_add (self->outputs, local_data->unit_days_in_queue);
  g_ptr_array_add (self->outputs, local_data->animal_days_in_queue);

  /* Set the reporting frequency for the output variables. */
  ee = scew_element_list (params, "output", &n);
#if DEBUG
  g_debug ("%u output variables", n);
#endif
  for (i = 0; i < n; i++)
    {
      e = ee[i];
      variable_name = scew_element_contents (scew_element_by_name (e, "variable-name"));
      freq = RPT_string_to_frequency (scew_element_contents
                                      (scew_element_by_name (e, "frequency")));
      broken_down = PAR_get_boolean (scew_element_by_name (e, "broken-down"), &success);
      if (!success)
      	broken_down = FALSE;
      broken_down = broken_down || (g_strstr_len (variable_name, -1, "-by-") != NULL); 
      /* Starting at version 3.2 we accept either the old, verbose output
       * variable names or the shorter ones used in NAADSM/PC. */
      if (strcmp (variable_name, "deswU") == 0
          || strncmp (variable_name, "num-units-awaiting-destruction", 30) == 0)
        {
          RPT_reporting_set_frequency (local_data->nherds_awaiting_destruction, freq);
          if (broken_down)
            RPT_reporting_set_frequency (local_data->nherds_awaiting_destruction_by_prodtype, freq);
        }
      else if (strcmp (variable_name, "deswA") == 0
               || strncmp (variable_name, "num-animals-awaiting-destruction", 32) == 0)
        {
          RPT_reporting_set_frequency (local_data->nanimals_awaiting_destruction, freq);
          if (broken_down)
            RPT_reporting_set_frequency (local_data->nanimals_awaiting_destruction_by_prodtype, freq);
        }
      else if (strcmp (variable_name, "deswUMax") == 0
               || strcmp (variable_name, "peak-num-units-awaiting-destruction") == 0)
        RPT_reporting_set_frequency (local_data->peak_nherds_awaiting_destruction, freq);
      else if (strcmp (variable_name, "deswUMaxDay") == 0)
        RPT_reporting_set_frequency (local_data->peak_nherds_awaiting_destruction_day, freq);
      else if (strcmp (variable_name, "deswAMax") == 0
               || strcmp (variable_name, "peak-num-animals-awaiting-destruction") == 0)
        RPT_reporting_set_frequency (local_data->peak_nanimals_awaiting_destruction, freq);
      else if (strcmp (variable_name, "deswAMaxDay") == 0)
        RPT_reporting_set_frequency (local_data->peak_nanimals_awaiting_destruction_day, freq);
      else if (strcmp (variable_name, "deswUTimeMax") == 0
               || strcmp (variable_name, "peak-wait-time") == 0
               || strcmp (variable_name, "peak-destruction-wait-time") == 0)
        RPT_reporting_set_frequency (local_data->peak_wait_time, freq);
      else if (strcmp (variable_name, "deswUTimeAvg") == 0
               || strcmp (variable_name, "average-wait-time") == 0
               || strcmp (variable_name, "average-destruction-wait-time") == 0)
        RPT_reporting_set_frequency (local_data->average_wait_time, freq);
      else if (strcmp (variable_name, "deswUDaysInQueue") == 0)
        RPT_reporting_set_frequency (local_data->unit_days_in_queue, freq);
      else if (strcmp (variable_name, "deswADaysInQueue") == 0)
        RPT_reporting_set_frequency (local_data->animal_days_in_queue, freq);
      else
        g_warning ("no output variable named \"%s\", ignoring", variable_name);        
    }
  free (ee);

  local_data->nherds = HRD_herd_list_length (herds);
  local_data->production_types = herds->production_type_names;
  for (i = 0; i < local_data->production_types->len; i++)
    {
      RPT_reporting_set_integer1 (local_data->nherds_awaiting_destruction_by_prodtype, 0,
                                  (char *) g_ptr_array_index (local_data->production_types, i));
      RPT_reporting_set_integer1 (local_data->nanimals_awaiting_destruction_by_prodtype, 0,
                                  (char *) g_ptr_array_index (local_data->production_types, i));
    }

  /* Initialize the herd status table. */
  local_data->status = g_hash_table_new (g_direct_hash, g_direct_equal);

#if DEBUG
  g_debug ("----- EXIT new (%s)", MODEL_NAME);
#endif

  return self;
}

/* end of file destruction-list-monitor.c */
