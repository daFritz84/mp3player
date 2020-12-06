/*
 * fat file system implementation
 * author: 	Stefan Seifried
 * date:	30.05.2011
 * matr.nr.:0925401
 */

/* includes */
#include <avr/io.h>
#include <avr/pgmspace.h>
#include "string.h"

#include "fat.h"
#include "common.h"


/* helper macro's */
#define CHECKREAD( __read__ ) \
	if( (__read__) == FAIL ) { \
		return FAIL; \
	}

// DEBUG
#include <stdio.h>



/*
 * convert cluster to corresponding sector
 */
static inline uint32_t fat_cluster2sector( FAT_Stream_t* _this, uint16_t _nCluster ) {
	return (uint32_t)(_nCluster - 2) * (uint32_t)_this->structBPB.nSecPerClus + (uint32_t)_this->nFirstDataSector;
}

/*
 * get next cluster
 */
static uint16_t fat_nextcluster( FAT_Stream_t* _this, uint16_t _nCluster ) {
	uint8_t aBuffer[32];
	uint16_t nNextSector;
	uint16_t nFATSecNum = _this->structBPB.nRsvdSecCnt + (_nCluster<<1)/_this->structBPB.nBytesPerSec;
	uint16_t nFATEntOffset = (_nCluster<<1) - ((_nCluster<<1)/_this->structBPB.nBytesPerSec)*_this->structBPB.nBytesPerSec;
	uint32_t nFATAddress = (nFATSecNum*_this->structBPB.nBytesPerSec + nFATEntOffset);	
	uint8_t nByteIndex = nFATAddress - ((nFATAddress>>5)<<5);

	_this->fpRead( nFATAddress>>5, aBuffer );
	nNextSector = ((aBuffer[ nByteIndex ]) | (aBuffer[ nByteIndex + 1 ]<<8));

	if( nNextSector > CLUSTER_LAST1 ) {
		return 0;	
	}
	else {
		return nNextSector;
	}	
}

/*
 * initialize fat file system
 */
uint8_t fat_ctor( FAT_Stream_t* _this, fat_readblock_t _fpRead ) {
	uint8_t aBuffer[32];
	
	/* set device access reader */	
	_this->fpRead = _fpRead;

	/* read boot parameter */
	CHECKREAD( _fpRead( 0, aBuffer ) );		

	/* populate boot parameter block */	
	_this->structBPB.nBytesPerSec = ( (aBuffer[11]) | (aBuffer[12]<<8) );	
	_this->structBPB.nSecPerClus = ( (aBuffer[13]) );
	_this->structBPB.nRsvdSecCnt = ( (aBuffer[14]) | (aBuffer[15]<<8) );	
	_this->structBPB.nNumFATs = ( (aBuffer[16]) );	
	_this->structBPB.nRootEntCnt = ( (aBuffer[17]) | (aBuffer[18]<<8) );

	_this->structBPB.nFATSz16 = ( (aBuffer[22]) | (aBuffer[23]<<8) ); 	

	if( ( (aBuffer[19]) | (aBuffer[20]<<8) ) == 0 ) {
		CHECKREAD( _fpRead( 1, aBuffer ) );
		_this->structBPB.nTotSec = (uint32_t)( ((uint32_t)aBuffer[0]) | ((uint32_t)aBuffer[1]<<8) | 
			((uint32_t)aBuffer[2]<<16) | ((uint32_t)aBuffer[3]>>24) );
	}
	else {
		_this->structBPB.nTotSec = ( (aBuffer[19]) | (aBuffer[20]<<8) );
	}
	
	#ifdef DEBUG
	printf_P( PSTR("Bytes per Sec: %d\nSector per Cluster: %d\nReserved Sector Count:%d\n"
		"Number of FAT's: %d\nRootEntryCount: %d\nFAT Size: %d\nTotal Sector Count:%lu\n"), 
		_this->structBPB.nBytesPerSec, _this->structBPB.nSecPerClus, _this->structBPB.nRsvdSecCnt,
		_this->structBPB.nNumFATs, _this->structBPB.nRootEntCnt, _this->structBPB.nFATSz16,
 		_this->structBPB.nTotSec );	
	#endif

	/* calc root dir sectors */
	_this->nRootDirSectors = ((_this->structBPB.nRootEntCnt << 5) + (_this->structBPB.nBytesPerSec - 1)) / 
		_this->structBPB.nBytesPerSec;
	
	/* calc start data start sector */
	_this->nFirstDataSector = _this->structBPB.nRsvdSecCnt + (_this->structBPB.nNumFATs * _this->structBPB.nFATSz16) + 
		_this->nRootDirSectors;

	/* check for correct fat type */
	if( fat_getfattype( _this ) == TYPE_FAT16 ) {
		return SUCCESS;
	}	
	else {
		return FAIL;
	}
}

/*
 * determine fat type
 * note, that only fat16 is supported right now
 * we could maybe implement fat32 detection, but would make no sense
 * since we do not support it. So fat32 is returned as unknown
 */
uint8_t fat_getfattype( FAT_Stream_t* _this ) {
	uint32_t nDataSectors;
	int32_t nCountOfClusters;	
	
	if( _this->structBPB.nFATSz16 == 0 ){
		return TYPE_UNKN;
	}
	
	nDataSectors = _this->structBPB.nTotSec - (_this->structBPB.nRsvdSecCnt + 
		(_this->structBPB.nNumFATs * _this->structBPB.nFATSz16) + _this->nRootDirSectors);
	nCountOfClusters = nDataSectors / _this->structBPB.nSecPerClus;

	if(nCountOfClusters < 4085LU ) {
		return TYPE_FAT12;
	}	
	if(nCountOfClusters < 65525LU ) {
		return TYPE_FAT16;
	} 
	else {
		return TYPE_UNKN;
	}
}

