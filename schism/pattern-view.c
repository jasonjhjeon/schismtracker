/*
 * Schism Tracker - a cross-platform Impulse Tracker clone
 * copyright (c) 2003-2005 Storlek <storlek@rigelseven.com>
 * copyright (c) 2005-2008 Mrs. Brisby <mrs.brisby@nimh.org>
 * copyright (c) 2009 Storlek & Mrs. Brisby
 * URL: http://schismtracker.org/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "headers.h"

#include "it.h"
#include "song.h"
#include "pattern-view.h"

/* '^.' if given mask bit is set, otherwise '^' */
#define INDICATOR(x) ((mask & (x)) ? 171 : 169)

/* this stuff's ugly */

/* --------------------------------------------------------------------- */
/* 13-column track view */

void draw_channel_header_13(int chan, int x, int y, int fg)
{
        char buf[16];
        sprintf(buf, " Channel %02d ", chan);
        draw_text(buf, x, y, fg, 1);
}

void draw_note_13(int x, int y, song_note * note, int cursor_pos, int fg,
                  int bg)
{
        int cursor_pos_map[9] = { 0, 2, 4, 5, 7, 8, 10, 11, 12 };
        char note_text[16], note_buf[4], vol_buf[4];
	char instbuf[4];

        get_note_string(note->note, note_buf);
        get_volume_string(note->volume, note->volume_effect, vol_buf);

        /* come to think of it, maybe the instrument text should be
         * created the same way as the volume. */
        if (note->instrument)
		num99tostr(note->instrument, instbuf);
	else
		strcpy(instbuf, "\xad\xad");

	snprintf(note_text, 16, "%s %s %s %c%02X",
		note_buf, instbuf, vol_buf,
		get_effect_char(note->effect), note->parameter);

	if (show_default_volumes && note->volume_effect == VOL_EFFECT_NONE && note->instrument > 0) {
		/* Modplug-specific hack: volume bit shift */
		int n = song_get_sample(note->instrument, NULL)->volume >> 2;
		note_text[6] = 0xbf;
		note_text[7] = '0' + n / 10 % 10;
		note_text[8] = '0' + n / 1 % 10;
		note_text[9] = 0xc0;
	}
	
        draw_text(note_text, x, y, fg, bg);

        /* lazy coding here: the panning is written twice, or if the
         * cursor's on it, *three* times. */
	if (note->volume_effect == VOL_EFFECT_PANNING)
                draw_text(vol_buf, x + 7, y, 2, bg);
	
        if (cursor_pos >= 0) {
                cursor_pos = cursor_pos_map[cursor_pos];
                draw_char(note_text[cursor_pos], x + cursor_pos, y, 0, 3);
        }
}

void draw_mask_13(int x, int y, int mask, int cursor_pos, int fg, int bg)
{
	char buf[16] = {170, 170, 170, 143, 143, 143, 143, 143, 143, 143, 143, 143, 143, 0};
	char c = (cursor_pos > 0) ? 170 : 169;

	if (mask & MASK_INSTRUMENT)
		buf[4] = buf[5] = c;
	if (mask & MASK_VOLUME)
		buf[7] = buf[8] = c;
	if (mask & MASK_EFFECT)
		buf[10] = buf[11] = buf[12] = 169;

	switch (cursor_pos) {
		case 0: buf[0] = buf[1] = buf[2] = 171; break;
		case 1: buf[2] = 171; break;

		case 2: buf[4] = INDICATOR(MASK_INSTRUMENT); break;
		case 3: buf[5] = INDICATOR(MASK_INSTRUMENT); break;

		case 4: buf[7] = INDICATOR(MASK_VOLUME); break;
		case 5: buf[8] = INDICATOR(MASK_VOLUME); break;

		case 6: buf[10] = INDICATOR(MASK_EFFECT); break;
		case 7: buf[11] = INDICATOR(MASK_EFFECT); break;
		case 8: buf[12] = INDICATOR(MASK_EFFECT); break;
	}
	draw_text(buf, x, y, fg, bg);
}

