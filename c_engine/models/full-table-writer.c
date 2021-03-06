/** @file full-table-writer.c
 * Writes out a table of values covering many aspects of the simulation, in
 * comma-separated values (csv) format.  On systems where the simulation runs
 * without a GUI, this table is meant to imitate the outputs the user will be
 * familiar with from NAADSM-PC.
 *
 * @author Neil Harvey <neilharvey@gmail.com><br>
 *   Department of Computing & Information Science, University of Guelph<br>
 *   Guelph, ON N1G 2W1<br>
 *   CANADA
 *
 * Copyright &copy; University of Guelph, 2009
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * @todo check errno after opening the file for writing.
 */

#if HAVE_CONFIG_H
#  include <config.h>
#endif

/* To avoid name clashes when multiple modules have the same interface. */
#define is_singleton full_table_writer_is_singleton
#define new full_table_writer_new
#define run full_table_writer_run
#define reset full_table_writer_reset
#define events_listened_for full_table_writer_events_listened_for
#define is_listening_for full_table_writer_is_listening_for
#define has_pending_actions full_table_writer_has_pending_actions
#define has_pending_infections full_table_writer_has_pending_infections
#define to_string full_table_writer_to_string
#define local_printf full_table_writer_printf
#define local_fprintf full_table_writer_fprintf
#define local_free full_table_writer_free
#define handle_before_any_simulations_event full_table_writer_handle_before_any_simulations_event
#define handle_declaration_of_outputs_event full_table_writer_handle_declaration_of_outputs_event
#define handle_before_each_simulation_event full_table_writer_handle_before_each_simulation_event
#define handle_new_day_event full_table_writer_handle_new_day_event
#define handle_end_of_day_event full_table_writer_handle_end_of_day_event
#define events_created full_table_writer_events_created

#include "model.h"
#include "model_util.h"

#include <stdio.h>
extern FILE *stdout;

#if STDC_HEADERS
#  include <string.h>
#endif

#if HAVE_MATH_H
#  include <math.h>
#endif

#if HAVE_STRINGS_H
#  include <strings.h>
#endif

#include "full-table-writer.h"

#if !HAVE_ROUND && HAVE_RINT
#  define round rint
#endif

/* Temporary fix -- "round" and "rint" are in the math library on Red Hat 7.3,
 * but they're #defined so AC_CHECK_FUNCS doesn't find them. */
double round (double x);

/** This must match an element name in the DTD. */
#define MODEL_NAME "full-table-writer"



#define NEVENTS_LISTENED_FOR 5
EVT_event_type_t events_listened_for[] = { EVT_BeforeAnySimulations,
  EVT_DeclarationOfOutputs, EVT_BeforeEachSimulation, EVT_NewDay, EVT_EndOfDay };



/* Specialized information for this model. */
typedef struct
{
  char *filename; /* with the .csv extension */
  FILE *stream; /* The open file. */
  gboolean stream_is_stdout;
  int run_number;
  gboolean printed_header;
}
local_data_t;



/**
 * Returns a copy of the given text, transformed into CamelCase.
 *
 * @param text the original text.
 * @param capitalize_first if TRUE, the first character of the text will be
 *   capitalized.
 * @return a newly-allocated string.  If the "text" parameter is NULL, the
 *   return value will also be NULL.
 */
char *
camelcase (char *text, gboolean capitalize_first)
{
  char *newtext; /* Address of the newly-allocated CamelCase string. */
  char *newchar; /* Pointer to the current character of the new string, as we
    are building it. */
  gboolean last_was_space;

  newtext = NULL;
  if (text != NULL)
    {
      newtext = g_new (char, strlen(text)+1); /* +1 to leave room for the '\0' at the end */
      last_was_space = capitalize_first;
      for (newchar = newtext; *text != '\0'; text++)
        {
          if (g_ascii_isspace (*text))
            {
              last_was_space = TRUE;
              continue;
            }
          if (last_was_space && g_ascii_islower(*text))
            *newchar++ = g_ascii_toupper (*text);
          else
            *newchar++ = *text;
          last_was_space = FALSE;
        }
      /* End the new string with a null character. */
      *newchar = '\0';
    }
  return newtext;
}



