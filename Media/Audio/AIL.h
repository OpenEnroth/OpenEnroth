#pragma once



/*  252 */
enum AILFILETYPE : __int32
{
  AILFILETYPE_UNKNOWN = 0x0,
  AILFILETYPE_PCM_WAV = 0x1,
  AILFILETYPE_ADPCM_WAV = 0x2,
  AILFILETYPE_OTHER_WAV = 0x3,
  AILFILETYPE_VOC = 0x4,
  AILFILETYPE_MIDI = 0x5,
  AILFILETYPE_XMIDI = 0x6,
  AILFILETYPE_7 = 0x7,
  AILFILETYPE_XMIDI_MLS = 0x8,
  AILFILETYPE_DLS = 0x9,
  AILFILETYPE_MLS = 0xA,
  AILFILETYPE_MPEG_L1_AUDIO = 0xB,
  AILFILETYPE_MPEG_L2_AUDIO = 0xC,
  AILFILETYPE_MPEG_L3_AUDIO = 0xD,
  AILFILETYPE_OTHER_ASI_WAV = 0xE,
  AILFILETYPE_OTHER_ASI_F = 0xF,
};



/*  253 */
#pragma pack(push, 1)
struct AILSOUNDINFO
{
  int uFormat;
  void *pData;
  unsigned int uDataSize;
  unsigned int uRate;
  int uBits;
  int uChannels;
  unsigned int uSamples;
  unsigned int uBlockSize;
  void *pInitial;
};
#pragma pack(pop)




//Bink
typedef struct _BINK    *HBINK;
typedef struct _BINKBUF *HBINKBUF;
//Smacker & AIL types
typedef struct _DIG_DRIVER *HDIGDRIVER;
typedef struct _SMACK      *HSMACK;
typedef struct _SMACKBLIT  *HSMACKBLIT;
typedef struct _SMACKBUF   *HSMACKBUF;
//AIL
typedef struct _REDBOOK    *HREDBOOK;
typedef struct _SAMPLE     *HSAMPLE;
typedef struct _PROVIDER   *HPROVIDER;
typedef struct _SEQUENCE   *HSEQUENCE;
typedef struct _STREAM     *HSTREAM;
typedef __int32             HATTRIB;
typedef __int32             HASISTREAM; // Handle to stream being managed by ASI codec



void MSS32_DLL_Initialize();

#define AILCALL __stdcall
#define AILCALLBACK __stdcall


typedef __int32 (AILCALLBACK * AILASIFETCHCB) (unsigned __int32		user,			// User value passed to ASI_open_stream()
												void *dest,			// Location to which stream data should be copied by app
												__int32		bytes_requested, // # of bytes requested by ASI codec
												__int32		offset);		 // If not -1, application should seek to this point in stream

typedef HASISTREAM (AILCALL *ASI_STREAM_OPEN) (unsigned __int32			user,				// User value passed to fetch callback
													AILASIFETCHCB fetch_CB,			// Source data fetch handler
													unsigned __int32			total_size);		// Total size for %-done calculations (0=unknown)
typedef __int32	(AILCALL *ASI_STREAM_PROCESS) (HASISTREAM	stream,				// Handle of stream
												void *buffer,				// Destination for processed data
												__int32		 buffer_size);		// # of bytes to return in buffer
typedef __int32 (AILCALL *ASI_STREAM_SEEK)	(HASISTREAM stream,
													 __int32		stream_offset);
typedef __int32 (AILCALL *ASI_STREAM_CLOSE) (HASISTREAM stream);
typedef __int32 (AILCALL *ASI_STREAM_ATTRIBUTE) (HASISTREAM stream,
												 HATTRIB	attrib);
typedef __int32 (AILCALL *ASI_STREAM_SET_PREFERENCE) (HASISTREAM stream,
														HATTRIB	preference,
														void const *	value);

typedef struct
	{
	ASI_STREAM_OPEN			ASI_stream_open;
	ASI_STREAM_PROCESS		ASI_stream_process;
	ASI_STREAM_SEEK			ASI_stream_seek;
	ASI_STREAM_CLOSE			ASI_stream_close;
	ASI_STREAM_ATTRIBUTE		ASI_stream_attribute;
	ASI_STREAM_SET_PREFERENCE ASI_stream_set_preference;

	HATTRIB INPUT_BIT_RATE;
	HATTRIB INPUT_SAMPLE_RATE;
	HATTRIB INPUT_BITS;
	HATTRIB INPUT_CHANNELS;
	HATTRIB OUTPUT_BIT_RATE;
	HATTRIB OUTPUT_SAMPLE_RATE;
	HATTRIB OUTPUT_BITS;
	HATTRIB OUTPUT_CHANNELS;
	HATTRIB POSITION;
	HATTRIB PERCENT_DONE;
	HATTRIB MIN_INPUT_BLOCK_SIZE;
	HATTRIB RAW_RATE;
	HATTRIB RAW_BITS;
	HATTRIB RAW_CHANNELS;
	HATTRIB REQUESTED_RATE;
	HATTRIB REQUESTED_BITS;
	HATTRIB REQUESTED_CHANS;

	HASISTREAM stream;
	}