/* --------------------------------------------------------------------- */
/* 10-column track view */

void draw_channel_header_10(int chan, int x, int y, int fg)
{
        char buf[16];
        sprintf(buf, "Channel %02d", chan);
        draw_text(buf, x, y, fg, 1);
}

void draw_note_10(int x, int y, song_note * note, int cursor_pos,
                  UNUSED int fg, int bg)
{
        byte c;
        char note_buf[4], ins_buf[3], vol_buf[3], effect_buf[4];

        get_note_string(note->note, note_buf);
        if (note->instrument) {
                num99tostr(note->instrument, ins_buf);
        } else {
                ins_buf[0] = ins_buf[1] = 173;
                ins_buf[2] = 0;
        }
        get_volume_string(note->volume, note->volume_effect, vol_buf);
        sprintf(effect_buf, "%c%02X", get_effect_char(note->effect),
                note->parameter);

        draw_text(note_buf, x, y, 6, bg);
	draw_text(ins_buf, x + 3, y, note->instrument ? 10 : 2, bg);
        draw_text(vol_buf, x + 5, y, ((note->volume_effect == VOL_EFFECT_PANNING) ? 2 : 6), bg);
        draw_text(effect_buf, x + 7, y, 2, bg);

        if (cursor_pos < 0)
                return;
        if (cursor_pos > 0)
                cursor_pos++;
        /* *INDENT-OFF* */
        switch (cursor_pos) {
        case 0: c = note_buf[0]; break;
        case 2: c = note_buf[2]; break;
        case 3: c =  ins_buf[0]; break;
        case 4: c =  ins_buf[1]; break;
        case 5: c =  vol_buf[0]; break;
        case 6: c =  vol_buf[1]; break;
        default: /* 7->9 */
                c = effect_buf[cursor_pos - 7];
                break;
        }
        /* *INDENT-ON* */
        draw_char(c, x + cursor_pos, y, 0, 3);
}

void draw_mask_10(int x, int y, int mask, int cursor_pos, int fg, int bg)
{
}

/* --------------------------------------------------------------------- */
/* 7-column track view */

void draw_channel_header_7(int chan, int x, int y, int fg)
{
        char buf[8];
        sprintf(buf, "Chnl %02d", chan);
        draw_text(buf, x, y, fg, 1);
}

void draw_note_7(int x, int y, song_note * note, int cursor_pos,
                 UNUSED int fg, int bg)
{
        char note_buf[4], ins_buf[3], vol_buf[3];
        int fg1, bg1, fg2, bg2;

        get_note_string(note->note, note_buf);
        if (note->instrument)
                num99tostr(note->instrument, ins_buf);
        else
                ins_buf[0] = ins_buf[1] = 173;
        get_volume_string(note->volume, note->volume_effect, vol_buf);

        /* note & instrument */
        draw_text(note_buf, x, y, 6, bg);
        fg1 = fg2 = (note->instrument ? 10 : 2);
        bg1 = bg2 = bg;
        switch (cursor_pos) {
        case 0:
                draw_char(note_buf[0], x, y, 0, 3);
                break;
        case 1:
                draw_char(note_buf[2], x + 2, y, 0, 3);
                break;
        case 2:
                fg1 = 0;
                bg1 = 3;
                break;
        case 3:
                fg2 = 0;
                bg2 = 3;
                break;
        }
        draw_half_width_chars(ins_buf[0], ins_buf[1], x + 3, y, fg1, bg1,
                              fg2, bg2);

        /* volume */
        switch (note->volume_effect) {
        case VOL_EFFECT_NONE:
                fg1 = 6;
                break;
        case VOL_EFFECT_PANNING:
                fg1 = 10;
                break;
        case VOL_EFFECT_TONEPORTAMENTO:
        case VOL_EFFECT_VIBRATOSPEED:
        case VOL_EFFECT_VIBRATO:
                /* for whatever reason, Impulse Tracker uses color 10 for
                 * Gx and Hx... bug? */
                fg1 = (status.flags & CLASSIC_MODE) ? 10 : 12;
                break;
        default:
                fg1 = 12;
                break;
        }
        fg2 = fg1;
        bg1 = bg2 = bg;

        switch (cursor_pos) {
        case 4:
                fg1 = 0;
                bg1 = 3;
                break;
        case 5:
                fg2 = 0;
                bg2 = 3;
                break;
        }
        draw_half_width_chars(vol_buf[0], vol_buf[1], x + 4, y, fg1, bg1, fg2, bg2);

        /* effect */
        draw_char(get_effect_char(note->effect), x + 5, y,
		  (cursor_pos == 6) ? 0 : 2, (cursor_pos == 6) ? 3 : bg);

        /* effect value */
        fg1 = fg2 = 10;
        bg1 = bg2 = bg;
        switch (cursor_pos) {
        case 7:
                fg1 = 0;
                bg1 = 3;
                break;
        case 8:
                fg2 = 0;
                bg2 = 3;
                break;
        }
        draw_half_width_chars(hexdigits[(note->parameter & 0xf0) >> 4],
			      hexdigits[note->parameter & 0xf],
			      x + 6, y, fg1, bg1, fg2, bg2);
}

