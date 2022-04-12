#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "user_op.h"
#include "library_op.h"


// Adds user in library database
void
add_user(hashtable_t *usr_table, char *name)
{
	if (ht_has_key(usr_table, name)) {
		ALREADY_REG
		return;
	}

	user_data_t usr_data;
	usr_data.b_book = NULL;
	usr_data.is_banned = 0;
	usr_data.score = 100;
	usr_data.days = -1;

	ht_put(usr_table, name, strlen(name) + 1, &usr_data, sizeof(usr_data), LOAD_F);
}

// Registers the act of borrowing
// Book becomes unavailable
void
borrow_book(hashtable_t *usr_table, hashtable_t *lib , char *name, char *b_name, int days)
{
	user_data_t *usr_data = ht_get(usr_table, name);
	if (!usr_data) {
		NOT_REG
		return;
	}
	if (usr_data->is_banned) {
		BANNED
		return;
	}

	book_info_t *b_data = ht_get(lib, b_name);
	if (!b_data) {
		BOOK_NOT_FOUND
		return;
	}
	if (b_data->barrowed) {
		BORROWED
		return;
	}

	if (usr_data->b_book) {
		ALREADY_BWD
		return;
	}

	b_data->barrowed = 1;
	usr_data->b_book = strdup(name);
	usr_data->days = days;
}

// Registers returning a book
// The score of a user is changed based on his punctuality
// Book rating is updated
void
return_book(hashtable_t *usr_table, hashtable_t *lib, char *name, char *b_name, int days, double rating)
{
	user_data_t *usr_data = ht_get(usr_table, name);
	if (!usr_data) {
		NOT_REG
		return;
	}
	if (usr_data->is_banned) {
		BANNED
		return;
	}

	if (!(usr_data->b_book) || strcmp(usr_data->b_book, b_name)) {
		NOT_BWD_BOOK
		return;
	}

	book_info_t *b_data = ht_get(lib, b_name);
	if (!b_data) {
		BOOK_NOT_FOUND
		return;
	}

	int return_delay = usr_data->days - days;
	if (return_delay > 0) {
		usr_data->score += 2 * return_delay;
	} else {
		usr_data->score += return_delay;
	}

	if (usr_data->score < 0) {
		USER_BANNED(name)
		usr_data->is_banned = 1;
	}

	free(usr_data->b_book);
	usr_data->b_book = NULL;
	usr_data->days = -1;
	
	b_data->purchases++;
	b_data->rating = (b_data->rating + rating) / 2;
	b_data->barrowed = 0;
}

// Registers losing a book
// The book is removed from the lib and user loses score
void
lost_book(hashtable_t *usr_table, hashtable_t *lib, char *name, char *b_name)
{
	user_data_t *usr_data = ht_get(usr_table, name);
	if (!usr_data) {
		NOT_REG
		return;
	}
	if (usr_data->is_banned) {
		BANNED
		return;
	}

	rmv_book(lib, b_name);

	usr_data->score -= 20;
	if (usr_data->score < 0) {
		USER_BANNED(name)
		usr_data->is_banned = 1;
	}

	free(usr_data->b_book);
	usr_data->b_book = NULL;
	usr_data->days = -1;
}

// Compares users by score and if needed by name
int
compare_users(info_t *usr_1, info_t *usr_2)
{	
	int diff = ((user_data_t *)usr_1->value)->score - ((user_data_t *)usr_2->value)->score;
	if (diff)
		return diff;
	return strcmp((char *)usr_1->key, (char *)usr_2->key);
}