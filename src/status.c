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

#include <gcli/status.h>
#include <gcli/forges.h>

void
gcli_status(gcli_ctx *ctx, int const count)
{
	gcli_notification *notifications = NULL;
	int notifications_size = 0;

	notifications_size = gcli_get_notifications(ctx, &notifications, count);
	if (notifications_size < 0)
		errx(1, "error: failed to get notifications");

	if (count < 0) {
		gcli_print_notifications(notifications, notifications_size);
	} else {
		gcli_print_notifications(
			notifications,
			(size_t)(count < (int)notifications_size
			         ? count : notifications_size));
	}

	gcli_free_notifications(notifications, notifications_size);
}

int
gcli_get_notifications(gcli_ctx *ctx, gcli_notification **const out,
                       int const count)
{
	return gcli_forge(ctx)->get_notifications(ctx, out, count);
}

void
gcli_free_notifications(gcli_notification *notifications,
                        size_t const notifications_size)
{
	for (size_t i = 0; i < notifications_size; ++i) {
		free(notifications[i].id);
		free(notifications[i].title);
		free(notifications[i].reason);
		free(notifications[i].date);
		free(notifications[i].type);
		free(notifications[i].repository);
	}

	free(notifications);
}

void
gcli_print_notifications(gcli_notification const *const notifications,
                         size_t const notifications_size)
{
	for (size_t i = 0; i < notifications_size; ++i) {
		printf("%s - %s - %s - %s - %s\n",
		       notifications[i].id, notifications[i].repository,
		       notifications[i].type, notifications[i].date,
		       notifications[i].reason);

		pretty_print(notifications[i].title, 4, 80, stdout);
		putchar('\n');
	}
}

int
gcli_notification_mark_as_read(gcli_ctx *ctx, char const *id)
{
	return gcli_forge(ctx)->notification_mark_as_read(ctx, id);
}