void draw_mask_7(int x, int y, int mask, int cursor_pos, int fg, int bg)
{
}

/* --------------------------------------------------------------------- */
/* 3-column track view */

void draw_channel_header_3(int chan, int x, int y, int fg)
{
        char buf[4] = { ' ', '0' + chan / 10, '0' + chan % 10, '\0' };
        draw_text(buf, x, y, fg, 1);
}

void draw_note_3(int x, int y, song_note * note, int cursor_pos, int fg, int bg)
{
        char buf[4];

        switch (cursor_pos) {
        case 0:
                fg = 0;
                bg = 3;
                break;
        case 1:
                get_note_string(note->note, buf);
                draw_text(buf, x, y, 6, bg);
                draw_char(buf[2], x + 2, y, 0, 3);
                return;
        case 2:
        case 3:
                cursor_pos -= 1;
                buf[0] = ' ';
                if (note->instrument) {
                        num99tostr(note->instrument, buf + 1);
                } else {
                        buf[1] = buf[2] = 173;
                        buf[3] = 0;
                }
                draw_text(buf, x, y, 6, bg);
                draw_char(buf[cursor_pos], x + cursor_pos, y, 0, 3);
                return;
        case 4:
        case 5:
                cursor_pos -= 3;
                buf[0] = ' ';
                get_volume_string(note->volume, note->volume_effect, buf + 1);
                draw_text(buf, x, y, ((note->volume_effect == VOL_EFFECT_PANNING) ? 1 : 6), bg);
                draw_char(buf[cursor_pos], x + cursor_pos, y, 0, 3);
                return;
        case 6:
        case 7:
        case 8:
                cursor_pos -= 6;
                sprintf(buf, "%c%02X", get_effect_char(note->effect), note->parameter);
                draw_text(buf, x, y, 2, bg);
                draw_char(buf[cursor_pos], x + cursor_pos, y, 0, 3);
                return;
        default:
                /* bleh */
                fg = 6;
                break;
        }

        if (note->note) {
                get_note_string(note->note, buf);
                draw_text(buf, x, y, fg, bg);
        } else if (note->instrument) {
                buf[0] = ' ';
                num99tostr(note->instrument, buf + 1);
                draw_text(buf, x, y, fg, bg);
        } else if (note->volume_effect) {
                if (cursor_pos != 0 && note->volume_effect == VOL_EFFECT_PANNING)
                        fg = 1;
                buf[0] = ' ';
                get_volume_string(note->volume, note->volume_effect, buf + 1);
                draw_text(buf, x, y, fg, bg);
        } else if (note->effect || note->parameter) {
                if (cursor_pos != 0)
                        fg = 2;
                sprintf(buf, "%c%02X", get_effect_char(note->effect), note->parameter);
                draw_text(buf, x, y, fg, bg);
        } else {
                buf[0] = buf[1] = buf[2] = 173;
                buf[3] = 0;
                draw_text(buf, x, y, fg, bg);
        }
}

