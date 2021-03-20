#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define     NUM_CHAR_PER_LINE       8
#define     FONT_WIDTH              8
#define     FONT_HEIGHT             8
#define     FONT_NBYTES_PER_ROW     1
#define     FONT_NBYTES_PER_CHAR    (FONT_NBYTES_PER_ROW * FONT_HEIGHT)
#define     PIXEL_FONT              '*'
#define     PIXEL_BACKGROUND        ' '

// FILE *fopen(const char *pathname, const char *mode) {
//   fprintf(stderr, "You should not use fopen to open %s in this problem\n", pathname);
//   return 0;
// };

/* buf is the display buffer.
 * s is the string to be displayed on this 'line'.
 * fp points to the opened font file.
 *
 * The function retrieves font information for up to NUM_CHAR_PER_LINE characters
 * in s from fp, and updates the display buffer (buf).
 *
 * The function returns the number of bytes that have been processed.
 * Should be a number between 0 and NUM_CHAR_PER_LINE.
 *
 */
unsigned int str_render (char **buf, char *s, char *font8x8_basic) { // font8x8_basic[128][8]

    unsigned int n = 0;

    // 'Unpack' pixels here.
    // You could do it in the print_buffer function.

    char font[FONT_NBYTES_PER_CHAR];

    while (n < NUM_CHAR_PER_LINE && s[n]) {

        memcpy(font, font8x8_basic + (s[n]*8), FONT_NBYTES_PER_CHAR);

        int i;
        int offset = n * FONT_WIDTH;

        // for each row
        for (i = 0; i < FONT_HEIGHT; i ++) {
            char dots = font[i];
            char *p = buf[i] + offset;
            int  j, mask = 0x01;
            // for column
            for (j = 0; j < FONT_WIDTH; j ++, mask <<= 1) {
                if (dots & mask)
                    p[j] = PIXEL_FONT;
            }
        }
        n ++;
    }
    return n;
}

/* Clear the display buffer.
 * If print_buffer() is called right after, only PIXEL_BACKGROUND will be displayed.
 * Try to type 'clear' in your bash.
 * */
void clear_buffer(char **buf) {
    int     i;
    for (i = 0; i < FONT_HEIGHT; i ++) {
        memset(buf[i], PIXEL_BACKGROUND, NUM_CHAR_PER_LINE * FONT_WIDTH);
        buf[i][NUM_CHAR_PER_LINE * FONT_WIDTH] = 0;
    }
}

/* show the display buffer on the screen. */
void print_buffer(char ** buf) {
    int i;

    if (buf == NULL)
        return;

    for (i = 0; i < FONT_HEIGHT; i ++) {
        printf("%s\n", buf[i]);
    }
}

int main(int argc, char **argv) { // command = ./generate-fontfile font8x8.dat

    if (argc < 2) {
	fprintf(stderr, "Usage: %s <string>\n", argv[0]);
	return 1;
    }

    // open the font file
    FILE *fp;
    fp = popen("./generate-fontfile","r");
    if (fp == NULL) {
        fprintf(stderr, "Cannot generate font file data!!!!\n");
        fprintf(stderr, "Fix generate-fontfile\n");
        return 2;
    }
    
    char *font8x8_basic;
    font8x8_basic = (char *)malloc((128*8)+1);
    fread(font8x8_basic, 1, (FONT_NBYTES_PER_CHAR*128), fp);

    /*
     * High level strategy:
     *
     *      1. Allocate a 'display' buffer, a 2-D array that has FONT_HEIGHT rows and FONT_WIDTH * NUM_CHAR_PER_LINE + 1 columns
     *      2. Clear buffer.
     *      3. Call str_render() to render NUM_CHAR_PER_LINE characters into the display buffer.
     *      4. Print the buffer.
     *      5. If there are more characters to display, goto 2.
     *      6. Free buffer.
     *
     * Elements in the buffer are either PIXEL_FONT for PIXEL_BACKGROUND.
     *      +Less work in print_buf()
     *      +Easy to operate on buffer, e.g., add a border
     *      -More memory
     */

    char **buf;

    buf = malloc(FONT_HEIGHT * sizeof(char *));
    assert(buf != NULL);

    int  len_row = FONT_WIDTH * NUM_CHAR_PER_LINE + 1; // Ending NUL
    buf[0] = malloc(len_row * FONT_HEIGHT);
    assert(buf[0] != NULL);

    int i, pos;
    for (i = 1; i < FONT_HEIGHT; i ++)
        buf[i] = buf[i-1] + len_row;

    for (pos = 0; argv[1][pos]; ) {
        clear_buffer(buf);
        int r = str_render(buf, argv[1] + pos, font8x8_basic);
        print_buffer(buf);
        pos += r;
    }

    free(buf[0]);
    free(buf);
    free(font8x8_basic);
    fclose(fp);
    return 0;
}
