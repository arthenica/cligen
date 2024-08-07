#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "fixtures/output/gnutls-cli-debug-args.h"
#include <errno.h>
#include <getopt.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef _WIN32
#include <unistd.h>
#endif /* !_WIN32 */
#include <limits.h>

struct gnutls_cli_debug_options gnutls_cli_debug_options;

static long
parse_number (const char *arg)
{
  char *endptr = NULL;
  errno = 0;
  long result;

  if (strncmp (arg, "0x", 2) == 0)
    result = strtol (arg + 2, &endptr, 16);
  else if (strncmp (arg, "0", 1) == 0
           && strspn (arg, "012345678") == strlen (optarg))
    result = strtol (arg + 1, &endptr, 8);
  else
    result = strtol (arg, &endptr, 10);

  if (errno != 0 || (endptr && *endptr != '\0'))
    {
      char buf[80];
      snprintf (buf, sizeof(buf),
                "'%s' is not a recognizable number",
                arg);
      perror (buf);
      exit (EXIT_FAILURE);
    }

  return result;
}

/* Long options.  */
static const struct option long_options[] =
{
  { "debug", required_argument, 0, 'd' },
  { "verbose", no_argument, 0, 'V' },
  { "port", required_argument, 0, 'p' },
  { "starttls-proto", required_argument, 0, CHAR_MAX + 2 },
  { "app-proto", required_argument, 0, CHAR_MAX + 1 },
  { "version", optional_argument, 0, 'v' },
  { "help", no_argument, 0, 'h' },
  { "more-help", no_argument, 0, '!' },
  { 0, 0, 0, 0 }

};

int
process_options (int argc, char **argv)
{
  struct gnutls_cli_debug_options *opts = &gnutls_cli_debug_options;
  int opt;


  while ((opt = getopt_long (argc, argv, "!Vd:hp:v:",
                             long_options, NULL)) != EOF)
    switch (opt)
      {
      case '\0': /* Long option.  */
        break;
      case 'd':
        opts->present.debug = true;
        opts->arg.debug = optarg;
        opts->value.debug = parse_number(optarg);
        opts->enabled.debug = true;
        break;
      case 'V':
        opts->present.verbose = true;
        opts->enabled.verbose = true;
        break;
      case 'p':
        opts->present.port = true;
        opts->arg.port = optarg;
        opts->value.port = parse_number(optarg);
        opts->enabled.port = true;
        break;
      case CHAR_MAX + 2: /* --starttls-proto */
      case CHAR_MAX + 1: /* --app-proto */
        opts->present.starttls_proto = true;
        opts->arg.starttls_proto = optarg;
        opts->enabled.starttls_proto = true;
        break;
      case 'v':
        opts->present.version = true;
        opts->arg.version = optarg;
        opts->enabled.version = true;
        break;
      case 'h':
        opts->present.help = true;
        opts->enabled.help = true;
        break;
      case '!':
        opts->present.more_help = true;
        opts->enabled.more_help = true;
        break;
      default:
        usage (stderr, EXIT_FAILURE);
        break;
      }

  if (HAVE_OPT(DEBUG) && OPT_VALUE_DEBUG < 0)
    {
      fprintf (stderr, "%s option value %d is out of range\n",
               "debug", opts->value.debug);
      exit (EXIT_FAILURE);
    }
  if (HAVE_OPT(DEBUG) && OPT_VALUE_DEBUG > 9999)
    {
      fprintf (stderr, "%s option value %d is out of range\n",
               "debug", opts->value.debug);
      exit (EXIT_FAILURE);
    }
  if (HAVE_OPT(PORT) && OPT_VALUE_PORT < 0)
    {
      fprintf (stderr, "%s option value %d is out of range\n",
               "port", opts->value.port);
      exit (EXIT_FAILURE);
    }
  if (HAVE_OPT(PORT) && OPT_VALUE_PORT > 65536)
    {
      fprintf (stderr, "%s option value %d is out of range\n",
               "port", opts->value.port);
      exit (EXIT_FAILURE);
    }


  if (HAVE_OPT(HELP))
    {
      USAGE(0);
    }

  if (HAVE_OPT(MORE_HELP))
#ifdef _WIN32
    {
      USAGE(0);
    }
#else /* _WIN32 */
    {
      pid_t pid;
      int pfds[2];

      if (pipe (pfds) < 0)
        {
          perror ("pipe");
          exit (EXIT_FAILURE);
        }

      pid = fork ();
      if (pid < 0)
        {
          perror ("fork");
          exit (EXIT_FAILURE);
        }

      if (pid == 0)
        {
          close (pfds[0]);
          dup2 (pfds[1], STDOUT_FILENO);
          close (pfds[1]);

          usage (stdout, 0);
        }
      else
        {
          const char *args[2];
          const char *envvar;

          close (pfds[1]);
          dup2 (pfds[0], STDIN_FILENO);
          close (pfds[0]);

          envvar = secure_getenv ("PAGER");
          if (!envvar || *envvar == '\0')
            args[0] = "more";
          else
            args[0] = envvar;

          args[1] = NULL;

          execvp (args[0], (char * const *)args);

          exit (EXIT_FAILURE);
        }
    }
#endif /* !_WIN32 */

  if (HAVE_OPT(VERSION))
    {
      if (!OPT_ARG_VERSION || !strcmp (OPT_ARG_VERSION, "c"))
        {
          const char str[] =
            "gnutls-cli-debug 3.7.4\n"
            "Copyright (C) 2024 Daiki Ueno\n"
            "This is free software. It is licensed for use, modification and\n"
            "redistribution under the terms of the GNU General Public License,\n"
            "version 3 or later <http://gnu.org/licenses/gpl.html>\n"
            "\n";
          fprintf (stdout, "%s", str);
          exit(0);
        }
      else if (!strcmp (OPT_ARG_VERSION, "v"))
        {
          const char str[] =
            "gnutls-cli-debug 3.7.4\n";
          fprintf (stdout, "%s", str);
          exit(0);
        }
      else if (!strcmp (OPT_ARG_VERSION, "n"))
        {
          const char str[] =
            "gnutls-cli-debug 3.7.4\n"
            "Copyright (C) 2024 Daiki Ueno\n"
            "This is free software. It is licensed for use, modification and\n"
            "redistribution under the terms of the GNU General Public License,\n"
            "version 3 or later <http://gnu.org/licenses/gpl.html>\n"
            "\n"
            "GnuTLS is free software: you can redistribute it and/or\n"
            "modify it under the terms of the GNU General Public License\n"
            "as published by the Free Software Foundation,\n"
            "either version 3 of the License, or (at your option) any later version.\n"
            "\n"
            "GnuTLS is distributed in the hope that it will be useful,\n"
            "but WITHOUT ANY WARRANTY; without even the implied warranty\n"
            "of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n"
            "See the GNU General Public License for more details.\n"
            "\n"
            "You should have received a copy of the GNU General Public License\n"
            "along with this program.  If not, see <http://www.gnu.org/licenses/>.\n"
            "\n";
          fprintf (stdout, "%s", str);
          exit(0);
        }
      else
        {
          fprintf (stderr,
                   "version option argument 'a' invalid.  Use:\n"
                   "	'v' - version only\n"
                   "	'c' - version and copyright\n"
                   "	'n' - version and full copyright notice\n");
          exit (EXIT_FAILURE);
        }
    }

  return optind;
}