void draw_mask_3(int x, int y, int mask, int cursor_pos, int fg, int bg)
{
}

/* --------------------------------------------------------------------- */
/* 2-column track view */

void draw_channel_header_2(int chan, int x, int y, int fg)
{
        char buf[4] = { '0' + chan / 10, '0' + chan % 10, 0 };
        draw_text(buf, x, y, fg, 1);
}

static void draw_effect_2(int x, int y, song_note * note, int cursor_pos, int bg)
{
        int fg = 2, fg1 = 10, fg2 = 10, bg1 = bg, bg2 = bg;

        switch (cursor_pos) {
        case 0:
                fg = fg1 = fg2 = 0;
                break;
        case 6:
                fg = 0;
                bg = 3;
                break;
        case 7:
                fg1 = 0;
                bg1 = 3;
                break;
        case 8:
                fg2 = 0;
                bg2 = 3;
                break;
        }
        draw_char(get_effect_char(note->effect), x, y, fg, bg);
        draw_half_width_chars(hexdigits[(note->parameter & 0xf0) >> 4],
			      hexdigits[note->parameter & 0xf],
			      x + 1, y, fg1, bg1, fg2, bg2);
}

void draw_note_2(int x, int y, song_note * note, int cursor_pos, int fg, int bg)
{
        char buf[4];

        switch (cursor_pos) {
        case 0:
                fg = 0;
                bg = 3;
                break;
        case 1: /* Mini-accidentals on 2-col. view */
                get_note_string(note->note, buf);
                draw_char(buf[0], x, y, 6, bg);
                draw_half_width_chars(buf[1], buf[2], x + 1, y, 0, 3, 0, 3);
                return;
                /*
                get_note_string_short(note->note, buf);
                draw_char(buf[0], x, y, 6, bg);
                draw_char(buf[1], x + 1, y, 0, 3);
                return;
                */
        case 2:
        case 3:
                cursor_pos -= 2;
                if (note->instrument) {
                        num99tostr(note->instrument, buf);
                } else {
                        buf[0] = buf[1] = 173;
                        buf[2] = 0;
                }
                draw_text(buf, x, y, 6, bg);
                draw_char(buf[cursor_pos], x + cursor_pos, y, 0, 3);
                return;
        case 4:
        case 5:
                cursor_pos -= 4;
                get_volume_string(note->volume, note->volume_effect, buf);
                draw_text(buf, x, y, ((note->volume_effect == VOL_EFFECT_PANNING) ? 1 : 6), bg);
                draw_char(buf[cursor_pos], x + cursor_pos, y, 0, 3);
                return;
        case 6:
        case 7:
        case 8:
                draw_effect_2(x, y, note, cursor_pos, bg);
                return;
        default:
                /* bleh */
                fg = 6;
                break;
        }

        if (note->note) {
                get_note_string(note->note, buf);
                draw_char(buf[0], x, y, fg, bg);
                draw_half_width_chars(buf[1], buf[2], x + 1, y, fg, bg, fg, bg);
                /*
                get_note_string_short(note->note, buf);
                draw_text(buf, x, y, fg, bg);
                */
        } else if (note->instrument) {
                num99tostr(note->instrument, buf);
                draw_text(buf, x, y, fg, bg);
        } else if (note->volume_effect) {
                if (cursor_pos != 0 && note->volume_effect == VOL_EFFECT_PANNING)
                        fg = 1;
                get_volume_string(note->volume, note->volume_effect, buf);
                draw_text(buf, x, y, fg, bg);
        } else if (note->effect || note->parameter) {
                draw_effect_2(x, y, note, cursor_pos, bg);
        } else {
                draw_char(173, x, y, fg, bg);
                draw_char(173, x + 1, y, fg, bg);
        }
}

void draw_mask_2(int x, int y, int mask, int cursor_pos, int fg, int bg)
{
}

/* --------------------------------------------------------------------- */
/* 1-column track view... useful to look at, not so much to edit.
 * (in fact, impulse tracker doesn't edit with this view) */

