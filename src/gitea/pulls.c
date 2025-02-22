/*
 * Copyright 2022-2024 Nico Sonack <nsonack@herrhotzenplotz.de>
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

#include <gcli/gitea/pulls.h>
#include <gcli/gitea/repos.h>
#include <gcli/github/issues.h>
#include <gcli/github/pulls.h>

#include <gcli/json_gen.h>

#include <templates/github/pulls.h>

#include <stdarg.h>

int
gitea_pull_make_url(struct gcli_ctx *ctx, struct gcli_path const *const path,
                    char **url, char const *const suffix_fmt, ...)
{
	char *suffix = NULL;
	int rc = 0;
	va_list vp;

	va_start(vp, suffix_fmt);
	suffix = sn_vasprintf(suffix_fmt, vp);
	va_end(vp);

	switch (path->kind) {
	case GCLI_PATH_DEFAULT: {
		char *e_owner = NULL, *e_repo = NULL;

		e_owner = gcli_urlencode(path->data.as_default.owner);
		e_repo  = gcli_urlencode(path->data.as_default.repo);

		*url = sn_asprintf("%s/repos/%s/%s/pulls/%"PRIid"%s",
		                   gcli_get_apibase(ctx), e_owner, e_repo,
		                   path->data.as_default.id, suffix);

		free(e_owner);
		free(e_repo);
	} break;
	case GCLI_PATH_URL: {
		*url = sn_asprintf("%s%s", path->data.as_url, suffix);
	} break;
	default: {
		rc = gcli_error(ctx, "unsupported path kind for Gitea pulls");
	} break;
	}

	free(suffix);

	return rc;
}

int
gitea_search_pulls(struct gcli_ctx *ctx, struct gcli_path const *const path,
                   struct gcli_pull_fetch_details const *details,
                   int const max, struct gcli_pull_list *const out)
{
	char *url = NULL, *e_author = NULL, *e_label = NULL,
	     *e_milestone = NULL, *e_query = NULL;
	int rc = 0;

	struct gcli_fetch_list_ctx fl = {
		.listp = &out->pulls,
		.sizep = &out->pulls_size,
		.parse = (parsefn)(parse_github_pulls),
		.max = max,
	};

	if (details->milestone) {
		char *tmp = gcli_urlencode(details->milestone);
		e_milestone = sn_asprintf("&milestones=%s", tmp);
		free(tmp);
	}

	if (details->author) {
		char *tmp = gcli_urlencode(details->author);
		e_author = sn_asprintf("&created_by=%s", tmp);
		free(tmp);
	}

	if (details->label) {
		char *tmp = gcli_urlencode(details->label);
		e_label = sn_asprintf("&labels=%s", tmp);
		free(tmp);
	}

	if (details->search_term) {
		char *tmp = gcli_urlencode(details->search_term);
		e_query = sn_asprintf("&q=%s", tmp);
		free(tmp);
	}

	rc = gitea_repo_make_url(ctx, path, &url,
	                         "/issues?type=pulls&state=%s%s%s%s%s",
	                         details->all ? "all" : "open",
	                         e_author ? e_author : "",
	                         e_label ? e_label : "",
	                         e_milestone ? e_milestone : "",
	                         e_query ? e_query : "");

	free(e_query);
	free(e_milestone);
	free(e_author);
	free(e_label);

	if (rc < 0)
		return rc;

	return gcli_fetch_list(ctx, url, &fl);
}

int
gitea_get_pull(struct gcli_ctx *ctx, struct gcli_path const *const path,
               struct gcli_pull *const out)
{
	int const rc = github_get_pull(ctx, path, out);

	if (rc == 0) {
		/* fix for https://gitlab.com/herrhotzenplotz/gcli/issues/222:
		 *
		 * Sometimes the "reviewers" array contains a single NULL value
		 * that means that this pull request is to be reviewed by the
		 * owners of the repository. */
		for (size_t i = 0; i < out->reviewers_size; ++i) {
			if (out->reviewers[i] == NULL)
				out->reviewers[i] = strdup("Owners");
		}
	}

	return rc;
}

int
gitea_get_pull_commits(struct gcli_ctx *ctx,
                       struct gcli_path const *const path,
                       struct gcli_commit_list *const out)
{
	return github_get_pull_commits(ctx, path, out);
}

