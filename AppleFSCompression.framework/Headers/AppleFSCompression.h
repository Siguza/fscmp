#ifndef _APPLEFSCOMPRESSION_PRIVATEFRAMEWORK
#define _APPLEFSCOMPRESSION_PRIVATEFRAMEWORK

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <CoreFoundation/CoreFoundation.h>

// Keys for the "options" dict passed to CreateCompressionQueue() and CreateStreamCompressorQueueWithOptions().
extern const _Nonnull CFStringRef kAFSCSkipMachOFileTypes;         // CFSTR("SkipMachOFileTypes")
extern const _Nonnull CFStringRef kAFSCSkipMachOOverridePaths;     // CFSTR("SkipMachOOverridePaths")
extern const _Nonnull CFStringRef kAFSCCompressionTypes;           // CFSTR("CompressionTypes")
extern const _Nonnull CFStringRef kAFSCCompressionLevel;           // CFSTR("CompressionLevel")
extern const _Nonnull CFStringRef kAFSCAllowLargeResourceForks;    // CFSTR("AllowLargeResourceForks")
extern const _Nonnull CFStringRef kAFSCSynchronous;                // CFSTR("Synchronous")
extern const _Nonnull CFStringRef kAFSCThrottledIO;                // CFSTR("AFSCThrottledIO")
extern const _Nonnull CFStringRef kAFSCIgnoreXattrErrors;          // CFSTR("IgnoreXattrErrors")
extern const _Nonnull CFStringRef kAFSCAllowStoringDataInXattr;    // CFSTR("AllowStoringDataInXattr")
extern const _Nonnull CFStringRef kAFSCDispatchQueuePriority;      // CFSTR("DispatchQueuePriority")
extern const _Nonnull CFStringRef kAFSCCompressionRulesKey;        // CFSTR("CompressionRules")
extern const _Nonnull CFStringRef kAFSCCompressionSystemRulesKey;  // CFSTR("SystemCompressionRules")
extern const _Nonnull CFStringRef kAFSCRuleTypeKey;                // CFSTR("RuleType")
extern const _Nonnull CFStringRef kAFSCRuleTypeInclude;            // CFSTR("Include")
extern const _Nonnull CFStringRef kAFSCRuleTypeExclude;            // CFSTR("Exclude")
extern const _Nonnull CFStringRef kAFSCPathEndsWithStringKey;      // CFSTR("PathEndsWithString")
extern const _Nonnull CFStringRef kAFSCPathBeginsWithStringKey;    // CFSTR("PathBeginsWithString")
extern const _Nonnull CFStringRef kAFSCPathContainsStringKey;      // CFSTR("PathContainsString")
extern const _Nonnull CFStringRef kAFSCPathExactlyEqualsStringKey; // CFSTR("PathExactlyEqualsString")
extern const _Nonnull CFStringRef kAFSCFlagsKey;                   // CFSTR("Flags")
extern const _Nonnull CFStringRef kAFSCInverseFlags;               // CFSTR("InverseFlags")

// "CompressionTypes" values
enum
{
    kAFSCTypeZLibChunk      =  3,
    kAFSCTypeZLib           =  4,
    kAFSCTypeLZVNChunk      =  7,
    kAFSCTypeLZVN           =  8,
    kAFSCTypeRawChunk       =  9,
    kAFSCTypeRaw            = 10,
    kAFSCTypeLZFSEChunk     = 11,
    kAFSCTypeLZFSE          = 12,
    kAFSCTypeLZBitmapChunk  = 13,
    kAFSCTypeLZBitmap       = 14,
};

typedef struct
{
    uint32_t type;
    uint32_t chunkCount;
    uint64_t compressedSize;
    uint64_t uncompressedSize;
    uint64_t f0x18; // XXX: unknown purpose, field name will change when I find out
} AppleFSCompressionInfo_t;

struct _AppleFSCompressionQueue; // opaque
typedef struct _AppleFSCompressionQueue *AppleFSCompressionQueue;
struct _AppleFSCompressionStreamCompressor; // opaque
typedef struct _AppleFSCompressionStreamCompressor *AppleFSCompressionStreamCompressor;

typedef const void * _Nullable _UNKNOWN;

_Nullable AppleFSCompressionQueue CreateCompressionQueue(_UNKNOWN, _UNKNOWN, _UNKNOWN, _UNKNOWN, _Nullable CFDictionaryRef options);
_Nullable AppleFSCompressionQueue CreateStreamCompressorQueue(void);
_Nullable AppleFSCompressionQueue CreateStreamCompressorQueueWithOptions(_Nullable CFDictionaryRef options);
int DrainStreamCompressorQueue(_Nonnull AppleFSCompressionQueue queue);
int FinishStreamCompressorQueue(_Nonnull AppleFSCompressionQueue queue);
void FinishCompressionAndCleanUp(_Nonnull AppleFSCompressionQueue queue);

_Nullable AppleFSCompressionStreamCompressor CreateStreamCompressor(int fd, const char * _Nonnull path, const char * _Nullable prefix, _Nonnull AppleFSCompressionQueue queue);
size_t WriteToStreamCompressor(_Nonnull AppleFSCompressionStreamCompressor compressor, const void * _Nonnull buffer, size_t length);
int CloseStreamCompressor(_Nonnull AppleFSCompressionStreamCompressor compressor);

int queryCompressionType(const char * _Nonnull path, uint32_t * _Nonnull type);
int fqueryCompressionType(int fd, uint32_t * _Nonnull type);
int queryCompressionInfo(const char * _Nonnull path, AppleFSCompressionInfo_t * _Nonnull info);
int fqueryCompressionInfo(int fd, AppleFSCompressionInfo_t * _Nonnull info);

bool CompressFile(_Nonnull AppleFSCompressionQueue queue, const char * _Nonnull path, const char * _Nullable prefix);
int decompressFile(const char * _Nonnull path, bool verifyData, bool _unknown);

bool VolumeSupportsCompression(const char * _Nonnull mountpoint);

#ifdef __cplusplus
}
#endif

#endif
