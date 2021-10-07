/*
 * Copyright 2021 Nico Sonack <nsonack@outlook.com>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following
 * disclaimer in the documentation and/or other materials provided
 * with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <ghcli/issues.h>
#include <ghcli/pulls.h>
#include <ghcli/comments.h>
#include <ghcli/gitconfig.h>

#include <sn/sn.h>

static char *
shift(int *argc, char ***argv)
{
    if (*argc == 0)
        errx(1, "Not enough arguments");

    (*argc)--;
    return *((*argv)++);
}

static int
subcommand_pulls(int argc, char *argv[])
{
    ghcli_pull *pulls      = NULL;
    int         pulls_size = 0;

    if (argc == 2) {
        pulls_size = ghcli_get_prs(argv[0], argv[1], &pulls);
        ghcli_print_pr_table(stdout, pulls, pulls_size);

        return EXIT_SUCCESS;
    }

    if (argc < 3)
        errx(1, "Need org and repo name (and PR number) to operate on");

    const char *org    = shift(&argc, &argv);
    const char *repo   = shift(&argc, &argv);
    const char *_pr    = shift(&argc, &argv);
    char       *endptr = NULL;

    int pr = strtoul(_pr, &endptr, 10);
    if (endptr != (_pr + strlen(_pr)))
        err(1, "cannot parse pr number »%s«", _pr);

    if (pr <= 0)
        errx(1, "pr number is out of range");

    while (argc > 0) {
        const char *operation = shift(&argc, &argv);

        if (strcmp(operation, "diff") == 0)
            ghcli_print_pr_diff(stdout, org, repo, pr);
        else if (strcmp(operation, "summary") == 0)
            ghcli_pr_summary(stdout, org, repo, pr);
        else if (strcmp(operation, "comments") == 0)
            ghcli_pr_comments(stdout, org, repo, pr);
        else
            errx(1, "unknown operation %s", operation);
    }

    return EXIT_SUCCESS;
}

static int
subcommand_issues(int argc, char *argv[])
{
    ghcli_issue *issues      = NULL;
    int          issues_size = 0;

    (void) argc;

    issues_size = ghcli_get_issues(argv[0], argv[1], &issues);

    ghcli_print_issues_table(stdout, issues, issues_size);

    return EXIT_SUCCESS;
}

int
main(int argc, char *argv[])
{
    shift(&argc, &argv);

    const char *foo = ghcli_find_gitconfig();

    const char *subcommand = shift(&argc, &argv);

    if (strcmp(subcommand, "pulls") == 0)
        return subcommand_pulls(argc, argv);
    else if (strcmp(subcommand, "issues") == 0)
        return subcommand_issues(argc, argv);
    else
        errx(1, "unknown subcommand %s", subcommand);

    return 42;
}
