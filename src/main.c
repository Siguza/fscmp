#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <AppleFSCompression/AppleFSCompression.h>
#include <CoreFoundation/CoreFoundation.h>

static void formatBytes(uint64_t bytes, char buf[10])
{
    const char *suffix[] = { "", "K", "M", "G", "T", "P", "E" };
    long double d = bytes;
    size_t i = 0;
    while(d >= 999.995)
    {
        d /= 1000.0;
        ++i;
    }
    snprintf(buf, 10, "%.*Lf %sB", i == 0 ? 0 : 2, d, suffix[i]);
}

int main(int argc, const char **argv)
{
    if(argc < 2)
    {
        goto help;
    }

    const char *act = argv[1];
    size_t n = strlen(act);
    if(!n)
    {
        fprintf(stderr, "No action given.\n\n");
        goto help;
    }
    int aoff = 2;

    if(strncmp(act, "compress", n) == 0)
    {
        bool progress = false;
        int compressionLevel = 9;
        int compressionType = kAFSCTypeLZFSE;

        for(; aoff < argc; ++aoff)
        {
            const char *opt = argv[aoff];
            if(opt[0] != '-')
            {
                break;
            }
            if(opt[1] == '-')
            {
                ++aoff;
                break;
            }
            char c;
            for(size_t i = 1; (c = opt[i]) != '\0'; ++i)
            {
                switch(c)
                {
                    case '1':
                    case '2':
                    case '3':
                    case '4':
                    case '5':
                    case '6':
                    case '7':
                    case '8':
                    case '9':
                        compressionLevel = c - '0';
                        break;

                    case 'B':
                        compressionType = kAFSCTypeLZBitmap;
                        break;

                    case 'F':
                        compressionType = kAFSCTypeLZFSE;
                        break;

                    case 'R':
                        compressionType = kAFSCTypeRaw;
                        break;

                    case 'V':
                        compressionType = kAFSCTypeLZVN;
                        break;

                    case 'Z':
                        compressionType = kAFSCTypeZLib;
                        break;

                    case 'p':
                        progress = true;
                        break;

                    default:
                        fprintf(stderr, "Bad compress option: -%c\n\n", c);
                        goto help;
                }
            }
        }
        if(aoff >= argc)
        {
            fprintf(stderr, "Missing file argument.\n\n");
            goto help;
        }

        CFNumberRef cfCompressionLevel = CFNumberCreate(NULL, kCFNumberIntType, &compressionLevel);
        if(!cfCompressionLevel)
        {
            fprintf(stderr, "Failed to allocate compression level number.\n");
            return -1;
        }

        CFStringRef cfCompressionType = CFStringCreateWithFormat(NULL, NULL, CFSTR("%d"), compressionType);
        if(!cfCompressionType)
        {
            CFRelease(cfCompressionLevel);
            fprintf(stderr, "Failed to allocate compression type string.\n");
            return -1;
        }

        CFMutableDictionaryRef options = CFDictionaryCreateMutable(NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
        if(!options)
        {
            CFRelease(cfCompressionLevel);
            CFRelease(cfCompressionType);
            fprintf(stderr, "Failed to allocate options dict.\n");
            return -1;
        }

        CFDictionarySetValue(options, kAFSCCompressionLevel, cfCompressionLevel);
        CFDictionarySetValue(options, kAFSCCompressionTypes, cfCompressionType);
        CFRelease(cfCompressionLevel);
        CFRelease(cfCompressionType);

        AppleFSCompressionQueue queue = CreateCompressionQueue(NULL, NULL, NULL, NULL, options);
        if(!queue)
        {
            CFRelease(options);
            fprintf(stderr, "Failed to allocate compression queue.\n");
            return -1;
        }

        int ret = 0;
        for(int i = aoff; i < argc; ++i)
        {
            if(progress)
            {
                printf("\r%d/%d", i - aoff, argc - aoff);
                fflush(stdout);
            }
            const char *file = argv[i];
            if(!CompressFile(queue, file, NULL))
            {
                int r = errno;
                fprintf(stderr, "%s: %s (%d)\n", file, strerror(r), r);
                if(ret == 0)
                {
                    ret = r == 0 ? -1 : r;
                }
            }
        }
        if(progress)
        {
            printf("\r%d/%d\n", argc - aoff, argc - aoff);
        }

        FinishCompressionAndCleanUp(queue);
        CFRelease(options);

        return ret;
    }
    else if(strncmp(act, "decompress", n) == 0)
    {
        bool progress = false;
        bool verify = true;

        for(; aoff < argc; ++aoff)
        {
            const char *opt = argv[aoff];
            if(opt[0] != '-')
            {
                break;
            }
            if(opt[1] == '-')
            {
                ++aoff;
                break;
            }
            char c;
            for(size_t i = 1; (c = opt[i]) != '\0'; ++i)
            {
                switch(c)
                {
                    case 'f':
                        verify = false;
                        break;

                    case 'p':
                        progress = true;
                        break;

                    default:
                        fprintf(stderr, "Bad decompress option: -%c\n\n", c);
                        goto help;
                }
            }
        }
        if(aoff >= argc)
        {
            fprintf(stderr, "Missing file argument.\n\n");
            goto help;
        }

        int ret = 0;
        for(int i = aoff; i < argc; ++i)
        {
            if(progress)
            {
                printf("\r%d/%d", i - aoff, argc - aoff);
                fflush(stdout);
            }
            const char *file = argv[i];
            if(decompressFile(file, verify, false) != 0) // XXX: setting the second param to "true" causes failure
            {
                int r = errno;
                fprintf(stderr, "%s: %s (%d)\n", file, strerror(r), r);
                if(ret == 0)
                {
                    ret = r == 0 ? -1 : r;
                }
            }
        }
        if(progress)
        {
            printf("\r%d/%d\n", argc - aoff, argc - aoff);
        }

        return ret;
    }
    else if(strncmp(act, "info", n) == 0)
    {
        int mode = 0;

        for(; aoff < argc; ++aoff)
        {
            const char *opt = argv[aoff];
            if(opt[0] != '-')
            {
                break;
            }
            if(opt[1] == '-')
            {
                ++aoff;
                break;
            }
            char c;
            for(size_t i = 1; (c = opt[i]) != '\0'; ++i)
            {
                switch(c)
                {
                    case 'q':
                        mode = 1;
                        break;

                    case 't':
                        mode = 2;
                        break;

                    default:
                        fprintf(stderr, "Bad info option: -%c\n\n", c);
                        goto help;
                }
            }
        }
        if(aoff >= argc)
        {
            fprintf(stderr, "Missing file argument.\n\n");
            goto help;
        }

        int ret = 0;
        bool first = true;
        for(int i = aoff; i < argc; ++i)
        {
            const char *file = argv[i];
            AppleFSCompressionInfo_t info = {};
            if(queryCompressionInfo(file, &info) == 0)
            {
                switch(mode)
                {
                    case 0:
                    {
                        char usize[10] = {};
                        formatBytes(info.uncompressedSize, usize);
                        if(info.type == 0)
                        {
                            printf("%s%s:\n"
                                   "                 Type: Uncompressed (0)\n"
                                   "                 Size: %s (0x%llx)\n"
                                   , first ? "" : "\n", file, usize, info.uncompressedSize);
                        }
                        else
                        {
                            const char *type = NULL;
                            switch(info.type)
                            {
                                case kAFSCTypeZLibChunk:     type = "ZLibChunk";     break;
                                case kAFSCTypeZLib:          type = "ZLib";          break;
                                case kAFSCTypeLZVNChunk:     type = "LZVNChunk";     break;
                                case kAFSCTypeLZVN:          type = "LZVN";          break;
                                case kAFSCTypeRawChunk:      type = "RawChunk";      break;
                                case kAFSCTypeRaw:           type = "Raw";           break;
                                case kAFSCTypeLZFSEChunk:    type = "LZFSEChunk";    break;
                                case kAFSCTypeLZFSE:         type = "LZFSE";         break;
                                case kAFSCTypeLZBitmapChunk: type = "LZBitmapChunk"; break;
                                case kAFSCTypeLZBitmap:      type = "LZBitmap";      break;
                                default:                     type = "Unknown";       break;
                            }
                            char csize[10] = {};
                            formatBytes(info.compressedSize, csize);
                            printf("%s%s:\n"
                                   "                 Type: %s (%u)\n"
                                   "          Chunk count: %u (0x%x)\n"
                                   "      Compressed size: %s (0x%llx)\n"
                                   "    Uncompressed size: %s (0x%llx)\n"
                                   "                 0x18: 0x%llx\n"
                                   , first ? "" : "\n", file, type, info.type, info.chunkCount, info.chunkCount, csize, info.compressedSize, usize, info.uncompressedSize, info.f0x18);
                        }
                        break;
                    }

                    case 1:
                    {
                        if(ret == 0 && info.type == 0)
                        {
                            ret = -1;
                        }
                        break;
                    }

                    case 2:
                    {
                        printf("%u\n", info.type);
                        break;
                    }
                }

                first = false;
            }
            else
            {
                int r = errno;
                fprintf(stderr, "%s: %s (%d)\n", file, strerror(r), r);
                if(ret == 0)
                {
                    ret = r == 0 ? -1 : r;
                }
            }
        }

        return ret;
    }
    else
    {
        fprintf(stderr, "Bad action: %s\n\n", act);
        goto help;
    }

help:;
    fprintf(stderr, "Usage: %s action [...]\n"
                    "\n"
                    "Actions:\n"
                    "    c[ompress] [options] file [file...]\n"
                    //"    d[ecompress] [options] file [file...]\n"
                    "    i[nfo] [options] file [file...]\n"
                    "\n"
                    "Options for compress:\n"
                    "    --  Treat all following arguments as path literals\n"
                    "    -1  Compression level 1\n"
                    "    -9  Compression level 9 (default)\n"
                    "    -B  Compression type LZBitmap\n"
                    "    -F  Compression type LZFSE (default)\n"
                    "    -R  Compression type raw (uncompressed, just store)\n"
                    "    -V  Compression type LZVN\n"
                    "    -Z  Compression type ZLib\n"
                    "    -p  Show progress (between files only)\n"
                    "\n"
                    //"Options for decompress:\n"
                    //"    --  Treat all following arguments as path literals\n"
                    //"    -f  Fast (skip data verification)\n"
                    //"    -p  Show progress (between files only)\n"
                    //"\n"
                    "Options for info:\n"
                    "    --  Treat all following arguments as path literals\n"
                    "    -q  Quiet, just exit with 0 if all files are compressed, non-zero otherwise\n"
                    "    -t  Print only the compression type ID, one line per file\n"
                    , argv[0]);
    return -1;
}