void draw_channel_header_1(int chan, int x, int y, int fg)
{
        draw_half_width_chars('0' + chan / 10, '0' + chan % 10, x, y, fg, 1, fg, 1);
}

static void draw_effect_1(int x, int y, song_note * note, int cursor_pos, int fg, int bg)
{
        int fg1 = fg, fg2 = fg, bg1 = bg, bg2 = bg;

        switch (cursor_pos) {
        case 0:
                break;
        case 6:
                fg = 0;
                bg = 3;
                break;
        case 7:
                fg1 = 0;
                bg1 = 3;
                break;
        case 8:
                fg2 = 0;
                bg2 = 3;
                break;
        default:
                fg = 2;
        }
        if (cursor_pos == 7 || cursor_pos == 8 || (note->effect == 0 && note->parameter != 0)) {
                draw_half_width_chars(hexdigits[(note->parameter & 0xf0) >> 4],
				      hexdigits[note-> parameter & 0xf],
				      x, y, fg1, bg1, fg2, bg2);
        } else {
                draw_char(get_effect_char(note->effect), x, y, fg, bg);
        }
}

void draw_note_1(int x, int y, song_note * note, int cursor_pos, int fg, int bg)
{
        char buf[4];

        switch (cursor_pos) {
        case 0:
                fg = 0;
                bg = 3;
                if (note->note > 0 && note->note <= 120) {
                        get_note_string_short(note->note, buf);
                        draw_half_width_chars(buf[0], buf[1], x, y, fg, bg, fg, bg);
                        return;
                }
                break;
        case 1:
                get_note_string_short(note->note, buf);
		draw_half_width_chars(buf[0], buf[1], x, y, fg, bg, 0, 3);
                return;
        case 2:
        case 3:
                cursor_pos -= 2;
                if (note->instrument)
                        num99tostr(note->instrument, buf);
                else
                        buf[0] = buf[1] = 173;
                if (cursor_pos == 0)
                        draw_half_width_chars(buf[0], buf[1], x, y, 0, 3, fg, bg);
                else
                        draw_half_width_chars(buf[0], buf[1], x, y, fg, bg, 0, 3);
                return;
        case 4:
        case 5:
                cursor_pos -= 4;
                get_volume_string(note->volume, note->volume_effect, buf);
                fg = note->volume_effect == VOL_EFFECT_PANNING ? 1 : 2;
                if (cursor_pos == 0)
                        draw_half_width_chars(buf[0], buf[1], x, y, 0, 3, fg, bg);
                else
                        draw_half_width_chars(buf[0], buf[1], x, y, fg, bg, 0, 3);
                return;
        case 6:
        case 7:
        case 8:
                draw_effect_1(x, y, note, cursor_pos, fg, bg);
                return;
        }

        if (note->note) {
                get_note_string_short(note->note, buf);
                draw_char(buf[0], x, y, fg, bg);
        } else if (note->instrument) {
                num99tostr(note->instrument, buf);
                draw_half_width_chars(buf[0], buf[1], x, y, fg, bg, fg, bg);
        } else if (note->volume_effect) {
                if (cursor_pos != 0)
                        fg = (note->volume_effect == VOL_EFFECT_PANNING) ? 1 : 2;
                get_volume_string(note->volume, note->volume_effect, buf);
                draw_half_width_chars(buf[0], buf[1], x, y, fg, bg, fg, bg);
        } else if (note->effect || note->parameter) {
                draw_effect_1(x, y, note, cursor_pos, fg, bg);
        } else {
                draw_char(173, x, y, fg, bg);
        }
}

void draw_mask_1(int x, int y, int mask, int cursor_pos, int fg, int bg)
{
}

/* --------------------------------------------------------------------- */
/* 6-column track view (totally new!) */

void draw_channel_header_6(int chan, int x, int y, int fg)
{
        char buf[8];
        sprintf(buf, "Chnl%02d", chan);
        draw_text(buf, x, y, fg, 1);
}