/*
 * find file in FAT directory
 * _pResult is only valid if function returns success
 */
uint8_t fat_fopen( const char* _sFileName, FAT_Stream_t* _this, /*out*/ FAT_Handle_t* _pResult ) {	
	uint8_t aBuffer[32];

	/* calculate address of current FATs data sector */
	uint32_t nFirstRootDirAddress = (uint32_t)(_this->structBPB.nRsvdSecCnt + (_this->structBPB.nNumFATs * 
		_this->structBPB.nFATSz16)) * _this->structBPB.nBytesPerSec;
	nFirstRootDirAddress/=32;	

	/* read entries until we reach end of list or find given filename */
	while(1) {
		CHECKREAD( _this->fpRead( nFirstRootDirAddress, aBuffer ) );
		
		/* dir entry not occupied, skip */
		if( aBuffer[0] == DIRENTRY_FREE ) {
			nFirstRootDirAddress++;
			continue;		
		}
		/* dir entry is end of list */
		else if( aBuffer[0] == DIRENTRY_EOL ) {
			return FAIL;
		}
		/* compare names */
		else if( strncmp( _sFileName, (char*) aBuffer, (strlen(_sFileName) > 8) ? 8 : strlen(_sFileName) ) == 0 ) {
			memcpy( &_pResult->sName, aBuffer, 11 );
			_pResult->sName[12] = '\0';		/* don't forget 0 term */

			_pResult->nCluster = ( (aBuffer[26]) | (aBuffer[27]<<8) );
			_pResult->nFileSize = ( ((uint32_t)aBuffer[28]) | ((uint32_t)aBuffer[29]<<8) | ((uint32_t)aBuffer[30]<<16) | ((uint32_t)aBuffer[31]<<24) );
			_pResult->nFilePos = 0;
			_pResult->nSector = 0;
			_pResult->nSectorOffset = 0;		
			return SUCCESS;		
		}
		else {
			nFirstRootDirAddress++;
		}
					
	}
}

/*
 * read 32 bytes from fat file
 */
uint8_t fat_fread( uint8_t* _pBuffer, FAT_Stream_t* _this, FAT_Handle_t* _fileHandle ) {
	/* check if cluster is already 0 */
	if( _fileHandle->nCluster == 0 || _fileHandle->nFilePos > _fileHandle->nFileSize ) {
		return FAT_FILEEND;	
	}

	/* calc address */
	uint32_t nByteAddress = (uint32_t)( fat_cluster2sector( _this, _fileHandle->nCluster ) + (uint32_t)_fileHandle->nSector )*
		(uint32_t)_this->structBPB.nBytesPerSec + (uint32_t)_fileHandle->nSectorOffset;
	nByteAddress = nByteAddress>>5;
	
	/* read 32 byte */
	CHECKREAD( _this->fpRead( nByteAddress, _pBuffer ) );

	/* advance file offset */
	_fileHandle->nFilePos += 32;		

	/* advance sector offset */
	_fileHandle->nSectorOffset += 32;	
	if( _fileHandle->nSectorOffset >= _this->structBPB.nBytesPerSec ) {
		_fileHandle->nSector++;
		_fileHandle->nSectorOffset = 0;			
	}
	
	/* advance cluster */
	if( _fileHandle->nSector >= _this->structBPB.nSecPerClus) {
		_fileHandle->nCluster = fat_nextcluster( _this, _fileHandle->nCluster );
		_fileHandle->nSector = 0;	
	}

	return SUCCESS; 	
}

/*
 * seek to file position
 */
uint8_t fat_seek( uint32_t _nPosition, FAT_Stream_t* _this, FAT_Handle_t* _fileHandle ) {
	uint16_t nCluster;	

	/* check if position is 32 byte aligned */
	if( (_nPosition & 31) != 0 ) {
		return FAIL;	
	}	

	/* check bounds! */
	if( _nPosition > _fileHandle->nFileSize || _nPosition < 0 ) {
		return FAIL;	
	}

	/* normally we would only need to reopen the file in case the position is 
	 * smaller than the actual one. In order to simplify the implementation we 
     * do it every time
	 */
	CHECKREAD( fat_fopen( _fileHandle->sName, _this, _fileHandle ) );		

	/* check for 0 position to avoid div failures */
	if( _nPosition == 0 ) {
		return SUCCESS;
	}	

	/* calc cluster & advance to it */
	_fileHandle->nFilePos = _nPosition;
	nCluster = _nPosition / ( _this->structBPB.nBytesPerSec * _this->structBPB.nSecPerClus );
	while( nCluster > 0 ) {
		_fileHandle->nCluster = fat_nextcluster( _this, _fileHandle->nCluster );
		nCluster--;	
	}
	
	/* calc sector */
	_nPosition -= ( _this->structBPB.nBytesPerSec * _this->structBPB.nSecPerClus * nCluster );
	_fileHandle->nSector = _nPosition / _this->structBPB.nBytesPerSec;

	/* calc sector offset */
	_fileHandle->nSectorOffset = _nPosition - (_this->structBPB.nBytesPerSec * _fileHandle->nSector );	
	
	return SUCCESS;
}

