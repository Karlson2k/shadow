/*
 * SPDX-FileCopyrightText: 1992 - 1994, Julianne Frances Haugh
 * SPDX-FileCopyrightText: 1996 - 2000, Marek Michałkiewicz
 * SPDX-FileCopyrightText: 2003 - 2006, Tomasz Kłoczko
 * SPDX-FileCopyrightText: 2008 - 2009, Nicolas François
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "config.h"

#ifndef USE_PAM
#ident "$Id$"

#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

#include "agetpass.h"
#include "defines.h"
#include "prototypes.h"
#include "pwauth.h"
#include "getdef.h"
#include "string/memset/memzero.h"
#include "string/sprintf/snprintf.h"
#include "string/strcmp/streq.h"

#ifdef SKEY
#include <skey.h>
#endif


#ifdef __linux__		/* standard password prompt by default */
static const char *PROMPT = gettext_noop ("Password: ");
#else
static const char *PROMPT = gettext_noop ("%s's Password: ");
#endif


/*
 * pw_auth - perform getpass/crypt authentication
 *
 *	pw_auth gets the user's cleartext password and encrypts it
 *	using the salt in the encrypted password. The results are
 *	compared.
 */

int
pw_auth(const char *cipher, const char *user)
{
	int          retval;
	char         prompt[1024];
	char         *clear;
	const char   *cp;
	const char   *encrypted;
	const char   *input;

#ifdef	SKEY
	bool         use_skey = false;
	char         challenge_info[40];
	struct skey  skey;
#endif

	/*
	 * WARNING:
	 *
	 * When we are logging in a user with no ciphertext password,
	 * we don't prompt for the password or anything.  In reality
	 * the user could just hit <ENTER>, so it doesn't really
	 * matter.
	 */

	if ((NULL == cipher) || streq(cipher, "")) {
		return 0;
	}

#ifdef	SKEY
	/*
	 * If the user has an S/KEY entry show them the pertinent info
	 * and then we can try validating the created ciphertext and the SKEY.
	 * If there is no SKEY information we default to not using SKEY.
	 */

# ifdef SKEY_BSD_STYLE
	/*
	 * Some BSD updates to the S/KEY API adds a fourth parameter; the
	 * sizeof of the challenge info buffer.
	 */
#  define skeychallenge(s,u,c) skeychallenge(s,u,c,sizeof(c))
# endif

	if (skeychallenge (&skey, user, challenge_info) == 0) {
		use_skey = true;
	}
#endif

	/*
	 * Prompt for the password as required.
	 */

	cp = getdef_str ("LOGIN_STRING");
	if (NULL == cp) {
		cp = _(PROMPT);
	}
#ifdef	SKEY
	if (use_skey) {
		printf ("[%s]\n", challenge_info);
	}
#endif

	SNPRINTF(prompt, cp, user);
	clear = agetpass(prompt);
	input = (clear == NULL) ? "" : clear;

	/*
	 * Convert the cleartext password into a ciphertext string.
	 * If the two match, the return value will be zero, which is
	 * SUCCESS. Otherwise we see if SKEY is being used and check
	 * the results there as well.
	 */

	encrypted = pw_encrypt (input, cipher);
	if (NULL != encrypted) {
		retval = strcmp (encrypted, cipher);
	} else {
		retval = -1;
	}

#ifdef  SKEY
	/*
	 * If (1) The password fails to match, and
	 * (2) The password is empty and
	 * (3) We are using OPIE or S/Key, then
	 * ...Re-prompt, with echo on.
	 * -- AR 8/22/1999
	 */
	if ((0 != retval) && streq(input, "") && use_skey) {
		erase_pass(clear);
		clear = agetpass(prompt);
		input = (clear == NULL) ? "" : clear;
	}

	if ((0 != retval) && use_skey) {
		int passcheck = -1;

		if (skeyverify (&skey, input) == 0) {
			passcheck = skey.n;
		}
		if (passcheck > 0) {
			retval = 0;
		}
	}
#endif
	erase_pass(clear);

	return retval;
}
#else				/* !USE_PAM */
extern int ISO_C_forbids_an_empty_translation_unit;
#endif				/* !USE_PAM */
