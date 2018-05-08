/**
 * @author Marius Orcsik <marius@habarnam.ro>
 */
#ifndef MPRIS_SCROBBLER_SCROBBLE_H
#define MPRIS_SCROBBLER_SCROBBLE_H

#include <assert.h>
#include <time.h>

#define NOW_PLAYING_DELAY 65 //seconds
#define MIN_TRACK_LENGTH  30 // seconds
#define MPRIS_SPOTIFY_TRACK_ID_PREFIX                          "spotify:track:"

char *get_player_namespace(DBusConnection *);
void get_mpris_properties(DBusConnection*, const char*, struct mpris_properties*, struct mpris_event*);

struct dbus *dbus_connection_init(struct state*);
void state_loaded_properties(struct state*, struct mpris_properties*, const struct mpris_event*);

#if 0
const char *get_api_status_label (api_return_codes code)
{
    switch (code) {
        case unavaliable:
            return "The service is temporarily unavailable, please try again.";
        case invalid_service:
            return "Invalid service - This service does not exist";
        case invalid_method:
            return "Invalid Method - No method with that name in this package";
        case authentication_failed:
            return "Authentication Failed - You do not have permissions to access the service";
        case invalid_format:
            return "Invalid format - This service doesn't exist in that format";
        case invalid_parameters:
            return "Invalid parameters - Your request is missing a required parameter";
        case invalid_resource:
            return "Invalid resource specified";
        case operation_failed:
            return "Operation failed - Something else went wrong";
        case invalid_session_key:
            return "Invalid session key - Please re-authenticate";
        case invalid_apy_key:
            return "Invalid API key - You must be granted a valid key by last.fm";
        case service_offline:
            return "Service Offline - This service is temporarily offline. Try again later.";
        case invalid_signature:
            return "Invalid method signature supplied";
        case temporary_error:
            return "There was a temporary error processing your request. Please try again";
        case suspended_api_key:
            return "Suspended API key - Access for your account has been suspended, please contact Last.fm";
        case rate_limit_exceeded:
            return "Rate limit exceeded - Your IP has made too many requests in a short period";
    }
    return "Unkown";
}
#endif

static void scrobble_init(struct scrobble *s)
{
    if (NULL == s) { return; }
    s->scrobbled = false;
    s->length = 0;
    s->position = 0;
    s->start_time = 0;
    s->track_number = 0;

    s->title = get_zero_string(MAX_PROPERTY_LENGTH);
    s->album = get_zero_string(MAX_PROPERTY_LENGTH);
    s->artist = NULL;

    s->mb_track_id = get_zero_string(MAX_PROPERTY_LENGTH);
    s->mb_artist_id = get_zero_string(MAX_PROPERTY_LENGTH);
    s->mb_album_id = get_zero_string(MAX_PROPERTY_LENGTH);
    s->mb_album_artist_id = get_zero_string(MAX_PROPERTY_LENGTH);
    s->mb_spotify_id = get_zero_string(MAX_PROPERTY_LENGTH);

    _trace2("mem::inited_scrobble(%p)", s);
}

struct scrobble *scrobble_new(void)
{
    struct scrobble *result = malloc(sizeof(struct scrobble));
    scrobble_init(result);

    return result;
}

