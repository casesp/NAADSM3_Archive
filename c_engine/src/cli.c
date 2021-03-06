/** @file cli.c
 * A command-line interface for NAADSM.
 *
 * @author Neil Harvey <neilharvey@gmail.com><br>
 *   Department of Computing & Information Science, University of Guelph<br>
 *   Guelph, ON N1G 2W1<br>
 *   CANADA
 *
 * Copyright &copy; University of Guelph and Colorado State University 2010
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 */

#if HAVE_CONFIG_H
#  include <config.h>
#endif

#if HAVE_MPI && !CANCEL_MPI
#  include "mpix.h"
#endif

#include "naadsm.h"
#include "general.h"



int
main (int argc, char *argv[])
{
  int verbosity = 0;
  const char *parameter_file = NULL;
  const char *herd_file = NULL;
#ifdef USE_SC_GUILIB
  const char *production_type_file = NULL;
#endif
  const char *output_file = NULL;
  double fixed_rng_value = -1;
  int seed = -1;
  GError *option_error = NULL;
  GOptionContext *context;
  GOptionEntry options[] = {
    { "herd-file", 'h', 0, G_OPTION_ARG_FILENAME, &herd_file, "Herd file", NULL },
    { "verbosity", 'V', 0, G_OPTION_ARG_INT, &verbosity, "Message verbosity level (0 = simulation output only, 1 = all debugging output)", NULL },
    { "output-file", 'o', 0, G_OPTION_ARG_FILENAME, &output_file, "Output file", NULL },
    { "fixed-random-value", 'r', 0, G_OPTION_ARG_DOUBLE, &fixed_rng_value, "Fixed number to use instead of random numbers", NULL },
    { "rng-seed", 's', 0, G_OPTION_ARG_INT, &seed, "Seed used to initialize the random number generator", NULL },
#ifdef USE_SC_GUILIB
    { "production-types", 'p', 0, G_OPTION_ARG_FILENAME, &production_type_file, "File containing production types used in this scenario", NULL },
#endif
    { NULL }
  };

#if HAVE_MPI && !CANCEL_MPI
  /* Initialize MPI. */
  if (MPIx_Init (&argc, &argv) != MPI_SUCCESS)
    g_error ("Couldn't initialize MPI.");
#endif

  clear_naadsm_fns ();

#ifdef USE_SC_GUILIB
  _scenario.scenarioId = NULL;
  _scenario.description = NULL;
  _scenario.nruns = 0;
  _scenario.random_seed = 0;
  _scenario.start_time = _scenario.end_time = 0;

  _iteration.susceptible_herds = NULL;
  _iteration.infectious_herds = NULL;
  _iteration._herdsInZones = NULL;
  _iteration.zoneFociCreated = FALSE;
  _iteration.diseaseEndDay = -1;
  _iteration.outbreakEndDay = -1;
  _iteration.first_detection = FALSE;
#endif

  init_MAIN_structs();

  context = g_option_context_new ("- Runs epidemiological simulations for animal populations");
  g_option_context_add_main_entries (context, options, /* translation = */ NULL);
  if (!g_option_context_parse (context, &argc, &argv, &option_error))
    {
      g_error ("option parsing failed: %s\n", option_error->message);
    }
  if (argc >= 2)
    parameter_file = argv[1];
  else
    {
      g_error ("Need name of parameter file");
    }
  g_option_context_free (context);

#ifdef USE_SC_GUILIB
  run_sim_main (herd_file,
                parameter_file,
                output_file,
                fixed_rng_value,
                verbosity,
                seed,
                production_type_file);
#else
  run_sim_main (herd_file,
                parameter_file,
                output_file,
                fixed_rng_value,
                verbosity,
                seed);
#endif

#if HAVE_MPI && !CANCEL_MPI
  MPI_Finalize ();
#endif

  return EXIT_SUCCESS;
}

/* end of file cli.c */
