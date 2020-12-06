/*
 * fat file system implementation
 * author: 	Stefan Seifried
 * date:	30.05.2011
 * matr.nr.:0925401
 */

#ifndef _FAT_H_
#define _FAT_H_


/* constants */
#define TYPE_FAT12	0
#define TYPE_FAT16 	1
#define TYPE_FAT32	2
#define TYPE_UNKN	3

#define DIRENTRY_FREE	0xE5
#define DIRENTRY_EOL	0x00

#define CLUSTER_FREE	0x0000
#define CLUSTER_DEFECT	0xFFF7
#define CLUSTER_LAST1	0xFFF8
#define CLUSTER_LAST2	0xFFF9
#define CLUSTER_LAST3	0xFFFA
#define CLUSTER_LAST4	0xFFFB
#define CLUSTER_LAST5	0xFFFC
#define CLUSTER_LAST6	0xFFFD
#define CLUSTER_LAST7	0xFFFE
#define CLUSETR_LAST8	0xFFFF
#define CLUSTER_ROOT1	0xFFF0
#define CLUSTER_ROOT2	0xFFF1
#define CLUSTER_ROOT3	0xFFF2
#define CLUSTER_ROOT4	0xFFF3
#define CLUSTER_ROOT5	0xFFF4
#define CLUSTER_ROOT6	0xFFF5
#define CLUSTER_ROOT7	0xFFF6


/* types */
typedef uint8_t (*fat_readblock_t)( uint32_t _nAddress, /*out*/ uint8_t* _pBuffer );


/* struct */
/*
 * FAT Boot Parameter Block, 
 * in order to save space on the MC we use a version
 * stripped down to our needs
 */
struct FAT_BootParameterBlock {
	uint16_t	nBytesPerSec;		/* Offset 11, bytes per sector */
	uint8_t		nSecPerClus;		/* Offset 13, sector per cluster */
	uint16_t	nRsvdSecCnt;		/* Offset 14, reserved sector count */
	uint8_t		nNumFATs;			/* Offset 16, number of fats */ 
	uint16_t	nRootEntCnt;		/* Offset 17, number of root directory entries */
	uint32_t	nTotSec;			/* Offset 19, total sector count, or Offset 32 - 35 */
	uint16_t	nFATSz16;			/* Offset 22, size of fat */
  
};
typedef struct FAT_BootParameterBlock FAT_BootParameterBlock_t;

/*
 * FAT directory entry
 * in order to save space on the MC we use a version
 * stripped down to our needs
 */
struct FAT_Handle {
	char		sName[12];			/* Offset 0, short name */
	uint16_t	nCluster;			/* Offset 26, first cluster number */
	uint32_t	nFileSize;			/* Offset 28, file size */

	uint32_t	nFilePos;			/* current file offset */
	uint8_t		nSector;			/* current sector */
	uint16_t	nSectorOffset;		/* current offset */				
};
typedef struct FAT_Handle FAT_Handle_t;

/*
 * FAT class storage
 */
struct FAT_Stream {
	fat_readblock_t				fpRead;				/* read function */

	uint16_t					nRootDirSectors;	/* number of root dir sectors */
	uint16_t					nFirstDataSector;	/* start of data sectors */
	FAT_BootParameterBlock_t	structBPB;			/* fat boot parameter block */
};
typedef struct FAT_Stream FAT_Stream_t;


/* functions */
uint8_t fat_ctor( FAT_Stream_t* _this, fat_readblock_t _fpRead );
uint8_t fat_getfattype( FAT_Stream_t* _this );
uint8_t fat_fopen( const char* _sFileName, FAT_Stream_t* _this, /*out*/ FAT_Handle_t* _pResult );
uint8_t fat_fread( uint8_t* _pBuffer, FAT_Stream_t* _this, FAT_Handle_t* _fileHandle );
uint8_t fat_seek( uint32_t _nPosition, FAT_Stream_t* _this, FAT_Handle_t* _fileHandle );

#endif /* _FAT_H_ */