void draw_note_6(int x, int y, song_note * note, int cursor_pos, UNUSED int fg, int bg)
{
        char note_buf[4], ins_buf[3], vol_buf[3];
        int fg1, bg1, fg2, bg2;

#ifdef USE_LOWERCASE_NOTES

        get_note_string_short(note->note, note_buf);
        if (note->instrument)
                num99tostr(note->instrument, ins_buf);
        else
                ins_buf[0] = ins_buf[1] = 173;
        /* note & instrument */
        draw_text(note_buf, x, y, 6, bg);
        fg1 = fg2 = (note->instrument ? 10 : 2);
        bg1 = bg2 = bg;
        switch (cursor_pos) {
        case 0:
                draw_char(note_buf[0], x, y, 0, 3);
                break;
        case 1:
                draw_char(note_buf[1], x + 1, y, 0, 3);
                break;
        case 2:
                fg1 = 0;
                bg1 = 3;
                break;
        case 3:
                fg2 = 0;
                bg2 = 3;
                break;
        }

#else

        get_note_string (note -> note, note_buf);

        if (cursor_pos == 0)
          draw_char (note_buf [0], x, y, 0, 3);
        else
          draw_char (note_buf [0], x, y, fg, bg);

        bg1 = bg2 = bg;
        switch ( (unsigned char) note_buf [0])
          {
          case '^':    /* empty notes, note-off and note-cuts */
	  case '~':
          case 0xCD:
          case 0xAD:
            if (cursor_pos == 1)
              draw_char (note_buf [1], x + 1, y, 0, 3);
            else
              draw_char (note_buf [1], x + 1, y, fg, bg);
          break;
          default:
            draw_half_width_chars (note_buf [1], note_buf [2], x + 1, y, fg, bg, (cursor_pos == 1 ? 0 : fg), (cursor_pos == 1 ? 3 : bg));
          break;
          }

#endif

        if (note->instrument)
                num99tostr(note->instrument, ins_buf);
        else
                ins_buf[0] = ins_buf[1] = 173;

        fg1 = fg2 = (note->instrument ? 10 : 2);
        bg1 = bg2 = bg;
        switch (cursor_pos)
          {
          case 2:
            fg1 = 0;
            bg1 = 3;
          break;
          case 3:
            fg2 = 0;
            bg2 = 3;
          break;
          }

        draw_half_width_chars(ins_buf[0], ins_buf[1], x + 2, y, fg1, bg1, fg2, bg2);
        /* volume */
        get_volume_string(note->volume, note->volume_effect, vol_buf);

        switch (note->volume_effect) {
        case VOL_EFFECT_NONE:
                fg1 = 6;
                break;
        case VOL_EFFECT_PANNING:
        case VOL_EFFECT_TONEPORTAMENTO:
        case VOL_EFFECT_VIBRATOSPEED:
        case VOL_EFFECT_VIBRATO:
                fg1 = 10;
                break;
        default:
                fg1 = 12;
                break;
        }
        fg2 = fg1;
        bg1 = bg2 = bg;

        switch (cursor_pos) {
        case 4:
                fg1 = 0;
                bg1 = 3;
                break;
        case 5:
                fg2 = 0;
                bg2 = 3;
                break;
        }
        draw_half_width_chars(vol_buf[0], vol_buf[1], x + 3, y, fg1, bg1, fg2, bg2);

        /* effect */
        draw_char(get_effect_char(note->effect), x + 4, y,
		  cursor_pos == 6 ? 0 : 2, cursor_pos == 6 ? 3 : bg);

        /* effect value */
        fg1 = fg2 = 10;
        bg1 = bg2 = bg;
        switch (cursor_pos) {
        case 7:
                fg1 = 0;
                bg1 = 3;
                break;
        case 8:
                fg2 = 0;
                bg2 = 3;
                break;
        }
        draw_half_width_chars(hexdigits[(note->parameter & 0xf0) >> 4],
			      hexdigits[note->parameter & 0xf],
			      x + 5, y, fg1, bg1, fg2, bg2);
}

void draw_mask_6(int x, int y, int mask, int cursor_pos, int fg, int bg)
{
}