void scrobble_free(struct scrobble *s)
{
    if (NULL == s) { return; }

    if (NULL != s->title) {
        if (strlen(s->title) > 0) { _trace2("mem::scrobble::free::title(%p): %s", s->title, s->title); }
        free(s->title);
        s->title = NULL;
    }
    if (NULL != s->album) {
        if (strlen(s->album) > 0) { _trace2("mem::scrobble::free::album(%p): %s", s->album, s->album); }
        free(s->album);
        s->album = NULL;
    }
    if (NULL != s->artist) {
        sb_arr_free(&s->artist);
    }
    if (NULL != s->mb_track_id) {
        if (NULL != s->mb_track_id) {
            if (strlen(s->mb_track_id) > 0) { _trace2("mem::scrobble::musicbrainz::free::track_id(%p): %s", s->mb_track_id, s->mb_track_id); }
            free(s->mb_track_id);
            s->mb_track_id = NULL;
        }
        if (NULL != s->mb_album_id) {
            if (strlen(s->mb_album_id) > 0) { _trace2("mem::scrobble::musicbrainz::free::album_id(%p): %s", s->mb_album_id, s->mb_album_id); }
            free(s->mb_album_id);
            s->mb_album_id = NULL;
        }
        if (NULL != s->mb_artist_id) {
            if (strlen(s->mb_artist_id) > 0) { _trace2("mem::scrobble::musicbrainz::free::artist_id(%p): %s", s->mb_artist_id, s->mb_artist_id); }
            free(s->mb_artist_id);
            s->mb_artist_id = NULL;
        }
        if (NULL != s->mb_album_artist_id) {
            if (strlen(s->mb_album_artist_id) > 0) { _trace2("mem::scrobble::musicbrainz::free::album_artist_id(%p): %s", s->mb_album_artist_id, s->mb_album_artist_id); }
            free(s->mb_album_artist_id);
            s->mb_album_artist_id = NULL;
        }
        if (NULL != s->mb_spotify_id) {
            if (strlen(s->mb_spotify_id) > 0) { _trace2("mem::scrobble::musicbrainz::free::spotify_id(%p): %s", s->mb_spotify_id, s->mb_spotify_id); }
            free(s->mb_spotify_id);
            s->mb_spotify_id = NULL;
        }
    }

    _trace2("mem::free::scrobble(%p)", s);
    free(s);
}

void scrobbles_free(struct scrobble **tracks[], bool free_buff)
{
    if (NULL == tracks) { return; }

    int track_count = sb_count(*tracks);
    if (track_count == 0) { goto _free_buffer; }

    for (int i = 0; i < track_count; i++) {
        if ( NULL == (*tracks)[i]) { continue; }

        _trace("scrobbler::freeing_track(%zu:%p) ", i, (*tracks)[i]);
        scrobble_free((*tracks)[i]);
        (*tracks)[i] = NULL;
        (void)sb_add(*tracks, (-1));
    }
    assert(sb_count(*tracks) == 0);
_free_buffer:
    if (free_buff) { sb_free(*tracks); }
}

#if 0
static bool scrobbles_remove(struct mpris_properties *queue[], size_t queue_length, size_t pos)
{
    struct mpris_properties *last = queue[pos];
    if (NULL == last) { return false; }
    struct mpris_metadata *d = last->metadata;
    _trace("scrobbler::remove_scrobble(%p//%u) %s//%s//%s", last, pos, d->title, d->artist, d->album);
    if (pos >= queue_length) { return queue_length; }

    mpris_properties_free(last);
    queue[pos] = NULL;

    //player->previous = player->queue[pos-1];
    return true;
}
#endif

static void scrobble_init(struct scrobble*);
void scrobbles_free(struct scrobble***, bool);
static void mpris_player_free(struct mpris_player *player)
{
    if (NULL == player) { return; }

    if (NULL != player->queue) { scrobbles_free(&player->queue, true); }
    if (NULL != player->mpris_name) { free(player->mpris_name); }
    if (NULL != player->properties) { mpris_properties_free(player->properties); }
    if (NULL != player->current) { mpris_properties_free(player->current); }
    if (NULL != player->changed) { free(player->changed); }

    free (player);
}

void events_free(struct events*);
void dbus_close(struct state*);
void scrobbler_free(struct scrobbler*);
void state_free(struct state *s)
{
    if (NULL != s->dbus) { dbus_close(s); }
    if (NULL != s->events) { events_free(s->events); }
    if (NULL != s->player) { mpris_player_free(s->player); }
    if (NULL != s->scrobbler) { scrobbler_free(s->scrobbler); }

    free(s);
}