ASISTAGE;




typedef void (AILCALLBACK * AILSTREAMCB)(HSTREAM stream);
typedef struct _STREAM {

	__int32 block_oriented; // 1 if this is an ADPCM or ASI-compressed stream
	__int32 using_ASI;		// 1 if using ASI decoder to uncompress stream data
	ASISTAGE *ASI;	// handy pointer to our ASI coded

	HSAMPLE samp;		// the sample handle

	unsigned __int32 fileh;			// the open file handle

	unsigned __int8 * bufs[3];	// the data buffers
	unsigned __int32 bufsizes[3];	// the size of each buffer
	__int32 reset_ASI[3];	// should we reset the ASI at the end of the buffer?
	__int32 bufstart[3];	// offset of where this buffer started
	void * asyncs[3];// async read structures

	__int32 loadedbufstart[2]; // offset of where the loaded buffer started
	__int32 loadedorder[2]; // order of the buffers as they were loaded
	__int32 loadorder;		// incremented as each buffer is loaded

	__int32 bufsize;		// size of each buffer
	__int32 readsize;		// size of each read block

	unsigned __int32 buf1;			// 0,1,2 (current buffer that we are reading into)
	__int32 size1;			// holds the current amount of data read

	unsigned __int32 buf2;			// 0,1,2 (the next buffer that we are reading into)
	__int32 size2;			// next buffer loaded up to

	unsigned __int32 buf3;			// 0,1,2 (the next buffer that we are reading into)
	__int32 size3;			// next buffer loaded up to

	unsigned __int32 datarate;		// datarate in bytes per second
	__int32 filerate;		// original datarate of the file
	__int32 filetype;		// file format type
	unsigned __int32 fileflags;		// file format flags (signed or unsigned)
	__int32 totallen;		// total length of the sound data

	__int32 substart;		// subblock loop start
	__int32 sublen;		 // subblock loop len
	__int32 subpadding;	 // amount to pad the final block

	unsigned __int32 blocksize;		// ADPCM block size
	__int32 padding;		// padding to be done
	__int32 padded;		 // padding done

	__int32 loadedsome;	 // have we done any loads?

	unsigned __int32 startpos;		// point that the sound data begins
	unsigned __int32 totalread;		// total bytes read from the disk

	unsigned __int32 loopsleft;		// how many loops are left

	unsigned __int32 error;			// read error has occurred

	__int32 preload;		// preload the file into the first buffer
	unsigned __int32 preloadpos;	 // position to use in preload
	__int32 noback;		 // no background processing
	__int32 alldone;		// alldone
	__int32 primeamount;	// amount to load after a seek
	__int32 readatleast;	// forced amount to read on next service

	__int32 playcontrol;	// control: 0=stopped, 1=started, |8=paused, |16=sample paused

	AILSTREAMCB callback;	// end of stream callback

	__int32 user_data[8];	// Miscellaneous user data
	void * next;	 // pointer to next stream

#if defined(IS_WINDOWS) || defined(IS_MAC)
	__int32 autostreaming;	// are we autostreaming this stream
#endif

#ifdef IS_WINDOWS
	__int32 cb_IsWin32s;	// Is the callback win32s?
#endif
	__int32 docallback;	 // set when it time to poll for a callback
#ifdef IS_MAC
	IOParam	stream_param;
	__int32		donext;
	__int32		donext1;
	unsigned __int32		fillup;
	unsigned __int32		session;
	unsigned __int32		tamt;
	unsigned __int32		buf;
	__int32*	 size;
	__int32*	 done;
	__int32		done1;
	__int32		done2;
	__int32		done3;
	Boolean	force_quit;
#endif

} MSTREAM_TYPE;



