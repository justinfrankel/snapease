#ifndef _UPLOADER_H_
#define _UPLOADER_H_


class IFileUploader
{
public:
  virtual ~IFileUploader() { }

  virtual bool SendFile(const char *srcfullfn, const char *destfn)=0; // true if success
  virtual int Run(const char *statusBuf, int statusBufLen)=0; // >0 completed, <0 error (statusBuf will be error text)

};


#endif