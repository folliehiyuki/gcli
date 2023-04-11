/*
 * Copyright 2021, 2022 Nico Sonack <nsonack@herrhotzenplotz.de>
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

#ifndef PULLS_H
#define PULLS_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdbool.h>

#include <sn/sn.h>
#include <gcli/gcli.h>

typedef struct gcli_pull                gcli_pull;
typedef struct gcli_submit_pull_options gcli_submit_pull_options;
typedef struct gcli_commit              gcli_commit;
typedef struct gcli_pull_list           gcli_pull_list;

struct gcli_pull_list {
	gcli_pull *pulls;
	size_t pulls_size;
};

struct gcli_pull {
	char   *author;
	char   *state;
	char   *title;
	char   *body;
	char   *created_at;
	char   *commits_link;
	char   *head_label;
	char   *base_label;
	char   *head_sha;
	int     id;
	int     number;
	int     comments;
	int     additions;
	int     deletions;
	int     commits;
	int     changed_files;
	int     head_pipeline_id;   /* This is GitLab specific */
	sn_sv  *labels;
	size_t  labels_size;
	bool    merged;
	bool    mergeable;
	bool    draft;
};

struct gcli_commit {
	char const *sha, *message, *date, *author, *email;
};

/* Options to submit to the gh api for creating a PR */
struct gcli_submit_pull_options {
	char const  *owner;
	char const  *repo;
	sn_sv        from;
	sn_sv        to;
	sn_sv        title;
	sn_sv        body;
	char const **labels;
	size_t       labels_size;
	int          draft;
	bool         always_yes;
};

int gcli_get_pulls(char const *owner,
                   char const *reponame,
                   bool all,
                   int max,
                   gcli_pull_list *out);

void gcli_pull_free(gcli_pull *it);

void gcli_pulls_free(gcli_pull_list *list);

void gcli_print_pulls_table(enum gcli_output_flags flags,
                            gcli_pull_list const *list,
                            int max);

void gcli_print_pull_diff(FILE *stream,
                          char const *owner,
                          char const *reponame,
                          int pr_number);

void gcli_pull_checks(char const *owner,
                      char const *repo,
                      int pr_number);

void gcli_pull_commits(char const *owner,
                       char const *repo,
                       int pr_number);

void gcli_get_pull(char const *owner,
                   char const *repo,
                   int pr_number,
                   gcli_pull *out);

void gcli_pull_submit(gcli_submit_pull_options);

void gcli_pull_print_status(gcli_pull const *it);

void gcli_pull_print_op(gcli_pull *summary);

enum gcli_merge_flags {
	GCLI_PULL_MERGE_SQUASH = 0x1, /* squash commits when merging */
	GCLI_PULL_MERGE_DELETEHEAD = 0x2, /* delete the source branch after merging */
};

void gcli_pull_merge(char const *owner,
                     char const *reponame,
                     int pr_number,
                     enum gcli_merge_flags flags);

void gcli_pull_close(char const *owner,
                     char const *reponame,
                     int pr_number);

void gcli_pull_reopen(char const *owner,
                      char const *reponame,
                      int pr_number);

void gcli_pull_add_labels(char const *owner,
                          char const *repo,
                          int pr_number,
                          char const *const labels[],
                          size_t labels_size);

void gcli_pull_remove_labels(char const *owner,
                             char const *repo,
                             int pr_number,
                             char const *const labels[],
                             size_t labels_size);

#endif /* PULLS_H */