static struct mpris_player *mpris_player_new(void)
{
    struct mpris_player *result = calloc(1, sizeof(struct mpris_player));

    return (result);
}

static void mpris_player_init(struct mpris_player *player, DBusConnection *conn)
{
    player->mpris_name = NULL;
    player->changed = calloc(1, sizeof(struct mpris_event));
    player->queue = NULL;

    if (NULL != conn) {
        player->properties = mpris_properties_new();
        _trace("mem::player::inited_properties(%p)", player->properties);
        player->current = mpris_properties_new();
        _trace("mem::player::inited_current(%p)", player->current);

        player->mpris_name = get_player_namespace(conn);
        if (NULL != player->mpris_name) {
            get_mpris_properties(conn, player->mpris_name, player->properties, player->changed);
        }
    }
    _trace("mem::player::inited(%p)", player);
}

struct scrobbler *scrobbler_new(void);
struct events *events_new(void);
void events_init(struct events*, struct sighandler_payload*);
void scrobbler_init(struct scrobbler*, struct configuration*, struct events*);
bool state_init(struct state *s, struct sighandler_payload *sig_data)
{
    _trace2("mem::initing_state(%p)", s);
    s->scrobbler = scrobbler_new();
    s->player = mpris_player_new();

    s->events = events_new();
    events_init(s->events, sig_data);
    s->dbus = dbus_connection_init(s);
    scrobbler_init(s->scrobbler, sig_data->config, s->events);

    if (NULL == s->dbus) { return false; }
    mpris_player_init(s->player, s->dbus->conn);
    _trace2("mem::inited_state(%p)", s);

    state_loaded_properties(s, s->player->properties, s->player->changed);

    return true;
}

struct state *state_new(void)
{
    struct state *result = malloc(sizeof(struct state));
    return result;
}

static bool scrobble_is_valid(const struct scrobble *s)
{
    if (NULL == s) { return false; }
    if (NULL == s->title) { return false; }
    if (NULL == s->album) { return false; }
    if (sb_count(s->artist) == 0 || NULL == s->artist[0]) { return false; }
#if 0
    _trace("Checking playtime: %u - %u", s->play_time, (s->length / 2));
    _trace("Checking scrobble:\n" \
        "title: %s\n" \
        "artist: %s\n" \
        "album: %s\n" \
        "length: %u\n" \
        "play_time: %u\n", s->title, s->artist, s->album, s->length, s->play_time);
#endif
    return (
        s->length >= MIN_TRACK_LENGTH &&
        //s->play_time >= (s->length / 2) &&
        s->scrobbled == false &&
        strlen(s->title) > 0 &&
        strlen(s->artist[0]) > 0 &&
        strlen(s->album) > 0
        );
}

bool now_playing_is_valid(const struct scrobble *m/*, const time_t current_time, const time_t last_playing_time*/) {
#if 0
    _trace("Checking playtime: %u - %u", m->play_time, (m->length / 2));
    _trace("Checking scrobble:\n" \
        "title: %s\n" \
        "artist: %s\n" \
        "album: %s\n" \
        "length: %u\n" \
        "play_time: %u\n", m->title, m->artist, m->album, m->length, m->play_time);
#endif
    return (
        NULL != m &&
        NULL != m->title && strlen(m->title) > 0 &&
        (
         (sb_count(m->artist) > 0 && NULL != m->artist[0] && strlen(m->artist[0]) > 0) ||
         (NULL != m->album && strlen(m->album) > 0)
        ) &&
        //        last_playing_time > 0 &&
        //        difftime(current_time, last_playing_time) >= LASTFM_NOW_PLAYING_DELAY &&
        m->length > 0
        );
}

#if 0
void scrobble_print(const struct scrobble *s, enum log_levels log) {
    _log(log, "scrobble[%p]: \n" \
        "\ttitle: %s\n" \
        "\tartist: %s\n" \
        "\talbum: %s",
        s->title, s->artist[0], s->album);
}