void
usage (FILE *out, int status)
{
  const char str[] =
    "gnutls-cli-debug - GnuTLS debug client\n"
    "Usage:  gnutls-cli-debug [ -<flag> [<val>] | --<name>[{=| }<val>] ]... [hostname]\n"
    "\n"
    "None:\n"
    "\n"
    "   -d, --debug=num            Enable debugging\n"
    "				- it must be in the range:\n"
    "				  0 to 9999\n"
    "   -V, --verbose              More verbose output\n"
    "   -p, --port=num             The port to connect to\n"
    "				- it must be in the range:\n"
    "				  0 to 65536\n"
    "       --app-proto            an alias for the 'starttls-proto' option\n"
    "       --starttls-proto=str   The application protocol to be used to obtain the server's certificate (https, ftp, smtp, imap, ldap, xmpp, lmtp, pop3, nntp, sieve, postgres)\n"
    "\n"
    "Version, usage and configuration options:\n"
    "\n"
    "   -v, --version[=arg]        output version information and exit\n"
    "   -h, --help                 display extended usage information and exit\n"
    "   -!, --more-help            extended usage information passed thru pager\n"
    "\n"
    "Options are specified by doubled hyphens and their name or by a single\n"
    "hyphen and the flag character.\n"
    "Operands and options may be intermixed.  They will be reordered.\n"
    "\n"
    "TLS debug client. It sets up multiple TLS connections to \n"
    "a server and queries its capabilities. It was created to assist in debugging \n"
    "GnuTLS, but it might be useful to extract a TLS server's capabilities.\n"
    "It connects to a TLS server, performs tests and print the server's \n"
    "capabilities. If called with the `-V' parameter more checks will be performed.\n"
    "Can be used to check for servers with special needs or bugs.\n"
    "\n";
  fprintf (out, "%s", str);
  exit (status);
}