/**
 * Before any simulations, this module:
 * - sets the run number to zero
 * - sets the "printed header" flag to false.
 * - opens its output file and writes the table header
 *
 * @param self the model.
 * @param queue for any new events the model creates.
 */
void
handle_before_any_simulations_event (struct naadsm_model_t_ * self,
                                     EVT_event_queue_t * queue)
{
  local_data_t *local_data;

#if DEBUG
  g_debug ("----- ENTER handle_before_any_simulations_event (%s)", MODEL_NAME);
#endif

  local_data = (local_data_t *) (self->model_data);

  /* This count will be incremented for each new simulation. */
  local_data->run_number = 0;

  local_data->printed_header = FALSE;

  if (!local_data->stream_is_stdout)
    {
      errno = 0;
      local_data->stream = fopen (local_data->filename, "w");
      if (errno != 0)
        {
          g_error ("%s: %s error when attempting to open file \"%s\"",
                   MODEL_NAME, strerror(errno), local_data->filename);
        }
    }

#if DEBUG
  g_debug ("----- EXIT handle_before_any_simulations_event (%s)", MODEL_NAME);
#endif
}



/**
 * Responds to an "declaration of outputs" event by adding the output
 * variable(s) to its list of outputs to report.
 *
 * @param self the model.
 * @param event a declaration of outputs event.
 */
void
handle_declaration_of_outputs_event (struct naadsm_model_t_ * self,
                                     EVT_declaration_of_outputs_event_t *event)
{
  unsigned int n, i;

#if DEBUG
  g_debug ("----- ENTER handle_declaration_of_outputs_event (%s)", MODEL_NAME);
#endif

  n = event->outputs->len;
  for (i = 0; i < n; i++)
    g_ptr_array_add (self->outputs, g_ptr_array_index (event->outputs, i));
#if DEBUG
  g_debug ("%s now has %u output variables", MODEL_NAME, self->outputs->len);
#endif

#if DEBUG
  g_debug ("----- EXIT handle_declaration_of_outputs_event (%s)", MODEL_NAME);
#endif
}



/**
 * Before each simulation, this module increments its "run number".
 *
 * @param self the model.
 */
void
handle_before_each_simulation_event (struct naadsm_model_t_ * self)
{
  local_data_t *local_data;

#if DEBUG
  g_debug ("----- ENTER handle_before_each_simulation_event (%s)", MODEL_NAME);
#endif

  local_data = (local_data_t *) (self->model_data);
  local_data->run_number++;

#if DEBUG
  g_debug ("----- EXIT handle_before_each_simulation_event (%s)", MODEL_NAME);
#endif
}



/**
 * On the first "new day" event of a set of simulations, we write the table
 * header.  It would be nicer to do this before any simulations begin, but we
 * need to be sure that the output variables have been initialized with all
 * causes of infection, reasons for destruction, etc. before the table header
 * is written.
 *
 * @param self the model.
 * @param event a new day event.
 */