int __stdcall AIL_startup();
HREDBOOK __stdcall AIL_redbook_open_drive(long drive);
HREDBOOK __stdcall AIL_redbook_open(int);
int __stdcall AIL_set_preference(unsigned int number, int value);
int __stdcall AIL_waveOutOpen(HDIGDRIVER *drv, void *phWaveOut, int wDeviceID, void *waveformat);
int __stdcall AIL_get_preference(unsigned int number);
int __stdcall AIL_digital_configuration(HDIGDRIVER drv, int *rate, int *format, char *string);
HSAMPLE __stdcall AIL_allocate_sample_handle(HDIGDRIVER hDrv);
unsigned int __stdcall AIL_redbook_tracks(HREDBOOK hRedbook);
int __stdcall AIL_redbook_volume(HREDBOOK hRedbook);
unsigned int __stdcall AIL_redbook_stop(HREDBOOK hRedbook);
void __stdcall AIL_set_digital_master_volume(HDIGDRIVER hDrv, float master_volume);
int __stdcall AIL_redbook_set_volume(HREDBOOK hRedbook, int volume);
unsigned int __stdcall AIL_redbook_pause(HREDBOOK hRedbook);
void __stdcall AIL_redbook_track_info(HREDBOOK hRedbook, unsigned int uTrackNum, unsigned int *pStartMS, unsigned int *pEndMS);
unsigned int __stdcall AIL_redbook_play(HREDBOOK hRedbook, unsigned int uStartMS, unsigned int uEndMS);  	
unsigned int __stdcall AIL_redbook_resume(HREDBOOK);
int __stdcall AIL_enumerate_3D_providers(int *a1, HPROVIDER *pOutProv, char **pOutName);
int __stdcall AIL_open_3D_provider(HPROVIDER a2);
void __stdcall AIL_3D_provider_attribute(HPROVIDER lib, char *name, void *val);
int __stdcall AIL_set_3D_provider_preference(HPROVIDER a1, const char *a2, int *a3);
void __stdcall AIL_waveOutClose(HDIGDRIVER drvr);
int __stdcall AIL_allocate_3D_sample_handle(HPROVIDER);
int __stdcall AIL_set_3D_sample_float_distances(void *a1, long a2, long a3, long a4, long a5);
int __stdcall AIL_set_3D_sample_volume(void *a1, long a2);
void __stdcall AIL_release_sample_handle(HSAMPLE s);
int __stdcall AIL_3D_position(void *a1, int *a2, float *a3, long *a4);
int __stdcall AIL_set_3D_sample_file(long a1, void *a2);
int __stdcall AIL_set_3D_sample_loop_count(long a1, long a2);
int __stdcall AIL_start_3D_sample(long a1);
int __stdcall AIL_set_3D_position(void *hSample, long a2, long a3, long a4);
int __stdcall AIL_set_3D_orientation(void *hSample, long a2, long a3, long a4, long a5, long a6, long a7);
int __stdcall AIL_release_3D_sample_handle(void *hHandle);

int __stdcall AIL_close_3D_provider(HPROVIDER);
int __stdcall AIL_redbook_close(HREDBOOK);

// sub_4D8344: using guessed type int __stdcall AIL_shutdown();
int __stdcall AIL_shutdown();

int __stdcall AIL_end_sample(HSAMPLE a1);
int __stdcall AIL_end_3D_sample(void *a1);
void __stdcall AIL_end_sequence(HSEQUENCE a1);
void __stdcall AIL_pause_stream(HSTREAM a1, int onoff);
int __stdcall AIL_set_sample_file(HSAMPLE, const void *file_image, int block);
int __stdcall AIL_start_sample(HSAMPLE);
void __stdcall AIL_set_sample_playback_rate(HSAMPLE, int rate);
void __stdcall AIL_sample_ms_position(HSAMPLE, int *pTotalMS, int *pCurrentMS);
int __stdcall AIL_3D_sample_status(void *a1);


namespace AIL
{
 namespace Sample
 {
  enum Status
  {
   Free = 1,               // Sample is available for allocation
   Done = 2,               // Sample has finished playing, or has never been started
   Playing = 4,            // Sample is playing
   Stopped = 8,            // Sample has been stopped
   PlayingButReleased = 16 // Sample is playing, but digital handle has been temporarily released
  };
 };
};
AIL::Sample::Status __stdcall AIL_sample_status(HSAMPLE a1);


void __stdcall AIL_set_sample_loop_count(HSAMPLE, int);
int __stdcall AIL_set_sample_volume(HSAMPLE a1, long a2);


int __stdcall AIL_sample_volume(HSAMPLE);
void __stdcall AIL_init_sample(HSAMPLE);
int __stdcall AIL_set_sample_pan(HSAMPLE, long a2);
AILFILETYPE __stdcall AIL_file_type(void *pSoundBytes, int numBytes);
int __stdcall AIL_WAV_info(void *pSoundBytes, AILSOUNDINFO *pInfo);
int __stdcall AIL_decompress_ADPCM(AILSOUNDINFO *pInfo, void *a2, void *a3);
int __stdcall AIL_mem_free_lock(void *a1);