int
gitea_pull_submit(struct gcli_ctx *ctx, struct gcli_submit_pull_options *opts)
{
	warnx("In case the following process errors out, see: "
	      "https://github.com/go-gitea/gitea/issues/20175");
	return github_perform_submit_pull(ctx, opts);
}

int
gitea_pull_merge(struct gcli_ctx *ctx, struct gcli_path const *const path,
                 enum gcli_merge_flags const flags)
{
	bool const delete_branch = flags & GCLI_PULL_MERGE_DELETEHEAD;
	bool const squash = flags & GCLI_PULL_MERGE_SQUASH;
	char *url = NULL, *payload = NULL;
	struct gcli_jsongen gen = {0};
	int rc = 0;

	/* Generate URL */
	rc = gitea_pull_make_url(ctx, path, &url, "/merge");
	if (rc < 0)
		return rc;

	/* Generate payload */
	gcli_jsongen_init(&gen);
	gcli_jsongen_begin_object(&gen);
	{
		gcli_jsongen_objmember(&gen, "Do");
		gcli_jsongen_string(&gen, squash ? "squash" : "merge");

		gcli_jsongen_objmember(&gen, "delete_branch_after_merge");
		gcli_jsongen_bool(&gen, delete_branch);
	}
	gcli_jsongen_end_object(&gen);

	payload = gcli_jsongen_to_string(&gen);
	gcli_jsongen_free(&gen);

	rc = gcli_fetch_with_method(ctx, "POST", url, payload, NULL, NULL);

	free(url);
	free(payload);

	return rc;
}

static int
gitea_pulls_patch_state(struct gcli_ctx *ctx,
                        struct gcli_path const *const path, char const *state)
{
	char *url = NULL;
	char *data = NULL;
	int rc = 0;

	rc = gitea_pull_make_url(ctx, path, &url, "");
	if (rc < 0)
		return rc;

	data = sn_asprintf("{ \"state\": \"%s\"}", state);

	rc = gcli_fetch_with_method(ctx, "PATCH", url, data, NULL, NULL);

	free(data);
	free(url);

	return rc;
}

int
gitea_pull_close(struct gcli_ctx *ctx, struct gcli_path const *const path)
{
	return gitea_pulls_patch_state(ctx, path, "closed");
}

int
gitea_pull_reopen(struct gcli_ctx *ctx, struct gcli_path const *const path)
{
	return gitea_pulls_patch_state(ctx, path, "open");
}

int
gitea_pull_get_patch(struct gcli_ctx *ctx, FILE *const stream,
                     struct gcli_path const *const path)
{
	char *url = NULL;
	int rc = 0;

	rc = gitea_pull_make_url(ctx, path, &url, ".patch");
	if (rc < 0)
		return rc;

	rc = gcli_curl(ctx, stream, url, NULL);

	free(url);

	return rc;
}
int
gitea_pull_get_diff(struct gcli_ctx *ctx, FILE *const stream,
                    struct gcli_path const *const path)
{
	char *url = NULL;
	int rc = 0;

	rc = gitea_pull_make_url(ctx, path, &url, ".diff");
	if (rc < 0)
		return rc;

	rc = gcli_curl(ctx, stream, url, NULL);

	free(url);

	return rc;
}

int
gitea_pull_get_checks(struct gcli_ctx *ctx, struct gcli_path const *const path,
                      struct gcli_pull_checks_list *out)
{
	(void) ctx;
	(void) path;
	(void) out;

	return gcli_error(ctx, "Pull Request checks are not available on Gitea");
}

int
gitea_pull_set_milestone(struct gcli_ctx *ctx,
                         struct gcli_path const *const pull_path,
                         gcli_id milestone_id)
{
	return github_issue_set_milestone(ctx, pull_path, milestone_id);
}

int
gitea_pull_clear_milestone(struct gcli_ctx *ctx,
                           struct gcli_path const *const pull_path)
{
	/* NOTE: The github routine for clearing issues sets the milestone
	 * to null (not the integer zero). However this does not work in
	 * the case of Gitea which clear the milestone by setting it to
	 * the integer value zero. */
	return github_issue_set_milestone(ctx, pull_path, 0);
}

int
gitea_pull_add_reviewer(struct gcli_ctx *ctx,
                        struct gcli_path const *const path,
                        char const *username)
{
	return github_pull_add_reviewer(ctx, path, username);
}

int
gitea_pull_set_title(struct gcli_ctx *ctx, struct gcli_path const *const path,
                     char const *const title)
{
	return github_pull_set_title(ctx, path, title);
}