static void scrobble_zero (struct scrobble *s)
{
    if (NULL == s) { return; }
    if (NULL != s->title) {
        memset(s->title, 0, strlen(s->title));
    }
    if (NULL != s->album) {
        memset(s->album, 0, strlen(s->album));
    }
    if (NULL != s->artist) {
        memset(s->artist, 0, strlen(s->artist));
    }
    if (NULL != s->mb_track_id) {
        memset(s->mb_track_id, 0, strlen(s->mb_track_id));
    }
    if (NULL != s->mb_album_id) {
        memset(s->mb_album_id, 0, strlen(s->mb_album_id));
    }
    if (NULL != s->mb_artist_id) {
        memset(s->mb_artist_id, 0, strlen(s->mb_artist_id));
    }
    if (NULL != s->mb_album_artist_id) {
        memset(s->mb_album_artist_id, 0, strlen(s->mb_album_artist_id));
    }
    if (NULL != s->mb_spotify_id) {
        memset(s->mb_spotify_id, 0, strlen(s->mb_spotify_id));
    }

    s->length = 0;
    s->position = 0;
    s->start_time = 0;
    s->play_time = 0;
    s->track_number = 0;
}
#endif

static void scrobble_copy (struct scrobble *t, const struct scrobble *s)
{
    if (NULL == t) { return; }
    if (NULL == s) { return; }

    assert(s->title);
    assert(s->album);
    assert(s->artist[0]);
    assert(t->album);
    assert(t->title);


    strncpy(t->title, s->title, MAX_PROPERTY_LENGTH);
    strncpy(t->album, s->album, MAX_PROPERTY_LENGTH);

    for (int i = 0; i < sb_count(s->artist); i++) {
        char *elem = get_zero_string(MAX_PROPERTY_LENGTH);
        strncpy(elem, s->artist[i], MAX_PROPERTY_LENGTH);
        sb_push(t->artist, elem);
    }

    if (NULL != t->mb_track_id) {
        strncpy(t->mb_track_id, s->mb_track_id, MAX_PROPERTY_LENGTH);
    }
    if (NULL != t->mb_album_id) {
        strncpy(t->mb_album_id, s->mb_album_id, MAX_PROPERTY_LENGTH);
    }
    if (NULL != t->mb_artist_id) {
        strncpy(t->mb_artist_id, s->mb_artist_id, MAX_PROPERTY_LENGTH);
    }
    if (NULL != t->mb_album_artist_id) {
        strncpy(t->mb_album_artist_id, s->mb_album_artist_id, MAX_PROPERTY_LENGTH);
    }
    if (NULL != t->mb_spotify_id) {
        strncpy(t->mb_spotify_id, s->mb_spotify_id, MAX_PROPERTY_LENGTH);
    }

    t->length = s->length;
    t->position = s->position;
    t->start_time = s->start_time;
    t->play_time = s->play_time;
    t->track_number = s->track_number;

    _trace("scrobbler::copied_scrobble:%p->%p", s, t);
}

static bool scrobbles_equal(const struct scrobble *s, const struct scrobble *p)
{
    if ((NULL == s) && (NULL == p)) { return true; }

    if (NULL == s) { return false; }
    if (NULL == p) { return false; }

    if (s == p) { return true; }

    assert(s->title);
    assert(p->title);
    assert(s->album);
    assert(p->album);
    assert(s->artist);
    assert(p->artist);
    bool result = (
        (strncmp(s->title, p->title, strlen(s->title)) == 0)
        && (strncmp(s->album, p->album, strlen(s->album)) == 0)
        && (strncmp(s->artist[0], p->artist[0], strlen(s->artist[0])) == 0)
#if 0
        && (s->length == p->length)
        && (s->track_number == p->track_number)
#endif
        );
    _trace("scrobbler::check_scrobbles(%p:%p) %s", s, p, result ? "same" : "different");

    return result;
}

