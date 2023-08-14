/*
 * Copyright 2022 Nico Sonack <nsonack@herrhotzenplotz.de>
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

#include <gcli/colour.h>
#include <gcli/config.h>
#include <gcli/forges.h>
#include <gcli/labels.h>
#include <gcli/table.h>

int
gcli_get_labels(gcli_ctx *ctx, char const *owner, char const *reponame,
                int const max, gcli_label_list *const out)
{
	return gcli_forge(ctx)->get_labels(ctx, owner, reponame, max, out);
}

void
gcli_free_label(gcli_label *const label)
{
	free(label->name);
	free(label->description);
}

void
gcli_free_labels(gcli_label_list *const list)
{
	for (size_t i = 0; i < list->labels_size; ++i)
		gcli_free_label(&list->labels[i]);
	free(list->labels);

	list->labels = NULL;
	list->labels_size = 0;
}

void
gcli_print_labels(gcli_ctx *ctx, gcli_label_list const *const list,
                  int const max)
{
	size_t n;
	gcli_tbl table;
	gcli_tblcoldef cols[] = {
		{ .name = "ID",          .type = GCLI_TBLCOLTYPE_LONG,   .flags = GCLI_TBLCOL_JUSTIFYR },
		{ .name = "",            .type = GCLI_TBLCOLTYPE_STRING, .flags = GCLI_TBLCOL_256COLOUR|GCLI_TBLCOL_TIGHT },
		{ .name = "NAME",        .type = GCLI_TBLCOLTYPE_STRING, .flags = 0 },
		{ .name = "DESCRIPTION", .type = GCLI_TBLCOLTYPE_STRING, .flags = 0 },
	};

	/* Determine number of items to print */
	if (max < 0 || (size_t)(max) > list->labels_size)
		n = list->labels_size;
	else
		n = max;

	/* Fill table */
	table = gcli_tbl_begin(ctx, cols, ARRAY_SIZE(cols));
	if (!table)
		errx(1, "error: could not init table");

	for (size_t i = 0; i < n; ++i) {
		gcli_tbl_add_row(table,
		                 (long)list->labels[i].id, /* Cast is important here (#165) */
		                 list->labels[i].colour,
		                 gcli_config_have_colours(ctx) ? "  " : "",
		                 list->labels[i].name,
		                 list->labels[i].description);
	}

	gcli_tbl_end(table);
}

int
gcli_create_label(gcli_ctx *ctx, char const *owner, char const *repo,
                  gcli_label *const label)
{
	return gcli_forge(ctx)->create_label(ctx, owner, repo, label);
}

int
gcli_delete_label(gcli_ctx *ctx, char const *owner, char const *repo,
                  char const *const label)
{
	return gcli_forge(ctx)->delete_label(ctx, owner, repo, label);
}
