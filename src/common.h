/*
 * common constant header
 * author: 	Stefan Seifried
 * date:	25.05.2011
 * matr.nr.:0925401
 */

/* constants */
#define SUCCESS			0	/* operation was a success */
#define FAIL			1 	/* general operation failure */
#define FAT_FILEEND		2	/* code when file has been finished reading */


/* WARNING, although debug output works fine in most cases it can lead to a lock up when
 * used during an interrupt routine. Seen so in the sending of mp3 data and removing the
 * card during this operation
 */
//#define	DEBUG		1	/* enable debug output, comment this line for release builds */