bool load_scrobble(struct scrobble *d, const struct mpris_properties *p)
{
    if (NULL == d) { return false; }
    if (NULL == p) { return false ; }
    if (NULL == p->metadata) { return false; }

    if (NULL == p->metadata->title || strlen(p->metadata->title) == 0) { return false; }
    if (NULL == p->metadata->album || strlen(p->metadata->album) == 0) { return false; }
    if (sb_count(p->metadata->artist) == 0 || NULL == p->metadata->artist[0] || strlen(p->metadata->artist[0]) == 0) { return false; }

    strncpy(d->title, p->metadata->title, MAX_PROPERTY_LENGTH);
    strncpy(d->album, p->metadata->album, MAX_PROPERTY_LENGTH);
    int artist_count = sb_count(p->metadata->artist);

    if (NULL != d->artist) {
        for (int i = 0; i < sb_count(d->artist); i++) {
            if (NULL != d->artist[i]) { free(d->artist[i]); }
        }
        sb_free(d->artist);
    }
    for (int i = 0; i < artist_count; i++) {
        char *elem = get_zero_string(MAX_PROPERTY_LENGTH);
        strncpy(elem, p->metadata->artist[i], strlen(p->metadata->artist[i]));

        sb_push(d->artist, elem);
    }

    if (p->metadata->length > 0) {
        d->length = p->metadata->length / 1000000.0f;
    } else {
        d->length = 59;
    }
    if (p->position > 0) {
        d->position = p->position / 1000000.0f;
    }
    d->scrobbled = false;
    d->track_number = p->metadata->track_number;
    d->start_time = p->metadata->timestamp;
    if (d->position == 0) {
        // TODO(marius): This is an awful way of doing this, but it's needed
        //               because Spotify doesn't populate the position correctly.
        //               These types of player based logic should be moved to their own functions. See #25
        time_t now = time(0);
        d->play_time = now - p->metadata->timestamp;
    } else {
        d->play_time = d->position;
    }

    // musicbrainz data
    if (NULL != p->metadata->mb_track_id && strlen(p->metadata->mb_track_id) > 0) {
        strncpy(d->mb_track_id, p->metadata->mb_track_id, strlen(p->metadata->mb_track_id));
    }

    if (NULL != p->metadata->mb_album_id && strlen(p->metadata->mb_album_id) > 0) {
        strncpy(d->mb_album_id, p->metadata->mb_album_id, MAX_PROPERTY_LENGTH);
    }

    if (NULL != p->metadata->mb_artist_id && strlen(p->metadata->mb_artist_id) > 0) {
        strncpy(d->mb_artist_id, p->metadata->mb_artist_id, MAX_PROPERTY_LENGTH);
    }

    if (NULL != p->metadata->mb_album_artist_id && strlen(p->metadata->mb_album_artist_id) > 0) {
        strncpy(d->mb_album_artist_id, p->metadata->mb_album_artist_id, MAX_PROPERTY_LENGTH);
    }

    // if this is spotify we add the track_id as the spotify_id
    if (NULL != p->metadata->track_id && strncmp(p->metadata->track_id, MPRIS_SPOTIFY_TRACK_ID_PREFIX, strlen(MPRIS_SPOTIFY_TRACK_ID_PREFIX)) == 0) {
        strncpy(d->mb_spotify_id, p->metadata->track_id + strlen(MPRIS_SPOTIFY_TRACK_ID_PREFIX), MAX_PROPERTY_LENGTH);
    }
    if (now_playing_is_valid(d)) {
        _trace("scrobbler::loaded_scrobble(%p)", d);
        _trace("  scrobble::title: %s", d->title);
        _trace("  scrobble::artist[%zu]: %s...", sb_count(d->artist), d->artist[0]);
        _trace("  scrobble::album: %s", d->album);
        _trace("  scrobble::length: %lu", d->length);
        _trace("  scrobble::position: %.2f", d->position);
        _trace("  scrobble::scrobbled: %s", d->scrobbled ? "yes" : "no");
        _trace("  scrobble::track_number: %u", d->track_number);
        _trace("  scrobble::start_time: %lu", d->start_time);
        _trace("  scrobble::play_time: %.2f", d->play_time);

        if (strlen(d->mb_track_id) > 0) {
            _trace("  scrobble::musicbrainz::track_id: %s", d->mb_track_id);
        }
        if (strlen(d->mb_artist_id) > 0) {
            _trace("  scrobble::musicbrainz::artist_id: %s", d->mb_artist_id);
        }
        if (strlen(d->mb_album_id) > 0) {
            _trace("  scrobble::musicbrainz::album_id: %s", d->mb_album_id);
        }
        if (strlen(d->mb_album_artist_id) > 0) {
            _trace("  scrobble::musicbrainz::album_artist_id: %s", d->mb_album_artist_id);
        }
        if (strlen(d->mb_spotify_id) > 0) {
            _trace("  scrobble::musicbrainz::spotify_id: %s", d->mb_spotify_id);
        }
    }
    return true;
}