void
handle_new_day_event (struct naadsm_model_t_ * self, EVT_new_day_event_t * event)
{
  local_data_t *local_data;
  unsigned int i,j;
  RPT_reporting_t *reporting;
  GPtrArray *names;
  char *name, *camel;

#if DEBUG
  g_debug ("----- ENTER handle_new_day_event (%s)", MODEL_NAME);
#endif

  local_data = (local_data_t *) (self->model_data);

  /* Print the table header, if we haven't already. */
  if (!local_data->printed_header)
    {
      /* The first two fields are run and day. */
      fprintf (local_data->stream, "Run,Day");

      /* Output the other variables in the order they were created in the
       * new() function. */
      for (i = 0; i < self->outputs->len; i++)
        {
          reporting = (RPT_reporting_t *) g_ptr_array_index (self->outputs, i);
          /* Skip the ones that are never reported. */
          if (reporting->frequency == RPT_never)
            continue;
          names = RPT_reporting_names (reporting);
          for (j = 0; j < names->len; j++)
            {
              name = (char *) g_ptr_array_index (names, j);
              camel = camelcase (name, /* capitalize first = */ FALSE); 
              fprintf (local_data->stream, ",%s", camel);
              g_free (camel);
              g_free (name);
            }
          g_ptr_array_free (names, TRUE);
        }
      fprintf (local_data->stream, "\n");
      fflush (local_data->stream);

      local_data->printed_header = TRUE;
    }  

#if DEBUG
  g_debug ("----- EXIT handle_new_day_event (%s)", MODEL_NAME);
#endif
  return;
}



/**
 * Responds to an end of day event by writing a table row.
 *
 * @param self the model.
 * @param event an end of day event.
 */
void
handle_end_of_day_event (struct naadsm_model_t_ * self,
                         EVT_end_of_day_event_t * event)
{
  local_data_t *local_data;
  unsigned int i,j;
  RPT_reporting_t *reporting;
  unsigned int var_count;
  GPtrArray *values;
  char *value;

#if DEBUG
  g_debug ("----- ENTER handle_end_of_day_event (%s)", MODEL_NAME);
#endif

  local_data = (local_data_t *) (self->model_data);

  /* An end-of-day event is currently used to set up the initially infected,
   * vaccinated, and destroyed units.  That will appear as "day 0", i.e.,
   * before the first day of the simulation.  Don't print a row for day 0. */
  if (event->day > 0)
    {
      /* The first two fields are run and day. */
      fprintf (local_data->stream, "%i,%i", local_data->run_number, event->day);

      /* Output the other variables in the order they were created in the
       * new() function. */
      for (i = 0; i < self->outputs->len; i++)
        {
          reporting = (RPT_reporting_t *) g_ptr_array_index (self->outputs, i);
          /* Skip the ones that are never reported. */
          if (reporting->frequency == RPT_never)
            continue;

          /* Remember to include all values if this is the last day. */
          if (RPT_reporting_due (reporting, event->day)
              || (event->done && reporting->frequency == RPT_once))
            {
              values = RPT_reporting_values_as_strings (reporting);
              for (j = 0; j < values->len; j++)
                {
                  value = (char *) g_ptr_array_index (values, j);
                  fprintf (local_data->stream, ",%s", value);
                  g_free (value);
                }
              g_ptr_array_free (values, TRUE);
            }
          else
            {
              /* The variable isn't due to be reported today.  Its columns will be
               * empty; just print a bunch of commas. */
              var_count = RPT_reporting_var_count (reporting);
              for (j = 0; j < var_count; j++)
                fprintf (local_data->stream, ",");
            }
        } /* end of loop over output variables */
      fprintf (local_data->stream, "\n");
    }

#if DEBUG
  g_debug ("----- EXIT handle_end_of_day_event (%s)", MODEL_NAME);
#endif
  return;
}



/**
 * Runs this model.
 *
 * @param self the model.
 * @param herds a list of herds.
 * @param zones a list of zones.
 * @param event the event that caused the model to run.
 * @param rng a random number generator.
 * @param queue for any new events the model creates.
 */
void
run (struct naadsm_model_t_ *self, HRD_herd_list_t * herds,
     ZON_zone_list_t * zones, EVT_event_t * event, RAN_gen_t * rng, EVT_event_queue_t * queue)
{
#if DEBUG
  g_debug ("----- ENTER run (%s)", MODEL_NAME);
#endif

  switch (event->type)
    {
    case EVT_BeforeAnySimulations:
      handle_before_any_simulations_event (self, queue);
      break;
    case EVT_DeclarationOfOutputs:
      handle_declaration_of_outputs_event (self, &(event->u.declaration_of_outputs));
      break;
    case EVT_BeforeEachSimulation:
      handle_before_each_simulation_event (self);
      break;
    case EVT_NewDay:
      handle_new_day_event (self, &(event->u.new_day));
      break;
    case EVT_EndOfDay:
      handle_end_of_day_event (self, &(event->u.end_of_day));
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
#if DEBUG
  g_debug ("----- ENTER reset (%s)", MODEL_NAME);
#endif

  /* Nothing to do. */

#if DEBUG
  g_debug ("----- EXIT reset (%s)", MODEL_NAME);
#endif
  return;
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
 * Reports whether this module has any pending infections to cause.
 *
 * @param self this module.
 * @return TRUE if this module has pending infections.
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
  local_data_t *local_data;
  GString *s;
  char *chararray;

  local_data = (local_data_t *) (self->model_data);
  s = g_string_new (NULL);
  g_string_sprintf (s, "<%s filename=\"%s\">", MODEL_NAME, local_data->filename);

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

  local_data = (local_data_t *) (self->model_data);

  /* Flush and close the file. */
  if (local_data->stream_is_stdout)
    fflush (local_data->stream);
  else
    fclose (local_data->stream);

  /* Free the dynamically-allocated parts. */
  g_free (local_data->filename);
  g_free (local_data);
  g_ptr_array_free (self->outputs, TRUE);
  g_free (self);

#if DEBUG
  g_debug ("----- EXIT free (%s)", MODEL_NAME);
#endif
}



/**
 * Returns whether this module is a singleton or not.
 */
gboolean
is_singleton (void)
{
  return TRUE;
}



/**
 * Returns a new full table writer.
 */
naadsm_model_t *
new (scew_element * params, HRD_herd_list_t * herds, projPJ projection,
     ZON_zone_list_t * zones)
{
  naadsm_model_t *self;
  local_data_t *local_data;
  scew_element *e;

#if DEBUG
  g_debug ("----- ENTER new (%s)", MODEL_NAME);
#endif

  self = g_new (naadsm_model_t, 1);
  local_data = g_new (local_data_t, 1);

  self->name = MODEL_NAME;
  self->events_listened_for = events_listened_for;
  self->nevents_listened_for = NEVENTS_LISTENED_FOR;
  self->outputs = g_ptr_array_new ();
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

  /* Get the filename for the table.  If the filename is omitted, blank, '-',
   * or 'stdout' (case insensitive), then the table is written to standard
   * output. */
  e = scew_element_by_name (params, "filename");
  if (e == NULL)
    {
      local_data->filename = g_strdup ("stdout"); /* just so we have something
        to display, and to free later */
      local_data->stream = stdout;
      local_data->stream_is_stdout = TRUE;    
    }
  else
    {
      local_data->filename = PAR_get_text (e);
      if (local_data->filename == NULL
          || g_ascii_strcasecmp (local_data->filename, "") == 0
          || g_ascii_strcasecmp (local_data->filename, "-") == 0
          || g_ascii_strcasecmp (local_data->filename, "stdout") == 0)
        {
          local_data->stream = stdout;
          local_data->stream_is_stdout = TRUE;
        }
      else
        {
          char *tmp;
          if (!g_str_has_suffix(local_data->filename, ".csv"))
            {
              tmp = local_data->filename;
              local_data->filename = g_strdup_printf("%s.csv", tmp);
              g_free(tmp);
            }
          tmp = local_data->filename;
          local_data->filename = naadsm_insert_node_number_into_filename (local_data->filename);
          g_free(tmp);
          local_data->stream_is_stdout = FALSE;
        }
    }

  /* This module maintains no output variables of its own.  Before any
   * simulations begin, we will gather all the output variables available from
   * other modules. */

#if DEBUG
  g_debug ("----- EXIT new (%s)", MODEL_NAME);
#endif

  return self;
}

/* end of file full-table-writer.c */