bool scrobbles_append(struct mpris_player *player, const struct scrobble *track)
{
    if (NULL == player) { return false; }
    if (NULL == track) { return false; }

    bool result = false;

    struct scrobble *n = scrobble_new();
    scrobble_copy(n, track);

    // TODO(marius) this looks very fishy, usually current and properties are equal
    int queue_count = sb_count(player->queue);
    if (queue_count > 0) {
        struct scrobble *current = sb_last(player->queue);
        _debug("scrobbler::queue_top[%u]: %p", queue_count, current);
        if (scrobbles_equal(current, n)) {
            _debug("scrobbler::queue: skipping existing scrobble(%p): %s//%s//%s", n, n->title, n->artist[0], n->album);
            scrobble_free(n);
            goto _exit;
        }
    }

    sb_push(player->queue, n);
    _debug("scrobbler::queue_push_scrobble(%p//%-4u) %s//%s//%s", n, queue_count, n->title, n->artist[0], n->album);
    _trace("scrobbler::new_queue_length: %zu", sb_count(player->queue));
    _trace2("scrobbler::copied_current:(%p::%p)", player->properties, player->current);

    result = true;

_exit:
    mpris_properties_zero(player->properties, true);

    return result;
}

static void debug_event(const struct mpris_event *e)
{
    _debug("scrobbler::checking_volume_changed:\t\t%3s", e->volume_changed ? "yes" : "no");
    _debug("scrobbler::checking_position_changed:\t\t%3s", e->position_changed ? "yes" : "no");
    _debug("scrobbler::checking_playback_status_changed:\t%3s", e->playback_status_changed ? "yes" : "no");
    _debug("scrobbler::checking_track_changed:\t\t%3s", e->track_changed ? "yes" : "no");
}

size_t scrobbles_consume_queue(struct scrobbler *scrobbler, struct scrobble **inc_tracks)
{
    if (NULL == scrobbler) { return 0; }

    int queue_length = sb_count(inc_tracks);
    if (queue_length == 0) { return 0; }

    _trace("scrobbler::queue_length: %u", queue_length);

    size_t consumed = 0;

    struct scrobble **tracks = NULL;
    for (int pos = 0; pos < queue_length; pos++) {
        struct scrobble *current = inc_tracks[pos];
        if (scrobble_is_valid(current)) {
            _info("scrobbler::scrobble::valid:(%p//%zu) %s//%s//%s", current, pos, current->title, current->artist[0], current->album);
            sb_push(tracks, current);
        } else {
            _warn("scrobbler::scrobble::invalid:(%p//%zu) %s//%s//%s", current, pos, current->title, current->artist[0], current->album);
        }
    }
    api_request_do(scrobbler, (const struct scrobble**)tracks, api_build_request_scrobble);
    sb_free(tracks);

    return consumed;
}

#endif // MPRIS_SCROBBLER_SCROBBLE_H
